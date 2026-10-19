#ifndef S21_MANTISSA_H_
#define S21_MANTISSA_H_

#include "s21_decimal.h"

int s21_compare_mantissa(s21_decimal value_1, s21_decimal value_2);
int s21_shift_left(s21_decimal* value);
int s21_add_mantissa(s21_decimal value_1, s21_decimal value_2,
                     s21_decimal* result);
int s21_sub_mantissa(s21_decimal value_1, s21_decimal value_2,
                     s21_decimal* result);
int s21_increment(s21_decimal* value);
int s21_mul_mantissa_by_10(s21_decimal value, s21_decimal* result);

#endif