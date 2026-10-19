#ifndef S21_MUL_INTERNAL_H_
#define S21_MUL_INTERNAL_H_

#include "s21_decimal.h"

void s21_mul_mantissa_192(s21_decimal a, s21_decimal b, unsigned int* product);
void s21_copy_192_to_decimal(unsigned int* product, s21_decimal* dst);
int s21_is_192_fits_96(unsigned int* arr);
void s21_reduce_192_to_fit(unsigned int* product, int* scale, int* last_rem,
                           int* has_tail);

#endif