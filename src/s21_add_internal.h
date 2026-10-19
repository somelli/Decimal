#ifndef S21_ADD_INTERNAL_H_
#define S21_ADD_INTERNAL_H_

#include "s21_decimal.h"

int s21_add_equal_signs(s21_decimal value_1, s21_decimal value_2,
                        s21_decimal* result);
int s21_add_different_signs(s21_decimal value_1, s21_decimal value_2,
                            s21_decimal* result);

#endif