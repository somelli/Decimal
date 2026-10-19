#ifndef S21_ROUNDING_H_
#define S21_ROUNDING_H_

#include "s21_decimal.h"

int s21_apply_bank_rounding(s21_decimal* value, int remainder, int has_tail);

#endif