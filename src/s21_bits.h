#ifndef S21_BITS_H_
#define S21_BITS_H_

#include "s21_decimal.h"

int s21_get_bit(s21_decimal value, int index);
int s21_set_bit(s21_decimal* value, int index, int bit);
int s21_get_sign(s21_decimal value);
int s21_set_sign(s21_decimal* value, int bit);
int s21_get_scale(s21_decimal value);
int s21_set_scale(s21_decimal* value, unsigned int scale);
int s21_check_zero_divisor(s21_decimal value);

#endif