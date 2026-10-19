#include "s21_scale.h"

#include "s21_bits.h"      // get/set scale
#include "s21_div10.h"     // div_mantissa_by_10
#include "s21_mantissa.h"  // compare, add/sub, increment, mul_by_10
#include "s21_rounding.h"  // apply_bank_rounding

// функция для выравнивания запятой у второго числа
int s21_align_scale(s21_decimal* value_1, s21_decimal* value_2) {
  // вводим переменные и вытаскиваем из 2х структур значения scale
  int error = 0;
  int scale_1 = s21_get_scale(*value_1);
  int scale_2 = s21_get_scale(*value_2);
  s21_decimal temp;

  // подгоняем scale одного под scale другого
  while (scale_1 < scale_2 && error == 0) {
    temp = *value_1;
    // записывая результат через temp
    error = s21_mul_mantissa_by_10(*value_1, &temp);
    // если все прошло удачно, сдвигаем запятую в scale
    if (error == 0) {
      *value_1 = temp;
      scale_1 += 1;
      s21_set_scale(value_1, scale_1);
    }
  }

  // делаем зеркально второй вариант
  while (scale_2 < scale_1 && error == 0) {
    temp = *value_2;
    // записывая результат через temp
    error = s21_mul_mantissa_by_10(*value_2, &temp);
    // если все прошло удачно, сдвигаем запятую в scale
    if (error == 0) {
      *value_2 = temp;
      scale_2 += 1;
      s21_set_scale(value_2, scale_2);
    }
  }
  return error;
}

// функция для уменьшения запятой и банковского округления
int s21_reduce_scale(s21_decimal* value, int new_scale) {
  int error = 0;
  int remainder = 0;
  int has_tail = 0;
  s21_decimal temp = *value;
  int scale = s21_get_scale(temp);
  // начинаем что-то делать, только если скейл > другого скейла
  if (scale > new_scale) {
    // делим на 10 и записываем остаток в has_tail
    while (scale > new_scale + 1) {
      remainder = s21_div_mantissa_by_10(temp, &temp);
      if (remainder != 0) {
        has_tail = 1;
      }
      scale--;
    }
    // делим на 10 еще раз и записываем остаток в remainder
    remainder = s21_div_mantissa_by_10(temp, &temp);
    scale--;

    // проверяем будет ли ошибка
    error = s21_apply_bank_rounding(&temp, remainder, has_tail);
    *value = temp;
    // сдвигаем запятую у оригинала
    s21_set_scale(value, scale);
  }

  return error;
}

// функция для отброса лишних нулей
int s21_remove_zeros(s21_decimal* value) {
  s21_decimal temp = *value;
  int remainder;
  int scale = s21_get_scale(*value);
  while (scale != 0) {
    remainder = s21_div_mantissa_by_10(*value, &temp);
    if (remainder == 0) {
      *value = temp;
      scale--;
      s21_set_scale(value, scale);
    } else {
      break;
    }
  }
  return 0;
}

// убираем дробную часть и сохраняем данные для округления
int s21_get_round_remainder(s21_decimal* value, int scale, int* has_tail) {
  int remainder = 0;
  *has_tail = 0;
  while (scale > 1) {
    remainder = s21_div_mantissa_by_10(*value, value);
    if (remainder != 0) {
      *has_tail = 1;
    }
    scale--;
  }
  remainder = s21_div_mantissa_by_10(*value, value);
  return remainder;
}