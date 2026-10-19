#include "s21_add_internal.h"

#include "s21_bits.h"      // get/set sign, get/set scale
#include "s21_div10.h"     // div_overflow_mantissa_by_10
#include "s21_mantissa.h"  // add/sub/compare
#include "s21_rounding.h"  // apply_bank_rounding

// функция для сложения мантисс с одинаковым знаком
int s21_add_equal_signs(s21_decimal value_1, s21_decimal value_2,
                        s21_decimal* result) {
  int error = 0;
  int full = 0;
  int scale = s21_get_scale(value_1);
  int sign = s21_get_sign(value_1);

  full = s21_add_mantissa(value_1, value_2, result);

  if (full == 1 && scale > 0) {
    int remainder = s21_div_overflow_mantissa_by_10(result);
    scale--;

    s21_decimal temp = *result;
    full = s21_apply_bank_rounding(&temp, remainder, 0);
    *result = temp;
  }

  if (full == 1 && sign == 0) {
    error = 1;
  } else if (full == 1 && sign == 1) {
    error = 2;
  } else {
    s21_set_scale(result, scale);
    s21_set_sign(result, sign);
  }

  return error;
}

// функция для сложения мантисс с разными знаками
int s21_add_different_signs(s21_decimal value_1, s21_decimal value_2,
                            s21_decimal* result) {
  // обозначаем переменные
  int compare = s21_compare_mantissa(value_1, value_2);
  int sign1 = s21_get_sign(value_1);
  int sign2 = s21_get_sign(value_2);
  int scale = 0;
  // если при сравнении первое число больше
  if (compare == 1) {
    // вычитаем первое из второго
    s21_sub_mantissa(value_1, value_2, result);
    // берем знак из первого
    s21_set_sign(result, sign1);
    // и запятую
    scale = s21_get_scale(value_1);
    s21_set_scale(result, scale);
    // наоборот
  } else if (compare == -1) {
    s21_sub_mantissa(value_2, value_1, result);
    s21_set_sign(result, sign2);
    scale = s21_get_scale(value_2);
    s21_set_scale(result, scale);
    // если они одинаковы по модулю, тогда в итоге будет 0
  } else {
    *result = (s21_decimal){0};
  }
  return 0;
}