#ifndef S21_DIV10_H_
#define S21_DIV10_H_

#include "s21_decimal.h"

int s21_div_array_by_10_ex(unsigned int* arr, int size,
                           unsigned long long init_rem);
int s21_div_array_by_10(unsigned int* arr, int size);
int s21_div_mantissa_by_10(s21_decimal value, s21_decimal* result);
int s21_div_overflow_mantissa_by_10(s21_decimal* value);

#endif