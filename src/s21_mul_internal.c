#include "s21_mul_internal.h"

#include "s21_div10.h"  // div_array_by_10 (внутри reduce_192_to_fit)

void s21_mul_mantissa_192(s21_decimal a, s21_decimal b, unsigned int* product) {
  for (int i = 0; i < 6; i++) product[i] = 0;

  for (int i = 0; i < 3; i++) {
    unsigned long long carry = 0;
    unsigned int ai = (unsigned int)a.bits[i];

    for (int j = 0; j < 3; j++) {
      unsigned int bj = (unsigned int)b.bits[j];
      unsigned long long cur =
          (unsigned long long)ai * bj + product[i + j] + carry;
      product[i + j] = (unsigned int)(cur & 0xFFFFFFFF);
      carry = cur >> 32;
    }

    int k = i + 3;
    while (carry != 0 && k < 6) {
      unsigned long long cur = (unsigned long long)product[k] + carry;
      product[k] = (unsigned int)(cur & 0xFFFFFFFF);
      carry = cur >> 32;
      k++;
    }
  }
}

void s21_copy_192_to_decimal(unsigned int* product, s21_decimal* dst) {
  dst->bits[0] = (int)product[0];
  dst->bits[1] = (int)product[1];
  dst->bits[2] = (int)product[2];
  dst->bits[3] = 0;
}

int s21_is_192_fits_96(unsigned int* arr) {
  return arr[3] == 0 && arr[4] == 0 && arr[5] == 0;
}

void s21_reduce_192_to_fit(unsigned int* product, int* scale, int* last_rem,
                           int* has_tail) {
  *last_rem = 0;
  *has_tail = 0;
  int first_rem = -1;

  while ((!s21_is_192_fits_96(product) || *scale > 28) && *scale > 0) {
    int rem = s21_div_array_by_10(product, 6);
    if (first_rem < 0) {
      first_rem = rem;
    } else if (rem != 0) {
      *has_tail = 1;
    }
    (*scale)--;
  }
  *last_rem = (first_rem < 0) ? 0 : first_rem;
}