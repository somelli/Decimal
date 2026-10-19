#include "s21_float_internal.h"

#include "s21_bits.h"  // set_scale, set_sign

void s21_parse_float_string(const char* buf, long long* mantissa, int* exp) {
  *mantissa = 0;
  *exp = 0;
  int i = 0;
  int exp_sign = 1;

  while (buf[i] != '\0' && buf[i] != 'e') {
    if (buf[i] >= '0' && buf[i] <= '9') {
      *mantissa = *mantissa * 10 + (buf[i] - '0');
    }
    i++;
  }

  if (buf[i] == 'e') {
    i++;
    if (buf[i] == '-') {
      exp_sign = -1;
      i++;
    } else if (buf[i] == '+') {
      i++;
    }
    while (buf[i] >= '0' && buf[i] <= '9') {
      *exp = *exp * 10 + (buf[i] - '0');
      i++;
    }
    *exp *= exp_sign;
  }
}

void s21_float_remove_trailing_zeros(long long* mantissa, int* exp) {
  while (*mantissa != 0 && *mantissa % 10 == 0) {
    *mantissa /= 10;
    *exp += 1;
  }
}

int s21_float_apply_to_decimal(long long mantissa, int exp, int sign,
                               s21_decimal* dst) {
  int error = 0;
  int scale = 6 - exp;

  while (scale < 0) {
    if (mantissa > 0x0CCCCCCCCCCCCCCCLL) {
      error = 1;
    }
    mantissa *= 10;
    scale++;
  }

  if (scale > 28) {
    error = 1;
  } else {
    dst->bits[0] = (int)(mantissa & 0xFFFFFFFF);
    dst->bits[1] = (int)((mantissa >> 32) & 0xFFFFFFFF);
    dst->bits[2] = 0;
    s21_set_scale(dst, scale);
    s21_set_sign(dst, sign);
  }
  return error;
}