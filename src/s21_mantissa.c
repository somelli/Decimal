#include "s21_mantissa.h"

#include "s21_bits.h"

// функция сравнения двух 96-битных мантисс
int s21_compare_mantissa(s21_decimal value_1, s21_decimal value_2) {
  int result = 0;
  // сравниваем мантиссы начинаем с bits2
  for (int i = 2; i >= 0; i--) {
    if ((unsigned int)value_1.bits[i] == (unsigned int)value_2.bits[i]) {
      result = 0;
    } else if ((unsigned int)value_1.bits[i] > (unsigned int)value_2.bits[i]) {
      result = 1;
    } else {
      result = -1;
    }

    if (value_1.bits[i] != value_2.bits[i]) {
      break;
    }
  }
  return result;
}

// функция для одновременного сдвига bits 0,1,2 влево
int s21_shift_left(s21_decimal* value) {
  int result = 0;
  // получаем старшие биты в каждом bits
  int max_bit0 = s21_get_bit(*value, 31);
  int max_bit1 = s21_get_bit(*value, 63);
  int max_bit2 = s21_get_bit(*value, 95);
  // если самы большой бит - 1, возвращаем ошибку (подумать - нужно ли?!)
  if (max_bit2 == 1) {
    result = -1;
  } else {
    // двигаем все влево
    for (int i = 0; i < 3; i++) {
      //"танцы" из-за разных типов int и unsigned int
      unsigned int current = (unsigned int)value->bits[i];
      current <<= 1;
      value->bits[i] = (int)current;
    }
    // вписываем переносимые биты
    s21_set_bit(value, 32, max_bit0);
    s21_set_bit(value, 64, max_bit1);
  }
  return result;
}

// функция для сложения мантисс
int s21_add_mantissa(s21_decimal value_1, s21_decimal value_2,
                     s21_decimal* result) {
  // добавляем переменную ошибки и переноса
  int error = 0;
  unsigned int carry = 0;
  // приводим все к ансайнд
  for (int i = 0; i < 3; i++) {
    unsigned int a = (unsigned int)value_1.bits[i];
    unsigned int b = (unsigned int)value_2.bits[i];
    // берем long long для проверки переполяется ли тип int
    unsigned long long sum =
        (unsigned long long)a + (unsigned long long)b + carry;
    // в result записываем только 32 бита
    result->bits[i] = (int)(unsigned int)sum;
    // сравниваем sum с самым большим числом для 32 бит
    if (sum > 4294967295ULL) {
      carry = 1;
    } else {
      carry = 0;
    }
  }
  // если после bits[2] carry все еще = 1, значит выдаем ошибку переполнения 96
  // бит
  if (carry == 1) {
    error = 1;
  }

  return error;
}

// функция для вычитания мантисс
int s21_sub_mantissa(s21_decimal value_1, s21_decimal value_2,
                     s21_decimal* result) {
  // добавляем переменную ошибки и переноса
  int error = 0;
  unsigned int borrow = 0;
  unsigned int borrow_old = 0;
  // приводим все к ансайнд
  for (int i = 0; i < 3; i++) {
    unsigned int a = (unsigned int)value_1.bits[i];
    unsigned int b = (unsigned int)value_2.bits[i];
    // если a минус занятое ранее число меньше b, занимаем еще
    if ((long long)a - borrow_old < (long long)b) {
      borrow = 1;
    } else {
      borrow = 0;
    }
    // проводим вычисления
    unsigned int dif = a - b - borrow_old;
    result->bits[i] = (int)dif;
    // запоминаем занимали ли доп цифру
    borrow_old = borrow;
  }
  // если после bits[2] остался borrow, значит value_1 < value_2
  if (borrow_old == 1) {
    error = 1;
  }

  return error;
}

int s21_increment(s21_decimal* value) {
  s21_decimal one = {0};
  one.bits[0] = 1;
  return s21_add_mantissa(*value, one, value);
}

// функция для умножения мантиссы на 10
int s21_mul_mantissa_by_10(s21_decimal value, s21_decimal* result) {
  int error = 0;
  unsigned long long carry = 0;

  for (int i = 0; i < 3; i++) {
    unsigned int a = (unsigned int)value.bits[i];
    // проводим вычисления
    unsigned long long mul = (unsigned long long)a * 10 + carry;
    result->bits[i] = (int)(unsigned int)mul;
    // чтобы забрать старшие биты, сдвигаем результат на 32 бита
    carry = mul >> 32;
  }

  // если после bits[2] carry != 0 -> overflow
  if (carry != 0) {
    error = 1;
  }

  return error;
}