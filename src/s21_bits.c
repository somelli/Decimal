#include "s21_bits.h"

// функция для получения нужного бита (0/1)
int s21_get_bit(s21_decimal value, int index) {
  int result = 0;
  // защита от неправильного index, если все ок- смотрим бит:
  if (index >= 0 && index <= 95) {
    // узнаем, в каком элементе массива bits лежит нужный бит
    int array_index = index / 32;
    // узнаем номер бита в этом массиве
    int bit_index = index % 32;
    // создаем маску с единицей на позиции нужного бита, создавая число,
    // отвечающее за этот бит
    // используем unsigned и u, чтобы все 32 бита были числом
    unsigned int mask = 1u << bit_index;

    // сраниваем позицию бита и полученное число mask через побитовое & (логика
    // как у &&)
    if ((value.bits[array_index] & mask) == 0) {
      result = 0;
    } else {
      result = 1;
    }
    // если нет, ошибка и выход из функции
  } else {
    result = -1;
  }

  return result;
}

// функция для изменения бита (0/1)
//(с каким decimal работаем, какой бит, что ставим(0/1))
int s21_set_bit(s21_decimal* value, int index, int bit) {
  int result = 0;
  // защита от неправильного index и bit, если все ок- смотрим бит:
  if (index >= 0 && index <= 95 && (bit == 1 || bit == 0)) {
    // узнаем, в каком элементе массива bits лежит нужный бит
    int array_index = index / 32;
    // узнаем номер бита в этом bits
    int bit_index = index % 32;
    // создаем маску с единицей на позиции нужного бита, создавая число,
    // отвечающее за этот бит
    unsigned int mask = 1u << bit_index;

    if (bit == 0) {
      // меняем на нужное значение через побитовое & (логика как у &&)
      //  ~ инвертирует значение маски
      value->bits[array_index] &= ~mask;
      result = 0;
    } else if (bit == 1) {
      // меняем на нужное значение через побитовое | (логика как у ||)
      value->bits[array_index] |= mask;
      result = 0;
    }
    // если index или bit некорректны, возвращаем ошибку
  } else {
    result = -1;
  }

  return result;
}

// функция для уточнения знака числа
int s21_get_sign(s21_decimal value) {
  int result = 0;
  // значение хранится в bits[3] на 31 бите в (0 - полож, 1 - отриц)
  unsigned int mask = 1u << 31;
  // если 31 элемент равен 0 (то есть положительный), возвращаем 0
  if ((value.bits[3] & mask) == 0) {
    result = 0;
    // если отрицательный - 1
  } else {
    result = 1;
  }
  return result;
}

// функция для изменения знака числа
int s21_set_sign(s21_decimal* value, int bit) {
  int result = 0;
  unsigned int mask = 1u << 31;

  // если нужно положительное меняем через & и конверсию ~
  if (bit == 0) {
    value->bits[3] &= ~mask;
    result = 0;
    // если отрицательное меняем через |
  } else if (bit == 1) {
    value->bits[3] |= mask;
    result = 0;
    // в остальных случаях ошибка
  } else {
    result = -1;
  }
  return result;
}

// функция для уточнения запятой числа
int s21_get_scale(s21_decimal value) {
  int result = 0;
  // уходим от отрицательного значения в int
  unsigned int bits = (unsigned int)value.bits[3];
  // сдвигаем в bits3 первые 16 битов, убирая их
  bits >>= 16;
  // берем число равное 11111111 в двоичной системе
  unsigned int mask = 255u;
  // накладываем маску на bits3
  result = mask & bits;
  // возвращаем scale
  return result;
}

// функция для установки scale
int s21_set_scale(s21_decimal* value, unsigned int scale) {
  int result = 0;
  // проверка на вводимое число
  if (scale <= 28) {
    // создаем маску с 11111111 и сдвигаем ее влево на 16 бит
    //  00000000 11111111 00000000 00000000
    unsigned int mask = 255u << 16;
    // инвертируем маску 11111111 00000000 11111111 11111111
    // чтобы не задеть остальные элементы
    mask = ~mask;
    // сравниваем маску с битс3 и обнуляем биты в диапаозне 16...23
    value->bits[3] &= mask;
    // берем scale и сдвигаем его на позицию 16...23
    scale <<= 16;
    // сравниваем маску с битс3 и записываем нужное значение
    value->bits[3] |= scale;
    result = 0;
  } else {
    result = -1;
  }

  return result;
}

// функция для проверки деления на 0
int s21_check_zero_divisor(s21_decimal value) {
  int result = 0;
  if (value.bits[0] == 0 && value.bits[1] == 0 && value.bits[2] == 0) {
    result = 0;
  } else {
    result = 1;
  }
  return result;
}