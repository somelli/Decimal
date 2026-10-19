#ifndef S21_FLOAT_INTERNAL_H_
#define S21_FLOAT_INTERNAL_H_

#include "s21_decimal.h"

void s21_parse_float_string(const char* buf, long long* mantissa, int* exp);
void s21_float_remove_trailing_zeros(long long* mantissa, int* exp);
int s21_float_apply_to_decimal(long long mantissa, int exp, int sign,
                               s21_decimal* dst);

#endif