#include "s21_rounding.h"

#include "s21_mantissa.h"  // s21_increment

int s21_apply_bank_rounding(s21_decimal* value, int remainder, int has_tail) {
  int error = 0;
  int round_up = 0;

  if (remainder > 5) {
    round_up = 1;
  } else if (remainder == 5) {
    if (has_tail) {
      round_up = 1;
    } else {
      round_up = (unsigned int)value->bits[0] & 1u;
    }
  }

  if (round_up) {
    error = s21_increment(value);
  }

  return error;
}