#include "s21_decimal.h"

#include <math.h>   // isnan, isinf
#include <stdio.h>  // snprintf

#include "s21_add_internal.h"    // add_equal_signs, add_different_signs
#include "s21_bits.h"            // get/set sign/scale, check_zero
#include "s21_div10.h"           // div_mantissa_by_10 (в truncate/floor/round)
#include "s21_div_internal.h"    // div_mantissa_96, div_frac_part, div_round
#include "s21_float_internal.h"  // parse_float_string, float_apply_to_decimal
#include "s21_mantissa.h"        // compare, shift, add/sub mantissa
#include "s21_mul_internal.h"    // mul_192, reduce_192_to_fit
#include "s21_rounding.h"        // apply_bank_rounding
#include "s21_scale.h"           // align_scale, reduce_scale, remove_zeros

// Сравнения
// если два числа равны
int s21_is_equal(s21_decimal value_1, s21_decimal value_2) {
  int result = 0;
  int compare_result = 0;
  // если обе мантиссы нулевые - значит оба нули, заканчиваем функцию
  if (s21_check_zero_divisor(value_1) == 0 &&
      s21_check_zero_divisor(value_2) == 0) {
    result = 1;
    // если знаки у обоих одинковые сравниваем дальше
  } else if (s21_get_sign(value_1) == s21_get_sign(value_2)) {
    // убираем лишние нули
    s21_remove_zeros(&value_1);
    s21_remove_zeros(&value_2);
    // сравниваем скейлы
    if (s21_get_scale(value_1) == s21_get_scale(value_2)) {
      // сравниваем мантиссы
      compare_result = s21_compare_mantissa(value_1, value_2);
      // 0 - одинаковые, 1 - первая больше, -1 - первая меньше
      if (compare_result == 0) {
        result = 1;
      }
    }
  }
  return result;
}

// если два числа не равны
int s21_is_not_equal(s21_decimal value_1, s21_decimal value_2) {
  int result = 0;
  int check = s21_is_equal(value_1, value_2);

  if (check == 0) {
    result = 1;
  }
  return result;
}

// если первое число меньше второго
int s21_is_less(s21_decimal value_1, s21_decimal value_2) {
  int result = 0;
  int compare_mantissa = 0;
  int align_result = 0;

  if (s21_check_zero_divisor(value_1) == 0 &&
      s21_check_zero_divisor(value_2) == 0) {
    result = 0;
  } else if (s21_get_sign(value_1) == 1 && s21_get_sign(value_2) == 0) {
    result = 1;
  } else if (s21_get_sign(value_1) == 0 && s21_get_sign(value_2) == 1) {
    result = 0;
  } else {
    s21_remove_zeros(&value_1);
    s21_remove_zeros(&value_2);
    align_result = s21_align_scale(&value_1, &value_2);
    if (align_result == 0) {
      compare_mantissa = s21_compare_mantissa(value_1, value_2);
      if (compare_mantissa == -1 && s21_get_sign(value_1) == 0) {
        result = 1;
      } else if (compare_mantissa == 1 && s21_get_sign(value_1) == 1) {
        result = 1;
      }
    } else if ((s21_get_scale(value_1) > s21_get_scale(value_2)) &&
               s21_get_sign(value_1) == 0) {
      result = 1;
    } else if ((s21_get_scale(value_1) < s21_get_scale(value_2)) &&
               s21_get_sign(value_1) == 1) {
      result = 1;
    }
  }
  return result;
}

// если первое меньше или равно второму
int s21_is_less_or_equal(s21_decimal value_1, s21_decimal value_2) {
  int result = 0;
  int check1 = s21_is_equal(value_1, value_2);
  int check2 = s21_is_less(value_1, value_2);

  if (check1 == 1 || check2 == 1) {
    result = 1;
  }
  return result;
}

// если первое больше второго
int s21_is_greater(s21_decimal value_1, s21_decimal value_2) {
  int result = 0;
  int check1 = s21_is_less(value_1, value_2);
  int check2 = s21_is_not_equal(value_1, value_2);

  if (check1 == 0 && check2 == 1) {
    result = 1;
  }

  return result;
}

// если первое больше или равно второму
int s21_is_greater_or_equal(s21_decimal value_1, s21_decimal value_2) {
  int result = 0;
  int check1 = s21_is_greater(value_1, value_2);
  int check2 = s21_is_equal(value_1, value_2);

  if (check1 == 1 || check2 == 1) {
    result = 1;
  }

  return result;
}

// Другие
// меняем знак числа на противоположный
int s21_negate(s21_decimal value, s21_decimal* result) {
  int error = 0;
  if (result == NULL) {
    error = 1;
  } else {
    *result = value;
    int sign = !s21_get_sign(value);
    s21_set_sign(result, sign);
  }
  return error;
}

// отбрасываем остаток
int s21_truncate(s21_decimal value, s21_decimal* result) {
  int error = 0;
  if (result == NULL) {
    error = 1;
  } else {
    *result = value;
    int scale = s21_get_scale(value);
    while (scale != 0) {
      s21_div_mantissa_by_10(*result, result);
      scale--;
      s21_set_scale(result, scale);
    }
  }
  return error;
}

// округление отрицательного числа
int s21_floor(s21_decimal value, s21_decimal* result) {
  int error = 0;
  int has_fract = 0;
  int remainder = 0;
  s21_decimal temp = {0};

  if (result == NULL) {
    error = 1;
  } else {
    int sign = s21_get_sign(value);
    temp = value;
    int scale = s21_get_scale(temp);

    while (scale != 0) {
      remainder = s21_div_mantissa_by_10(temp, &temp);
      if (remainder != 0) has_fract = 1;
      scale--;
    }

    s21_truncate(value, result);

    if (has_fract != 0 && sign == 1) {
      error = s21_increment(result);
    }
  }

  return error;
}

// округление до ближайшего целого
int s21_round(s21_decimal value, s21_decimal* result) {
  int error = 0;
  int scale = 0;
  int remainder = 0;
  int has_tail = 0;
  s21_decimal temp = value;

  if (result == NULL) {
    error = 1;
  } else {
    scale = s21_get_scale(value);
    if (scale == 0) {
      *result = value;
    } else {
      remainder = s21_get_round_remainder(&temp, scale, &has_tail);
      s21_set_scale(&temp, 0);
      error = s21_apply_bank_rounding(&temp, remainder, has_tail);
      *result = temp;
    }
  }

  return error;
}

// Арифметические
// складывание децимал
int s21_add(s21_decimal value_1, s21_decimal value_2, s21_decimal* result) {
  int error = 0;
  int align = 0;
  s21_decimal temp1 = value_1;
  s21_decimal temp2 = value_2;
  if (result == NULL) {
    error = -1;
  } else {
    int sign1 = s21_get_sign(temp1);
    int sign2 = s21_get_sign(temp2);
    // убираем лишние нули после запятой, если есть
    s21_remove_zeros(&temp1);
    s21_remove_zeros(&temp2);
    // пытаемся уравнять запятые через умножение на 10
    align = s21_align_scale(&temp1, &temp2);
    // если удачно и знаки одинаковые
    if (align == 0 && sign1 == sign2) {
      error = s21_add_equal_signs(temp1, temp2, result);
      // если удачно и знаки разные
    } else if (align == 0 && sign1 != sign2) {
      error = s21_add_different_signs(temp1, temp2, result);
      // если неудачно пытаемся уравнять знаки через деление на 10
    } else if (align != 0) {
      int scale1 = s21_get_scale(temp1);
      int scale2 = s21_get_scale(temp2);

      // берем более высокую запятую и гоним ее вниз
      if (scale1 < scale2) {
        error = s21_reduce_scale(&temp2, scale1);
      } else {
        error = s21_reduce_scale(&temp1, scale2);
      }
      // если все удачно и знаки равны
      if (error == 0 && sign1 == sign2) {
        error = s21_add_equal_signs(temp1, temp2, result);
        // если все удачно и знаки не равны
      } else if (error == 0 && sign1 != sign2) {
        error = s21_add_different_signs(temp1, temp2, result);
      } else if (error == 1 && sign1 == 1 && sign2 == 1) {
        error = 2;
      }
    }
  }
  return error;
}

// вычитание децимал
int s21_sub(s21_decimal value_1, s21_decimal value_2, s21_decimal* result) {
  int error = 0;
  if (result == NULL) {
    error = -1;
  } else {
    // меняем знак второго децимала на минус и делаем сложение как в пред
    // функции
    s21_negate(value_2, &value_2);
    error = s21_add(value_1, value_2, result);
  }
  return error;
}

// умножение
static int s21_mul_impl(s21_decimal value_1, s21_decimal value_2,
                        s21_decimal* result) {
  int error = 0;
  int sign = s21_get_sign(value_1) ^ s21_get_sign(value_2);
  int scale = s21_get_scale(value_1) + s21_get_scale(value_2);

  unsigned int product[6];
  s21_mul_mantissa_192(value_1, value_2, product);

  int last_rem = 0;
  int has_tail = 0;
  s21_reduce_192_to_fit(product, &scale, &last_rem, &has_tail);

  if (!s21_is_192_fits_96(product)) {
    *result = (s21_decimal){0};
    error = (sign == 0) ? 1 : 2;
  } else {
    s21_copy_192_to_decimal(product, result);
    s21_apply_bank_rounding(result, last_rem, has_tail);
    s21_set_scale(result, scale);
    s21_set_sign(result, sign);
    s21_remove_zeros(result);
  }

  return error;
}

int s21_mul(s21_decimal value_1, s21_decimal value_2, s21_decimal* result) {
  int error = 0;

  if (result == NULL) {
    error = 1;
  } else {
    *result = (s21_decimal){0};

    if (s21_check_zero_divisor(value_1) == 0 ||
        s21_check_zero_divisor(value_2) == 0) {
      error = 0;
    } else {
      s21_remove_zeros(&value_1);
      s21_remove_zeros(&value_2);
      error = s21_mul_impl(value_1, value_2, result);
    }
  }

  return error;
}

// деление
static int s21_div_impl(s21_decimal value_1, s21_decimal value_2,
                        s21_decimal* result) {
  int error = 0;
  int sign = s21_get_sign(value_1) ^ s21_get_sign(value_2);

  int scale_1 = s21_get_scale(value_1);
  int scale_2 = s21_get_scale(value_2);

  while (scale_1 < scale_2) {
    if (s21_mul_mantissa_by_10(value_1, &value_1) != 0) {
      error = (sign == 0) ? 1 : 2;
    }
    scale_1++;
    s21_set_scale(&value_1, scale_1);
  }
  while (scale_2 < scale_1) {
    if (s21_mul_mantissa_by_10(value_2, &value_2) != 0) {
      error = (sign == 0) ? 1 : 2;
    }
    scale_2++;
    s21_set_scale(&value_2, scale_2);
  }

  if (error == 0) {
    s21_decimal quotient = {0};
    s21_decimal remainder = {0};
    s21_div_mantissa_96(value_1, value_2, &quotient, &remainder);

    int scale = 0;
    s21_div_frac_part(&quotient, &remainder, value_2, &scale);
    s21_div_round(&quotient, remainder, value_2);

    *result = quotient;
    s21_set_scale(result, scale);
    s21_set_sign(result, sign);
    s21_remove_zeros(result);
  } else {
    *result = (s21_decimal){0};
  }

  return error;
}

int s21_div(s21_decimal value_1, s21_decimal value_2, s21_decimal* result) {
  int error = 0;

  if (result == NULL) {
    error = 1;
  } else {
    *result = (s21_decimal){0};

    if (s21_check_zero_divisor(value_2) == 0) {
      error = 3;
    } else if (s21_check_zero_divisor(value_1) == 0) {
      error = 0;
    } else {
      s21_remove_zeros(&value_1);
      s21_remove_zeros(&value_2);
      error = s21_div_impl(value_1, value_2, result);
    }
  }

  return error;
}

// преобразователи
int s21_from_int_to_decimal(int src, s21_decimal* dst) {
  int error = 0;
  if (dst == NULL) {
    error = 1;
  } else {
    *dst = (s21_decimal){0};
    if (src < 0) {
      s21_set_sign(dst, 1);
    }

    long long magnitude = src;
    if (magnitude < 0) magnitude = -magnitude;
    dst->bits[0] = (int)(unsigned int)magnitude;
  }

  return error;
}

int s21_from_float_to_decimal(float src, s21_decimal* dst) {
  int error = 0;
  if (dst == NULL || (isnan(src) || isinf(src))) {
    error = 1;
  } else {
    *dst = (s21_decimal){0};

    int sign = 0;
    float abs_src = src;
    if (src < 0) {
      sign = 1;
      abs_src = -src;
    }

    if (abs_src == 0.0f) {
    } else if (abs_src < 1e-28f ||
               abs_src > 7.9228162514264337593543950335e28f) {
      error = 1;
    } else {
      char buf[32] = {0};
      snprintf(buf, sizeof(buf), "%.6e", abs_src);

      long long mantissa = 0;
      int exp = 0;
      s21_parse_float_string(buf, &mantissa, &exp);
      s21_float_remove_trailing_zeros(&mantissa, &exp);

      error = s21_float_apply_to_decimal(mantissa, exp, sign, dst);
    }
  }

  return error;
}

int s21_from_decimal_to_int(s21_decimal src, int* dst) {
  int error = 0;
  if (dst == NULL) {
    error = 1;
  } else {
    s21_decimal temp = src;
    s21_truncate(src, &temp);

    if (temp.bits[1] != 0 || temp.bits[2] != 0) {
      error = 1;
    } else {
      long long result = (long long)(unsigned int)temp.bits[0];
      if (s21_get_sign(temp) == 1) {
        result = -result;
      }
      if (result > 2147483647LL || result < -2147483648LL) {
        error = 1;
      } else {
        *dst = (int)result;
      }
    }
  }
  return error;
}

int s21_from_decimal_to_float(s21_decimal src, float* dst) {
  int error = 0;
  if (dst == NULL) {
    error = 1;
  } else {
    long double mantissa = 0.0L;
    mantissa += (long double)(unsigned int)src.bits[0];
    mantissa += (long double)(unsigned int)src.bits[1] * 4294967296.0L;
    mantissa +=
        (long double)(unsigned int)src.bits[2] * 18446744073709551616.0L;

    int scale = s21_get_scale(src);
    long double divisor = 1.0L;
    for (int i = 0; i < scale; i++) {
      divisor *= 10.0L;
    }
    long double result = mantissa / divisor;

    if (s21_get_sign(src) == 1) {
      result = -result;
    }

    *dst = (float)result;
  }

  return error;
}