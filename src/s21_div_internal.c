#include "s21_div_internal.h"

#include "s21_bits.h"      // get_bit, set_bit, check_zero
#include "s21_mantissa.h"  // compare, sub, shift_left, mul_by_10
#include "s21_rounding.h"  // apply_bank_rounding

// деление 96 мантиссы на 96
void s21_div_mantissa_96(s21_decimal dividend, s21_decimal divisor,
                         s21_decimal* quotient, s21_decimal* remainder) {
  *quotient = (s21_decimal){0};
  *remainder = (s21_decimal){0};

  for (int i = 95; i >= 0; i--) {
    s21_shift_left(remainder);

    int bit = s21_get_bit(dividend, i);
    if (bit == 1) {
      s21_set_bit(remainder, 0, 1);
    }

    if (s21_compare_mantissa(*remainder, divisor) >= 0) {
      s21_sub_mantissa(*remainder, divisor, remainder);
      s21_set_bit(quotient, i, 1);
    }
  }
}

// накопление дробной части: добавляем цифры, пока remainder != 0 и scale < 28
void s21_div_frac_part(s21_decimal* quotient, s21_decimal* remainder,
                       s21_decimal divisor, int* scale) {
  int stop = 0;
  while (!stop && s21_check_zero_divisor(*remainder) != 0 && *scale < 28) {
    if (s21_mul_mantissa_by_10(*remainder, remainder) != 0) {
      stop = 1;
    } else {
      int d = 0;
      while (s21_compare_mantissa(*remainder, divisor) >= 0) {
        s21_sub_mantissa(*remainder, divisor, remainder);
        d++;
      }

      s21_decimal temp = *quotient;
      int mul_err = s21_mul_mantissa_by_10(temp, &temp);

      if (mul_err != 0) {
        s21_apply_bank_rounding(quotient, d, 0);
        stop = 1;
      } else {
        s21_decimal digit = {0};
        digit.bits[0] = d;
        s21_add_mantissa(temp, digit, &temp);
        *quotient = temp;
        (*scale)++;
      }
    }
  }
}

void s21_div_round(s21_decimal* quotient, s21_decimal remainder,
                   s21_decimal divisor) {
  if (s21_check_zero_divisor(remainder) == 0) return;

  s21_decimal rem2 = remainder;
  s21_shift_left(&rem2);
  int cmp = s21_compare_mantissa(rem2, divisor);

  int rem_digit = 4;  // < 0.5
  if (cmp > 0) {
    rem_digit = 6;  // > 0.5
  } else if (cmp == 0) {
    rem_digit = 5;  // ровно 0.5
  }

  s21_apply_bank_rounding(quotient, rem_digit, 0);
}