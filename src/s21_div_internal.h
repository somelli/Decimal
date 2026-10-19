#ifndef S21_DIV_INTERNAL_H_
#define S21_DIV_INTERNAL_H_

#include "s21_decimal.h"

void s21_div_mantissa_96(s21_decimal dividend, s21_decimal divisor,
                         s21_decimal* quotient, s21_decimal* remainder);
void s21_div_frac_part(s21_decimal* quotient, s21_decimal* remainder,
                       s21_decimal divisor, int* scale);
void s21_div_round(s21_decimal* quotient, s21_decimal remainder,
                   s21_decimal divisor);

#endif