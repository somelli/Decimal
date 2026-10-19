#ifndef S21_SCALE_H_
#define S21_SCALE_H_

#include "s21_decimal.h"

int s21_align_scale(s21_decimal* value_1, s21_decimal* value_2);
int s21_reduce_scale(s21_decimal* value, int new_scale);
int s21_remove_zeros(s21_decimal* value);
int s21_get_round_remainder(s21_decimal* value, int scale, int* has_tail);

#endif