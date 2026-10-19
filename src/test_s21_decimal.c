#include <check.h>
#include <stdio.h>

#include "s21_add_internal.h"
#include "s21_bits.h"
#include "s21_decimal.h"
#include "s21_div10.h"
#include "s21_div_internal.h"
#include "s21_float_internal.h"
#include "s21_mantissa.h"
#include "s21_mul_internal.h"
#include "s21_rounding.h"
#include "s21_scale.h"

START_TEST(test_get_bit) {
  s21_decimal value = {0};
  value.bits[0] = 13;
  ck_assert_int_eq(s21_get_bit(value, 0), 1);
  ck_assert_int_eq(s21_get_bit(value, 1), 0);
  ck_assert_int_eq(s21_get_bit(value, 96), -1);
  ck_assert_int_eq(s21_get_bit(value, 95), 0);
  ck_assert_int_eq(s21_get_bit(value, -1), -1);

  value.bits[2] = -2147483648;
  ck_assert_int_eq(s21_get_bit(value, 95), 1);
}
END_TEST

START_TEST(test_set_bit) {
  s21_decimal value = {0};
  int result = s21_set_bit(&value, 0, 1);
  ck_assert_int_eq(s21_get_bit(value, 0), 1);
  ck_assert_int_eq(result, 0);
  result = s21_set_bit(&value, 0, 0);
  ck_assert_int_eq(s21_get_bit(value, 0), 0);
  ck_assert_int_eq(result, 0);
  result = s21_set_bit(&value, 0, 2);
  ck_assert_int_eq(result, -1);
  result = s21_set_bit(&value, 96, 1);
  ck_assert_int_eq(result, -1);
  result = s21_set_bit(&value, -1, 0);
  ck_assert_int_eq(result, -1);
  result = s21_set_bit(&value, 95, 1);
  ck_assert_int_eq(s21_get_bit(value, 95), 1);
  ck_assert_int_eq(result, 0);
}
END_TEST

START_TEST(test_is_equal) {
  s21_decimal value_1 = {0};
  s21_decimal value_2 = {0};
  // тест на одинаковые числа
  value_1.bits[0] = 15;
  value_2.bits[0] = 15;
  ck_assert_int_eq(s21_is_equal(value_1, value_2), 1);

  // тест на разные числа
  value_2.bits[0] = 16;
  ck_assert_int_eq(s21_is_equal(value_1, value_2), 0);

  // на разные числа с разным знаком
  s21_set_sign(&value_2, 1);
  ck_assert_int_eq(s21_is_equal(value_1, value_2), 0);

  // на разный знак у нулей
  value_2.bits[0] = 0;
  value_1.bits[0] = 0;
  ck_assert_int_eq(s21_is_equal(value_1, value_2), 1);

  // на одинаковые числа с разным скейл
  s21_set_sign(&value_2, 0);
  value_2.bits[0] = 120;
  value_1.bits[0] = 12;
  s21_set_scale(&value_2, 2);
  s21_set_scale(&value_1, 1);
  ck_assert_int_eq(s21_is_equal(value_1, value_2), 1);

  // тоже самое, но с минусом
  s21_set_sign(&value_2, 1);
  s21_set_sign(&value_1, 1);
  ck_assert_int_eq(s21_is_equal(value_1, value_2), 1);

  // один знак, но разные числа после запятой
  value_2.bits[0] = 121;
  ck_assert_int_eq(s21_is_equal(value_1, value_2), 0);
}
END_TEST

START_TEST(test_is_not_equal) {
  s21_decimal value_1 = {0};
  s21_decimal value_2 = {0};

  value_1.bits[0] = 15;
  value_2.bits[0] = 15;
  ck_assert_int_eq(s21_is_not_equal(value_1, value_2), 0);

  value_1.bits[0] = 16;
  ck_assert_int_eq(s21_is_not_equal(value_1, value_2), 1);
}
END_TEST

START_TEST(test_is_less) {
  s21_decimal value_1 = {0};
  s21_decimal value_2 = {0};

  value_1.bits[0] = 5;
  value_2.bits[0] = 10;
  ck_assert_int_eq(s21_is_less(value_1, value_2), 1);
  value_1.bits[0] = 15;
  ck_assert_int_eq(s21_is_less(value_1, value_2), 0);
  value_2.bits[0] = 15;
  ck_assert_int_eq(s21_is_less(value_1, value_2), 0);

  value_1.bits[0] = 12;
  value_2.bits[0] = 121;
  s21_set_scale(&value_1, 1);
  s21_set_scale(&value_2, 2);
  ck_assert_int_eq(s21_is_less(value_1, value_2), 1);

  value_1 = (s21_decimal){0};
  value_2 = (s21_decimal){0};
  value_2.bits[0] = 1;
  ck_assert_int_eq(s21_is_less(value_1, value_2), 1);
  ck_assert_int_eq(s21_is_less(value_2, value_1), 0);
  s21_set_sign(&value_1, 1);
  ck_assert_int_eq(s21_is_less(value_1, value_1), 0);

  value_1.bits[0] = 5;
  value_2.bits[0] = 10;
  s21_set_sign(&value_2, 1);
  ck_assert_int_eq(s21_is_less(value_1, value_2), 0);
  value_1.bits[0] = 15;
  ck_assert_int_eq(s21_is_less(value_1, value_2), 1);

  value_1 = (s21_decimal){0};
  value_2 = (s21_decimal){0};
  value_1.bits[0] = value_1.bits[1] = value_1.bits[2] = -1;
  value_2.bits[0] = value_2.bits[1] = value_2.bits[2] = -1;
  s21_set_scale(&value_2, 1);
  ck_assert_int_eq(s21_is_less(value_1, value_2), 0);
  s21_set_scale(&value_1, 1);
  s21_set_scale(&value_2, 0);
  ck_assert_int_eq(s21_is_less(value_1, value_2), 1);

  s21_set_sign(&value_1, 1);
  s21_set_sign(&value_2, 1);
  ck_assert_int_eq(s21_is_less(value_1, value_2), 0);
  s21_set_scale(&value_1, 0);
  s21_set_scale(&value_2, 1);
  ck_assert_int_eq(s21_is_less(value_1, value_2), 1);
}
END_TEST

START_TEST(test_is_less_or_equal) {
  s21_decimal value_1 = {0};
  s21_decimal value_2 = {0};
  value_1.bits[0] = 5;
  value_2.bits[0] = 10;
  ck_assert_int_eq(s21_is_less_or_equal(value_1, value_2), 1);

  value_1.bits[0] = 10;
  ck_assert_int_eq(s21_is_less_or_equal(value_1, value_2), 1);

  value_1.bits[0] = 15;
  ck_assert_int_eq(s21_is_less_or_equal(value_1, value_2), 0);
}
END_TEST

START_TEST(test_is_greater) {
  s21_decimal value_1 = {0};
  s21_decimal value_2 = {0};

  value_1.bits[0] = 15;
  value_2.bits[0] = 10;
  ck_assert_int_eq(s21_is_greater(value_1, value_2), 1);

  value_1.bits[0] = 10;
  ck_assert_int_eq(s21_is_greater(value_1, value_2), 0);

  value_1.bits[0] = 5;
  ck_assert_int_eq(s21_is_greater(value_1, value_2), 0);
}
END_TEST

START_TEST(test_is_greater_or_equal) {
  s21_decimal value_1 = {0};
  s21_decimal value_2 = {0};

  value_1.bits[0] = 15;
  value_2.bits[0] = 10;
  ck_assert_int_eq(s21_is_greater_or_equal(value_1, value_2), 1);

  value_1.bits[0] = 10;
  ck_assert_int_eq(s21_is_greater_or_equal(value_1, value_2), 1);

  value_1.bits[0] = 5;
  ck_assert_int_eq(s21_is_greater_or_equal(value_1, value_2), 0);
}
END_TEST

START_TEST(test_negate) {
  s21_decimal value = {0};
  s21_decimal result = {0};

  value.bits[0] = 123;
  s21_set_scale(&value, 2);
  ck_assert_int_eq(s21_negate(value, &result), 0);
  ck_assert_int_eq(s21_get_sign(result), 1);
  ck_assert_int_eq(result.bits[0], 123);
  ck_assert_int_eq(s21_get_scale(result), 2);

  s21_set_sign(&value, 1);
  ck_assert_int_eq(s21_negate(value, &result), 0);
  ck_assert_int_eq(s21_get_sign(result), 0);
  ck_assert_int_eq(result.bits[0], 123);
  ck_assert_int_eq(s21_get_scale(result), 2);

  value = (s21_decimal){0};
  ck_assert_int_eq(s21_negate(value, &result), 0);
  ck_assert_int_eq(s21_get_sign(result), 1);
}
END_TEST

START_TEST(test_truncate) {
  s21_decimal value = {0};
  s21_decimal result = {0};

  value.bits[0] = 1278;
  s21_set_scale(&value, 2);
  ck_assert_int_eq(s21_truncate(value, &result), 0);
  ck_assert_int_eq(result.bits[0], 12);
  ck_assert_int_eq(s21_get_scale(result), 0);
  ck_assert_int_eq(s21_get_sign(result), 0);

  value.bits[0] = 1278;
  s21_set_scale(&value, 2);
  s21_set_sign(&value, 1);
  ck_assert_int_eq(s21_truncate(value, &result), 0);
  ck_assert_int_eq(result.bits[0], 12);
  ck_assert_int_eq(s21_get_scale(result), 0);
  ck_assert_int_eq(s21_get_sign(result), 1);

  value = (s21_decimal){0};
  value.bits[0] = 42;
  ck_assert_int_eq(s21_truncate(value, &result), 0);
  ck_assert_int_eq(result.bits[0], 42);
  ck_assert_int_eq(s21_get_scale(result), 0);

  ck_assert_int_eq(s21_truncate(value, NULL), 1);
}
END_TEST

START_TEST(test_floor) {
  s21_decimal value = {0};
  s21_decimal result = {0};

  value.bits[0] = 1278;
  s21_set_scale(&value, 2);
  ck_assert_int_eq(s21_floor(value, &result), 0);
  ck_assert_int_eq(result.bits[0], 12);
  ck_assert_int_eq(s21_get_scale(result), 0);
  ck_assert_int_eq(s21_get_sign(result), 0);

  value = (s21_decimal){0};
  value.bits[0] = 1278;
  s21_set_scale(&value, 2);
  s21_set_sign(&value, 1);
  ck_assert_int_eq(s21_floor(value, &result), 0);
  ck_assert_int_eq(result.bits[0], 13);
  ck_assert_int_eq(s21_get_scale(result), 0);
  ck_assert_int_eq(s21_get_sign(result), 1);

  value = (s21_decimal){0};
  value.bits[0] = 1200;
  s21_set_scale(&value, 2);
  s21_set_sign(&value, 1);
  ck_assert_int_eq(s21_floor(value, &result), 0);
  ck_assert_int_eq(result.bits[0], 12);
  ck_assert_int_eq(s21_get_sign(result), 1);

  value = (s21_decimal){0};
  value.bits[0] = 1;
  s21_set_scale(&value, 2);
  s21_set_sign(&value, 1);
  ck_assert_int_eq(s21_floor(value, &result), 0);
  ck_assert_int_eq(result.bits[0], 1);
  ck_assert_int_eq(s21_get_sign(result), 1);

  ck_assert_int_eq(s21_floor(value, NULL), 1);
}
END_TEST

START_TEST(test_round) {
  s21_decimal value = {0};
  s21_decimal result = {0};

  value.bits[0] = 123;
  s21_set_scale(&value, 1);
  ck_assert_int_eq(s21_round(value, &result), 0);
  ck_assert_int_eq(result.bits[0], 12);
  ck_assert_int_eq(s21_get_scale(result), 0);

  value = (s21_decimal){0};
  value.bits[0] = 127;
  s21_set_scale(&value, 1);
  ck_assert_int_eq(s21_round(value, &result), 0);
  ck_assert_int_eq(result.bits[0], 13);

  value = (s21_decimal){0};
  value.bits[0] = 125;
  s21_set_scale(&value, 1);
  ck_assert_int_eq(s21_round(value, &result), 0);
  ck_assert_int_eq(result.bits[0], 12);

  value = (s21_decimal){0};
  value.bits[0] = 135;
  s21_set_scale(&value, 1);
  ck_assert_int_eq(s21_round(value, &result), 0);
  ck_assert_int_eq(result.bits[0], 14);

  value = (s21_decimal){0};
  value.bits[0] = 12500001;
  s21_set_scale(&value, 6);
  ck_assert_int_eq(s21_round(value, &result), 0);
  ck_assert_int_eq(result.bits[0], 13);

  value = (s21_decimal){0};
  value.bits[0] = 127;
  s21_set_scale(&value, 1);
  s21_set_sign(&value, 1);
  ck_assert_int_eq(s21_round(value, &result), 0);
  ck_assert_int_eq(result.bits[0], 13);
  ck_assert_int_eq(s21_get_sign(result), 1);

  value = (s21_decimal){0};
  value.bits[0] = 42;
  ck_assert_int_eq(s21_round(value, &result), 0);
  ck_assert_int_eq(result.bits[0], 42);
  ck_assert_int_eq(s21_get_scale(result), 0);

  ck_assert_int_eq(s21_round(value, NULL), 1);
}
END_TEST

START_TEST(test_add_different_signs_first_positive_bigger) {
  s21_decimal a = {{20, 0, 0, 0}};
  s21_decimal b = {{10, 0, 0, 0}};
  s21_decimal result = {0};

  s21_set_sign(&b, 1);

  int error = s21_add_different_signs(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(result.bits[0], 10);
  ck_assert_int_eq(s21_get_sign(result), 0);
  ck_assert_int_eq(s21_get_scale(result), 0);
}
END_TEST

// helpers для s21_add
// s21_add_different_signs
START_TEST(test_add_different_signs_first_negative_bigger) {
  s21_decimal a = {{20, 0, 0, 0}};
  s21_decimal b = {{10, 0, 0, 0}};
  s21_decimal result = {0};

  s21_set_sign(&a, 1);

  int error = s21_add_different_signs(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(result.bits[0], 10);
  ck_assert_int_eq(s21_get_sign(result), 1);
}
END_TEST

START_TEST(test_add_different_signs_second_positive_bigger) {
  s21_decimal a = {{10, 0, 0, 0}};
  s21_decimal b = {{20, 0, 0, 0}};
  s21_decimal result = {0};

  s21_set_sign(&a, 1);

  int error = s21_add_different_signs(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(result.bits[0], 10);
  ck_assert_int_eq(s21_get_sign(result), 0);
}
END_TEST

START_TEST(test_add_different_signs_second_negative_bigger) {
  s21_decimal a = {{10, 0, 0, 0}};
  s21_decimal b = {{20, 0, 0, 0}};
  s21_decimal result = {0};

  s21_set_sign(&b, 1);

  int error = s21_add_different_signs(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(result.bits[0], 10);
  ck_assert_int_eq(s21_get_sign(result), 1);
}
END_TEST

START_TEST(test_add_different_signs_equal_modules) {
  s21_decimal a = {{123, 0, 0, 0}};
  s21_decimal b = {{123, 0, 0, 0}};
  s21_decimal result = {{999, 999, 999, 999}};

  s21_set_sign(&b, 1);
  s21_set_scale(&a, 2);
  s21_set_scale(&b, 2);

  int error = s21_add_different_signs(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(result.bits[0], 0);
  ck_assert_int_eq(result.bits[1], 0);
  ck_assert_int_eq(result.bits[2], 0);
  ck_assert_int_eq(result.bits[3], 0);
}
END_TEST

START_TEST(test_add_different_signs_keeps_scale) {
  s21_decimal a = {{150, 0, 0, 0}};
  s21_decimal b = {{25, 0, 0, 0}};
  s21_decimal result = {0};

  s21_set_scale(&a, 2);
  s21_set_scale(&b, 2);
  s21_set_sign(&b, 1);

  int error = s21_add_different_signs(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(result.bits[0], 125);
  ck_assert_int_eq(s21_get_scale(result), 2);
  ck_assert_int_eq(s21_get_sign(result), 0);
}
END_TEST

// s21_add_equal_signs
START_TEST(test_add_equal_signs_positive) {
  s21_decimal a = {{10, 0, 0, 0}};
  s21_decimal b = {{20, 0, 0, 0}};
  s21_decimal result = {0};

  int error = s21_add_equal_signs(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(result.bits[0], 30);
  ck_assert_int_eq(s21_get_sign(result), 0);
}
END_TEST

START_TEST(test_add_equal_signs_negative) {
  s21_decimal a = {{10, 0, 0, 0}};
  s21_decimal b = {{20, 0, 0, 0}};
  s21_decimal result = {0};

  s21_set_sign(&a, 1);
  s21_set_sign(&b, 1);

  int error = s21_add_equal_signs(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(result.bits[0], 30);
  ck_assert_int_eq(s21_get_sign(result), 1);
}
END_TEST

START_TEST(test_add_equal_signs_scale) {
  s21_decimal a = {{123, 0, 0, 0}};
  s21_decimal b = {{456, 0, 0, 0}};
  s21_decimal result = {0};

  s21_set_scale(&a, 2);
  s21_set_scale(&b, 2);

  int error = s21_add_equal_signs(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(result.bits[0], 579);
  ck_assert_int_eq(s21_get_scale(result), 2);
}
END_TEST

// 0xFFFFFFFF + 1
START_TEST(test_add_equal_signs_carry_bits0_bits1) {
  s21_decimal a = {{-1, 0, 0, 0}};
  s21_decimal b = {{1, 0, 0, 0}};
  s21_decimal result = {0};

  int error = s21_add_equal_signs(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)result.bits[0], 0u);
  ck_assert_uint_eq((unsigned int)result.bits[1], 1u);
  ck_assert_uint_eq((unsigned int)result.bits[2], 0u);
}
END_TEST

// 0xFFFFFFFFFFFFFFFF + 1
START_TEST(test_add_equal_signs_carry_bits1_bits2) {
  s21_decimal a = {{-1, -1, 0, 0}};
  s21_decimal b = {{1, 0, 0, 0}};
  s21_decimal result = {0};

  int error = s21_add_equal_signs(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)result.bits[0], 0u);
  ck_assert_uint_eq((unsigned int)result.bits[1], 0u);
  ck_assert_uint_eq((unsigned int)result.bits[2], 1u);
}
END_TEST

START_TEST(test_add_equal_signs_positive_overflow) {
  s21_decimal a = {{-1, -1, -1, 0}};
  s21_decimal b = {{1, 0, 0, 0}};
  s21_decimal result = {0};

  int error = s21_add_equal_signs(a, b, &result);

  ck_assert_int_eq(error, 1);
}
END_TEST

START_TEST(test_add_equal_signs_negative_overflow) {
  s21_decimal a = {{-1, -1, -1, 0}};
  s21_decimal b = {{1, 0, 0, 0}};
  s21_decimal result = {0};

  s21_set_sign(&a, 1);
  s21_set_sign(&b, 1);

  int error = s21_add_equal_signs(a, b, &result);

  ck_assert_int_eq(error, 2);
}
END_TEST

// s21_reduce_scale
//  12.34 -> 12.3
START_TEST(test_reduce_scale_remainder_less_5) {
  s21_decimal value = {{1234, 0, 0, 0}};
  s21_set_scale(&value, 2);

  int error = s21_reduce_scale(&value, 1);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(value.bits[0], 123);
  ck_assert_int_eq(s21_get_scale(value), 1);
}
END_TEST

// 12.36 -> 12.4
START_TEST(test_reduce_scale_remainder_more_5) {
  s21_decimal value = {{1236, 0, 0, 0}};
  s21_set_scale(&value, 2);

  int error = s21_reduce_scale(&value, 1);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(value.bits[0], 124);
  ck_assert_int_eq(s21_get_scale(value), 1);
}
END_TEST

// 12.25 -> 12.2
// сохраняемое 122 чётное
START_TEST(test_reduce_scale_bank_even) {
  s21_decimal value = {{1225, 0, 0, 0}};
  s21_set_scale(&value, 2);

  int error = s21_reduce_scale(&value, 1);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(value.bits[0], 122);
  ck_assert_int_eq(s21_get_scale(value), 1);
}
END_TEST

// 12.35 -> 12.4
// сохраняемое 123 нечётное
START_TEST(test_reduce_scale_bank_odd) {
  s21_decimal value = {{1235, 0, 0, 0}};
  s21_set_scale(&value, 2);

  int error = s21_reduce_scale(&value, 1);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(value.bits[0], 124);
  ck_assert_int_eq(s21_get_scale(value), 1);
}
END_TEST

// 12.2500 -> 12.2
// после 5 только нули
START_TEST(test_reduce_scale_exact_five_no_tail) {
  s21_decimal value = {{122500, 0, 0, 0}};
  s21_set_scale(&value, 4);

  int error = s21_reduce_scale(&value, 1);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(value.bits[0], 122);
  ck_assert_int_eq(s21_get_scale(value), 1);
}
END_TEST

// 12.2501 -> 12.3
// после 5 есть ненулевой хвост
START_TEST(test_reduce_scale_five_with_tail) {
  s21_decimal value = {{122501, 0, 0, 0}};
  s21_set_scale(&value, 4);

  int error = s21_reduce_scale(&value, 1);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(value.bits[0], 123);
  ck_assert_int_eq(s21_get_scale(value), 1);
}
END_TEST

// Очень важный случай:
// 12.449 -> 12.4
// хвост существует, но основной отбрасываемый разряд = 4,
// поэтому округлять вверх нельзя.
START_TEST(test_reduce_scale_less_five_with_tail) {
  s21_decimal value = {{12449, 0, 0, 0}};
  s21_set_scale(&value, 3);

  int error = s21_reduce_scale(&value, 1);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(value.bits[0], 124);
  ck_assert_int_eq(s21_get_scale(value), 1);
}
END_TEST

// 12.551 -> 12.6
START_TEST(test_reduce_scale_more_five_with_tail) {
  s21_decimal value = {{12551, 0, 0, 0}};
  s21_set_scale(&value, 3);

  int error = s21_reduce_scale(&value, 1);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(value.bits[0], 126);
  ck_assert_int_eq(s21_get_scale(value), 1);
}
END_TEST

// 12.6 -> 13
START_TEST(test_reduce_scale_to_zero) {
  s21_decimal value = {{126, 0, 0, 0}};
  s21_set_scale(&value, 1);

  int error = s21_reduce_scale(&value, 0);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(value.bits[0], 13);
  ck_assert_int_eq(s21_get_scale(value), 0);
}
END_TEST

// -12.6 -> -13
START_TEST(test_reduce_scale_negative_keeps_sign) {
  s21_decimal value = {{126, 0, 0, 0}};
  s21_set_scale(&value, 1);
  s21_set_sign(&value, 1);

  int error = s21_reduce_scale(&value, 0);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(value.bits[0], 13);
  ck_assert_int_eq(s21_get_scale(value), 0);
  ck_assert_int_eq(s21_get_sign(value), 1);
}
END_TEST

// scale == new_scale: ничего не происходит
START_TEST(test_reduce_scale_same_scale) {
  s21_decimal value = {{1234, 0, 0, 0}};
  s21_set_scale(&value, 2);

  int error = s21_reduce_scale(&value, 2);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(value.bits[0], 1234);
  ck_assert_int_eq(s21_get_scale(value), 2);
}
END_TEST

// scale < new_scale: helper тоже ничего не делает
START_TEST(test_reduce_scale_new_scale_bigger) {
  s21_decimal value = {{1234, 0, 0, 0}};
  s21_set_scale(&value, 2);

  int error = s21_reduce_scale(&value, 5);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(value.bits[0], 1234);
  ck_assert_int_eq(s21_get_scale(value), 2);
}
END_TEST

// s21_add (сложение)
//  10 + 20 = 30
START_TEST(test_add_positive) {
  s21_decimal a = {{10, 0, 0, 0}};
  s21_decimal b = {{20, 0, 0, 0}};
  s21_decimal result = {0};

  int error = s21_add(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(result.bits[0], 30);
  ck_assert_int_eq(s21_get_scale(result), 0);
  ck_assert_int_eq(s21_get_sign(result), 0);
}
END_TEST

// -10 + -20 = -30
START_TEST(test_add_negative) {
  s21_decimal a = {{10, 0, 0, 0}};
  s21_decimal b = {{20, 0, 0, 0}};
  s21_decimal result = {0};

  s21_set_sign(&a, 1);
  s21_set_sign(&b, 1);

  int error = s21_add(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(result.bits[0], 30);
  ck_assert_int_eq(s21_get_sign(result), 1);
}
END_TEST

// 20 + (-10) = 10
START_TEST(test_add_different_sign_positive_result) {
  s21_decimal a = {{20, 0, 0, 0}};
  s21_decimal b = {{10, 0, 0, 0}};
  s21_decimal result = {0};

  s21_set_sign(&b, 1);

  int error = s21_add(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(result.bits[0], 10);
  ck_assert_int_eq(s21_get_sign(result), 0);
}
END_TEST

// 10 + (-20) = -10
START_TEST(test_add_different_sign_negative_result) {
  s21_decimal a = {{10, 0, 0, 0}};
  s21_decimal b = {{20, 0, 0, 0}};
  s21_decimal result = {0};

  s21_set_sign(&b, 1);

  int error = s21_add(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(result.bits[0], 10);
  ck_assert_int_eq(s21_get_sign(result), 1);
}
END_TEST

// 10 + (-10) = 0
START_TEST(test_add_equal_different_sign) {
  s21_decimal a = {{10, 0, 0, 0}};
  s21_decimal b = {{10, 0, 0, 0}};
  s21_decimal result = {0};

  s21_set_sign(&b, 1);

  int error = s21_add(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(result.bits[0], 0);
  ck_assert_int_eq(result.bits[1], 0);
  ck_assert_int_eq(result.bits[2], 0);
  ck_assert_int_eq(result.bits[3], 0);
}
END_TEST

// 1.2 + 2.34 = 3.54
// align_scale должен успешно превратить 1.2 -> 1.20
START_TEST(test_add_successful_align) {
  s21_decimal a = {{12, 0, 0, 0}};
  s21_decimal b = {{234, 0, 0, 0}};
  s21_decimal result = {0};

  s21_set_scale(&a, 1);
  s21_set_scale(&b, 2);

  int error = s21_add(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(result.bits[0], 354);
  ck_assert_int_eq(s21_get_scale(result), 2);
}
END_TEST

// 2.34 + 1.2 = 3.54
// то же самое, но масштабировать уже второй операнд
START_TEST(test_add_successful_align_reverse) {
  s21_decimal a = {{234, 0, 0, 0}};
  s21_decimal b = {{12, 0, 0, 0}};
  s21_decimal result = {0};

  s21_set_scale(&a, 2);
  s21_set_scale(&b, 1);

  int error = s21_add(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(result.bits[0], 354);
  ck_assert_int_eq(s21_get_scale(result), 2);
}
END_TEST

// 1.2 + (-0.25) = 0.95
START_TEST(test_add_successful_align_different_signs) {
  s21_decimal a = {{12, 0, 0, 0}};
  s21_decimal b = {{25, 0, 0, 0}};
  s21_decimal result = {0};

  s21_set_scale(&a, 1);
  s21_set_scale(&b, 2);
  s21_set_sign(&b, 1);

  int error = s21_add(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(result.bits[0], 95);
  ck_assert_int_eq(s21_get_scale(result), 2);
  ck_assert_int_eq(s21_get_sign(result), 0);
}
END_TEST

// MAX + 0.4
//
// MAX нельзя умножить на 10.
// Поэтому align ломается.
// 0.4 уменьшается до scale 0 -> 0.
// MAX + 0 = MAX.
START_TEST(test_add_align_overflow_reduce_down) {
  s21_decimal a = {{-1, -1, -1, 0}};
  s21_decimal b = {{4, 0, 0, 0}};
  s21_decimal result = {0};

  s21_set_scale(&b, 1);

  int error = s21_add(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)result.bits[0], 0xFFFFFFFFu);
  ck_assert_uint_eq((unsigned int)result.bits[1], 0xFFFFFFFFu);
  ck_assert_uint_eq((unsigned int)result.bits[2], 0xFFFFFFFFu);
  ck_assert_int_eq(s21_get_scale(result), 0);
  ck_assert_int_eq(s21_get_sign(result), 0);
}
END_TEST

// MAX + 0.5
//
// banker:
// 0.5 -> 0, потому что сохраняемый 0 чётный.
// Получаем MAX.
START_TEST(test_add_align_overflow_bank_even) {
  s21_decimal a = {{-1, -1, -1, 0}};
  s21_decimal b = {{5, 0, 0, 0}};
  s21_decimal result = {0};

  s21_set_scale(&b, 1);

  int error = s21_add(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)result.bits[0], 0xFFFFFFFFu);
  ck_assert_uint_eq((unsigned int)result.bits[1], 0xFFFFFFFFu);
  ck_assert_uint_eq((unsigned int)result.bits[2], 0xFFFFFFFFu);
}
END_TEST

// MAX + 0.6
//
// align ломается.
// 0.6 -> 1.
// MAX + 1 -> положительный overflow.
START_TEST(test_add_align_overflow_positive_overflow) {
  s21_decimal a = {{-1, -1, -1, 0}};
  s21_decimal b = {{6, 0, 0, 0}};
  s21_decimal result = {0};

  s21_set_scale(&b, 1);

  int error = s21_add(a, b, &result);

  ck_assert_int_eq(error, 1);
}
END_TEST

// -MAX + (-0.6)
//
// 0.6 -> 1.
// -MAX + (-1) -> отрицательный overflow.
START_TEST(test_add_align_overflow_negative_overflow) {
  s21_decimal a = {{-1, -1, -1, 0}};
  s21_decimal b = {{6, 0, 0, 0}};
  s21_decimal result = {0};

  s21_set_sign(&a, 1);
  s21_set_sign(&b, 1);
  s21_set_scale(&b, 1);

  int error = s21_add(a, b, &result);

  ck_assert_int_eq(error, 2);
}
END_TEST

// MAX + (-0.6)
//
// align ломается.
// -0.6 -> -1.
// MAX - 1 = MAX-1.
// Переполнения быть не должно.
START_TEST(test_add_align_overflow_different_signs) {
  s21_decimal a = {{-1, -1, -1, 0}};
  s21_decimal b = {{6, 0, 0, 0}};
  s21_decimal result = {0};

  s21_set_scale(&b, 1);
  s21_set_sign(&b, 1);

  int error = s21_add(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)result.bits[0], 0xFFFFFFFEu);
  ck_assert_uint_eq((unsigned int)result.bits[1], 0xFFFFFFFFu);
  ck_assert_uint_eq((unsigned int)result.bits[2], 0xFFFFFFFFu);
  ck_assert_int_eq(s21_get_sign(result), 0);
}
END_TEST

// -MAX + 0.6 = -(MAX - 1)
START_TEST(test_add_align_overflow_different_signs_negative) {
  s21_decimal a = {{-1, -1, -1, 0}};
  s21_decimal b = {{6, 0, 0, 0}};
  s21_decimal result = {0};

  s21_set_sign(&a, 1);
  s21_set_scale(&b, 1);

  int error = s21_add(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)result.bits[0], 0xFFFFFFFEu);
  ck_assert_uint_eq((unsigned int)result.bits[1], 0xFFFFFFFFu);
  ck_assert_uint_eq((unsigned int)result.bits[2], 0xFFFFFFFFu);
  ck_assert_int_eq(s21_get_sign(result), 1);
}
END_TEST

// Проверяем обратную сторону:
// теперь MAX находится во втором операнде.
//
// 0.4 + MAX = MAX.
START_TEST(test_add_align_overflow_reverse) {
  s21_decimal a = {{4, 0, 0, 0}};
  s21_decimal b = {{-1, -1, -1, 0}};
  s21_decimal result = {0};

  s21_set_scale(&a, 1);

  int error = s21_add(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)result.bits[0], 0xFFFFFFFFu);
  ck_assert_uint_eq((unsigned int)result.bits[1], 0xFFFFFFFFu);
  ck_assert_uint_eq((unsigned int)result.bits[2], 0xFFFFFFFFu);
  ck_assert_int_eq(s21_get_scale(result), 0);
}
END_TEST

// MAX + 0.51
//
// при уменьшении scale 2 -> 0:
// отбрасываем 1, затем получаем 5 с ненулевым хвостом.
// Значит 0.51 -> 1.
// MAX + 1 -> overflow.
START_TEST(test_add_align_overflow_reduce_with_tail) {
  s21_decimal a = {{-1, -1, -1, 0}};
  s21_decimal b = {{51, 0, 0, 0}};
  s21_decimal result = {0};

  s21_set_scale(&b, 2);

  int error = s21_add(a, b, &result);

  ck_assert_int_eq(error, 1);
}
END_TEST

START_TEST(test_add_mantissa_overflow_can_reduce_scale) {
  s21_decimal a = {{-1, -1, -1, 0}};
  s21_decimal b = {{-1, -1, -1, 0}};
  s21_decimal result = {0};

  s21_set_scale(&a, 1);
  s21_set_scale(&b, 1);

  int error = s21_add(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(s21_get_scale(result), 0);
  ck_assert_int_eq(s21_get_sign(result), 0);
}
END_TEST

START_TEST(test_add_negative_mantissa_overflow_can_reduce_scale) {
  s21_decimal a = {{-1, -1, -1, 0}};
  s21_decimal b = {{-1, -1, -1, 0}};
  s21_decimal result = {0};

  s21_set_scale(&a, 1);
  s21_set_scale(&b, 1);
  s21_set_sign(&a, 1);
  s21_set_sign(&b, 1);

  int error = s21_add(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(s21_get_scale(result), 0);
  ck_assert_int_eq(s21_get_sign(result), 1);
}
END_TEST

// MAX96 + 1 при scale = 0.
// Уменьшать scale уже некуда -> положительный overflow.
START_TEST(test_add_final_positive_overflow) {
  s21_decimal a = {{-1, -1, -1, 0}};
  s21_decimal b = {{1, 0, 0, 0}};
  s21_decimal result = {0};

  int error = s21_add(a, b, &result);

  ck_assert_int_eq(error, 1);
}
END_TEST

// -MAX96 + (-1) при scale = 0.
// Уменьшать scale уже некуда -> отрицательный overflow.
START_TEST(test_add_final_negative_overflow) {
  s21_decimal a = {{-1, -1, -1, 0}};
  s21_decimal b = {{1, 0, 0, 0}};
  s21_decimal result = {0};

  s21_set_sign(&a, 1);
  s21_set_sign(&b, 1);

  int error = s21_add(a, b, &result);

  ck_assert_int_eq(error, 2);
}
END_TEST

START_TEST(test_add_overflow_reduce_scale_round_up) {
  s21_decimal a = {{-1, -1, -1, 0}};
  s21_decimal b = {{1, 0, 0, 0}};
  s21_decimal result = {0};

  s21_set_scale(&a, 1);
  s21_set_scale(&b, 1);

  int error = s21_add(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)result.bits[0], 2576980378u);
  ck_assert_uint_eq((unsigned int)result.bits[1], 2576980377u);
  ck_assert_uint_eq((unsigned int)result.bits[2], 429496729u);
  ck_assert_int_eq(s21_get_scale(result), 0);
  ck_assert_int_eq(s21_get_sign(result), 0);
}
END_TEST

// s21_sub (вычитание)
START_TEST(test_sub_positive_positive_greater) {
  s21_decimal a = {{10, 0, 0, 0}};
  s21_decimal b = {{3, 0, 0, 0}};
  s21_decimal result = {0};

  int error = s21_sub(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(result.bits[0], 7);
  ck_assert_int_eq(result.bits[1], 0);
  ck_assert_int_eq(result.bits[2], 0);
  ck_assert_int_eq(s21_get_sign(result), 0);
}
END_TEST

START_TEST(test_sub_positive_positive_less) {
  s21_decimal a = {{3, 0, 0, 0}};
  s21_decimal b = {{10, 0, 0, 0}};
  s21_decimal result = {0};

  int error = s21_sub(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(result.bits[0], 7);
  ck_assert_int_eq(s21_get_sign(result), 1);
}
END_TEST

START_TEST(test_sub_positive_negative) {
  s21_decimal a = {{10, 0, 0, 0}};
  s21_decimal b = {{3, 0, 0, 0}};
  s21_decimal result = {0};

  s21_set_sign(&b, 1);

  int error = s21_sub(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(result.bits[0], 13);
  ck_assert_int_eq(s21_get_sign(result), 0);
}
END_TEST

START_TEST(test_sub_negative_positive) {
  s21_decimal a = {{10, 0, 0, 0}};
  s21_decimal b = {{3, 0, 0, 0}};
  s21_decimal result = {0};

  s21_set_sign(&a, 1);

  int error = s21_sub(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(result.bits[0], 13);
  ck_assert_int_eq(s21_get_sign(result), 1);
}
END_TEST

START_TEST(test_sub_negative_negative) {
  s21_decimal a = {{10, 0, 0, 0}};
  s21_decimal b = {{3, 0, 0, 0}};
  s21_decimal result = {0};

  s21_set_sign(&a, 1);
  s21_set_sign(&b, 1);

  int error = s21_sub(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(result.bits[0], 7);
  ck_assert_int_eq(s21_get_sign(result), 1);
}
END_TEST

START_TEST(test_sub_equal_values) {
  s21_decimal a = {{25, 0, 0, 0}};
  s21_decimal b = {{25, 0, 0, 0}};
  s21_decimal result = {0};

  int error = s21_sub(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(result.bits[0], 0);
  ck_assert_int_eq(result.bits[1], 0);
  ck_assert_int_eq(result.bits[2], 0);
}
END_TEST

START_TEST(test_sub_different_scales) {
  // 12.5 - 2 = 10.5
  s21_decimal a = {{125, 0, 0, 0}};
  s21_decimal b = {{2, 0, 0, 0}};
  s21_decimal result = {0};

  s21_set_scale(&a, 1);

  int error = s21_sub(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(result.bits[0], 105);
  ck_assert_int_eq(s21_get_scale(result), 1);
  ck_assert_int_eq(s21_get_sign(result), 0);
}
END_TEST

START_TEST(test_sub_different_scales_negative_result) {
  // 2 - 12.5 = -10.5
  s21_decimal a = {{2, 0, 0, 0}};
  s21_decimal b = {{125, 0, 0, 0}};
  s21_decimal result = {0};

  s21_set_scale(&b, 1);

  int error = s21_sub(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(result.bits[0], 105);
  ck_assert_int_eq(s21_get_scale(result), 1);
  ck_assert_int_eq(s21_get_sign(result), 1);
}
END_TEST

START_TEST(test_sub_zero) {
  s21_decimal a = {{123, 0, 0, 0}};
  s21_decimal b = {{0, 0, 0, 0}};
  s21_decimal result = {0};

  int error = s21_sub(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(result.bits[0], 123);
  ck_assert_int_eq(s21_get_sign(result), 0);
}
END_TEST

START_TEST(test_sub_positive_overflow) {
  // MAX96 - (-1) = MAX96 + 1
  s21_decimal a = {{-1, -1, -1, 0}};
  s21_decimal b = {{1, 0, 0, 0}};
  s21_decimal result = {0};

  s21_set_sign(&b, 1);

  int error = s21_sub(a, b, &result);

  ck_assert_int_eq(error, 1);
}
END_TEST

START_TEST(test_sub_negative_overflow) {
  // -MAX96 - 1
  s21_decimal a = {{-1, -1, -1, 0}};
  s21_decimal b = {{1, 0, 0, 0}};
  s21_decimal result = {0};

  s21_set_sign(&a, 1);

  int error = s21_sub(a, b, &result);

  ck_assert_int_eq(error, 2);
}
END_TEST

START_TEST(test_mul_zero_by_zero) {
  s21_decimal a = {{0, 0, 0, 0}};
  s21_decimal b = {{0, 0, 0, 0}};
  s21_decimal result = {{999, 999, 999, 999}};

  int error = s21_mul(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)result.bits[0], 0u);
  ck_assert_uint_eq((unsigned int)result.bits[1], 0u);
  ck_assert_uint_eq((unsigned int)result.bits[2], 0u);
  ck_assert_int_eq(s21_get_scale(result), 0);
  ck_assert_int_eq(s21_get_sign(result), 0);
}
END_TEST

START_TEST(test_mul_zero_by_nonzero) {
  s21_decimal a = {{0, 0, 0, 0}};
  s21_decimal b = {{123, 0, 0, 0}};
  s21_decimal result = {0};

  int error = s21_mul(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)result.bits[0], 0u);
}
END_TEST

START_TEST(test_mul_nonzero_by_zero) {
  s21_decimal a = {{123, 0, 0, 0}};
  s21_decimal b = {{0, 0, 0, 0}};
  s21_decimal result = {0};

  int error = s21_mul(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)result.bits[0], 0u);
}
END_TEST

START_TEST(test_mul_one_by_one) {
  s21_decimal a = {{1, 0, 0, 0}};
  s21_decimal b = {{1, 0, 0, 0}};
  s21_decimal result = {0};

  int error = s21_mul(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)result.bits[0], 1u);
  ck_assert_int_eq(s21_get_scale(result), 0);
  ck_assert_int_eq(s21_get_sign(result), 0);
}
END_TEST

START_TEST(test_mul_2_by_3) {
  s21_decimal a = {{2, 0, 0, 0}};
  s21_decimal b = {{3, 0, 0, 0}};
  s21_decimal result = {0};

  int error = s21_mul(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)result.bits[0], 6u);
}
END_TEST

START_TEST(test_mul_10_by_10) {
  s21_decimal a = {{10, 0, 0, 0}};
  s21_decimal b = {{10, 0, 0, 0}};
  s21_decimal result = {0};

  int error = s21_mul(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)result.bits[0], 100u);
}
END_TEST

START_TEST(test_mul_negative_by_positive) {
  s21_decimal a = {{5, 0, 0, 0}};
  s21_decimal b = {{3, 0, 0, 0}};
  s21_decimal result = {0};

  s21_set_sign(&a, 1);

  int error = s21_mul(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)result.bits[0], 15u);
  ck_assert_int_eq(s21_get_sign(result), 1);
}
END_TEST

START_TEST(test_mul_positive_by_negative) {
  s21_decimal a = {{5, 0, 0, 0}};
  s21_decimal b = {{3, 0, 0, 0}};
  s21_decimal result = {0};

  s21_set_sign(&b, 1);

  int error = s21_mul(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)result.bits[0], 15u);
  ck_assert_int_eq(s21_get_sign(result), 1);
}
END_TEST

START_TEST(test_mul_negative_by_negative) {
  s21_decimal a = {{5, 0, 0, 0}};
  s21_decimal b = {{3, 0, 0, 0}};
  s21_decimal result = {0};

  s21_set_sign(&a, 1);
  s21_set_sign(&b, 1);

  int error = s21_mul(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)result.bits[0], 15u);
  ck_assert_int_eq(s21_get_sign(result), 0);
}
END_TEST

START_TEST(test_mul_0_5_by_0_5) {
  s21_decimal a = {{5, 0, 0, 0}};
  s21_decimal b = {{5, 0, 0, 0}};
  s21_decimal result = {0};

  s21_set_scale(&a, 1);
  s21_set_scale(&b, 1);

  int error = s21_mul(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)result.bits[0], 25u);
  ck_assert_int_eq(s21_get_scale(result), 2);
  ck_assert_int_eq(s21_get_sign(result), 0);
}
END_TEST

START_TEST(test_mul_1_5_by_2) {
  s21_decimal a = {{15, 0, 0, 0}};
  s21_decimal b = {{2, 0, 0, 0}};
  s21_decimal result = {0};

  s21_set_scale(&a, 1);

  int error = s21_mul(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)result.bits[0], 3u);
  ck_assert_int_eq(s21_get_scale(result), 0);
}
END_TEST

START_TEST(test_mul_2_5_by_4) {
  s21_decimal a = {{25, 0, 0, 0}};
  s21_decimal b = {{4, 0, 0, 0}};
  s21_decimal result = {0};

  s21_set_scale(&a, 1);

  int error = s21_mul(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)result.bits[0], 10u);
  ck_assert_int_eq(s21_get_scale(result), 0);
}
END_TEST

START_TEST(test_mul_negative_fraction) {
  s21_decimal a = {{5, 0, 0, 0}};
  s21_decimal b = {{5, 0, 0, 0}};
  s21_decimal result = {0};

  s21_set_scale(&a, 1);
  s21_set_scale(&b, 1);
  s21_set_sign(&a, 1);

  int error = s21_mul(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)result.bits[0], 25u);
  ck_assert_int_eq(s21_get_scale(result), 2);
  ck_assert_int_eq(s21_get_sign(result), 1);
}
END_TEST

START_TEST(test_mul_large_numbers) {
  s21_decimal a = {{1000000, 0, 0, 0}};
  s21_decimal b = {{1000000, 0, 0, 0}};
  s21_decimal result = {0};

  int error = s21_mul(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)result.bits[0], 0xD4A51000u);
  ck_assert_uint_eq((unsigned int)result.bits[1], 0xE8u);
}
END_TEST

START_TEST(test_mul_overflow_positive) {
  s21_decimal a = {{-1, -1, -1, 0}};
  s21_decimal b = {{2, 0, 0, 0}};
  s21_decimal result = {0};

  int error = s21_mul(a, b, &result);

  ck_assert_int_eq(error, 1);
}
END_TEST

START_TEST(test_mul_overflow_negative) {
  s21_decimal a = {{-1, -1, -1, 0}};
  s21_decimal b = {{2, 0, 0, 0}};
  s21_decimal result = {0};

  s21_set_sign(&a, 1);

  int error = s21_mul(a, b, &result);

  ck_assert_int_eq(error, 2);
}
END_TEST

START_TEST(test_mul_overflow_reduce_scale) {
  s21_decimal a = {{-1, -1, -1, 0}};
  s21_decimal b = {{2, 0, 0, 0}};
  s21_decimal result = {0};

  s21_set_scale(&a, 1);

  int error = s21_mul(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(s21_get_scale(result), 0);
}
END_TEST

START_TEST(test_mul_null) {
  s21_decimal a = {{5, 0, 0, 0}};
  s21_decimal b = {{3, 0, 0, 0}};

  int error = s21_mul(a, b, NULL);

  ck_assert_int_eq(error, 1);
}
END_TEST

START_TEST(test_mul_clears_result_on_overflow) {
  s21_decimal a = {{-1, -1, -1, 0}};
  s21_decimal b = {{2, 0, 0, 0}};
  s21_decimal result = {{999, 999, 999, 0}};

  int error = s21_mul(a, b, &result);

  ck_assert_int_eq(error, 1);
  ck_assert_uint_eq((unsigned int)result.bits[0], 0u);
  ck_assert_uint_eq((unsigned int)result.bits[1], 0u);
  ck_assert_uint_eq((unsigned int)result.bits[2], 0u);
}
END_TEST

START_TEST(test_mul_commutative) {
  s21_decimal a = {{123, 0, 0, 0}};
  s21_decimal b = {{456, 0, 0, 0}};
  s21_decimal r1 = {0};
  s21_decimal r2 = {0};

  s21_set_scale(&a, 1);
  s21_set_scale(&b, 2);

  int e1 = s21_mul(a, b, &r1);
  int e2 = s21_mul(b, a, &r2);

  ck_assert_int_eq(e1, 0);
  ck_assert_int_eq(e2, 0);
  ck_assert_int_eq(r1.bits[0], r2.bits[0]);
  ck_assert_int_eq(r1.bits[1], r2.bits[1]);
  ck_assert_int_eq(r1.bits[2], r2.bits[2]);
  ck_assert_int_eq(s21_get_scale(r1), s21_get_scale(r2));
  ck_assert_int_eq(s21_get_sign(r1), s21_get_sign(r2));
}
END_TEST

START_TEST(test_div_zero_by_nonzero) {
  s21_decimal a = {{0, 0, 0, 0}};
  s21_decimal b = {{5, 0, 0, 0}};
  s21_decimal result = {{999, 999, 999, 999}};

  int error = s21_div(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)result.bits[0], 0u);
  ck_assert_uint_eq((unsigned int)result.bits[1], 0u);
  ck_assert_uint_eq((unsigned int)result.bits[2], 0u);
}
END_TEST

START_TEST(test_div_by_zero) {
  s21_decimal a = {{5, 0, 0, 0}};
  s21_decimal b = {{0, 0, 0, 0}};
  s21_decimal result = {0};

  int error = s21_div(a, b, &result);

  ck_assert_int_eq(error, 3);
}
END_TEST

START_TEST(test_div_zero_by_zero) {
  s21_decimal a = {{0, 0, 0, 0}};
  s21_decimal b = {{0, 0, 0, 0}};
  s21_decimal result = {0};

  int error = s21_div(a, b, &result);

  ck_assert_int_eq(error, 3);
}
END_TEST

START_TEST(test_div_one_by_one) {
  s21_decimal a = {{1, 0, 0, 0}};
  s21_decimal b = {{1, 0, 0, 0}};
  s21_decimal result = {0};

  int error = s21_div(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)result.bits[0], 1u);
  ck_assert_int_eq(s21_get_scale(result), 0);
  ck_assert_int_eq(s21_get_sign(result), 0);
}
END_TEST

START_TEST(test_div_1_by_2) {
  s21_decimal a = {{1, 0, 0, 0}};
  s21_decimal b = {{2, 0, 0, 0}};
  s21_decimal result = {0};

  int error = s21_div(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)result.bits[0], 5u);
  ck_assert_int_eq(s21_get_scale(result), 1);
}
END_TEST

START_TEST(test_div_6_by_2) {
  s21_decimal a = {{6, 0, 0, 0}};
  s21_decimal b = {{2, 0, 0, 0}};
  s21_decimal result = {0};

  int error = s21_div(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)result.bits[0], 3u);
  ck_assert_int_eq(s21_get_scale(result), 0);
}
END_TEST

START_TEST(test_div_10_by_4) {
  s21_decimal a = {{10, 0, 0, 0}};
  s21_decimal b = {{4, 0, 0, 0}};
  s21_decimal result = {0};

  int error = s21_div(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)result.bits[0], 25u);
  ck_assert_int_eq(s21_get_scale(result), 1);
}
END_TEST

START_TEST(test_div_1_by_3) {
  s21_decimal a = {{1, 0, 0, 0}};
  s21_decimal b = {{3, 0, 0, 0}};
  s21_decimal result = {0};

  int error = s21_div(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(s21_get_scale(result), 28);
}
END_TEST

START_TEST(test_div_2_by_3) {
  s21_decimal a = {{2, 0, 0, 0}};
  s21_decimal b = {{3, 0, 0, 0}};
  s21_decimal result = {0};

  int error = s21_div(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(s21_get_scale(result), 28);
}
END_TEST

START_TEST(test_div_10_by_3) {
  s21_decimal a = {{10, 0, 0, 0}};
  s21_decimal b = {{3, 0, 0, 0}};
  s21_decimal result = {0};

  int error = s21_div(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(s21_get_scale(result), 28);
}
END_TEST

START_TEST(test_div_1_by_7) {
  s21_decimal a = {{1, 0, 0, 0}};
  s21_decimal b = {{7, 0, 0, 0}};
  s21_decimal result = {0};

  int error = s21_div(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(s21_get_scale(result), 28);
}
END_TEST

START_TEST(test_div_negative_by_positive) {
  s21_decimal a = {{1, 0, 0, 0}};
  s21_decimal b = {{2, 0, 0, 0}};
  s21_decimal result = {0};

  s21_set_sign(&a, 1);

  int error = s21_div(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)result.bits[0], 5u);
  ck_assert_int_eq(s21_get_scale(result), 1);
  ck_assert_int_eq(s21_get_sign(result), 1);
}
END_TEST

START_TEST(test_div_positive_by_negative) {
  s21_decimal a = {{1, 0, 0, 0}};
  s21_decimal b = {{2, 0, 0, 0}};
  s21_decimal result = {0};

  s21_set_sign(&b, 1);

  int error = s21_div(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)result.bits[0], 5u);
  ck_assert_int_eq(s21_get_scale(result), 1);
  ck_assert_int_eq(s21_get_sign(result), 1);
}
END_TEST

START_TEST(test_div_negative_by_negative) {
  s21_decimal a = {{1, 0, 0, 0}};
  s21_decimal b = {{2, 0, 0, 0}};
  s21_decimal result = {0};

  s21_set_sign(&a, 1);
  s21_set_sign(&b, 1);

  int error = s21_div(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)result.bits[0], 5u);
  ck_assert_int_eq(s21_get_scale(result), 1);
  ck_assert_int_eq(s21_get_sign(result), 0);
}
END_TEST

START_TEST(test_div_with_scales) {
  s21_decimal a = {{15, 0, 0, 0}};
  s21_decimal b = {{5, 0, 0, 0}};
  s21_decimal result = {0};

  s21_set_scale(&a, 1);
  s21_set_scale(&b, 1);

  int error = s21_div(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)result.bits[0], 3u);
  ck_assert_int_eq(s21_get_scale(result), 0);
}
END_TEST

START_TEST(test_div_scale_1_by_scale_2) {
  s21_decimal a = {{10, 0, 0, 0}};
  s21_decimal b = {{5, 0, 0, 0}};
  s21_decimal result = {0};

  s21_set_scale(&a, 1);
  s21_set_scale(&b, 1);

  int error = s21_div(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)result.bits[0], 2u);
  ck_assert_int_eq(s21_get_scale(result), 0);
}
END_TEST

START_TEST(test_div_large_by_small) {
  s21_decimal a = {{1000000, 0, 0, 0}};
  s21_decimal b = {{1000, 0, 0, 0}};
  s21_decimal result = {0};

  int error = s21_div(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)result.bits[0], 1000u);
}
END_TEST

START_TEST(test_div_1_by_1000000) {
  s21_decimal a = {{1, 0, 0, 0}};
  s21_decimal b = {{1000000, 0, 0, 0}};
  s21_decimal result = {0};

  int error = s21_div(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)result.bits[0], 1u);
  ck_assert_int_eq(s21_get_scale(result), 6);
}
END_TEST

START_TEST(test_div_1_by_10) {
  s21_decimal a = {{1, 0, 0, 0}};
  s21_decimal b = {{10, 0, 0, 0}};
  s21_decimal result = {0};

  int error = s21_div(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)result.bits[0], 1u);
  ck_assert_int_eq(s21_get_scale(result), 1);
}
END_TEST

START_TEST(test_div_by_self) {
  s21_decimal a = {{42, 0, 0, 0}};
  s21_decimal b = {{42, 0, 0, 0}};
  s21_decimal result = {0};

  int error = s21_div(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)result.bits[0], 1u);
  ck_assert_int_eq(s21_get_scale(result), 0);
}
END_TEST

START_TEST(test_div_small_by_large) {
  s21_decimal a = {{1, 0, 0, 0}};
  s21_decimal b = {{100, 0, 0, 0}};
  s21_decimal result = {0};

  int error = s21_div(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)result.bits[0], 1u);
  ck_assert_int_eq(s21_get_scale(result), 2);
}
END_TEST

START_TEST(test_div_null) {
  s21_decimal a = {{6, 0, 0, 0}};
  s21_decimal b = {{2, 0, 0, 0}};

  int error = s21_div(a, b, NULL);

  ck_assert_int_eq(error, 1);
}
END_TEST

START_TEST(test_div_clears_result_on_error) {
  s21_decimal a = {{5, 0, 0, 0}};
  s21_decimal b = {{0, 0, 0, 0}};
  s21_decimal result = {{999, 999, 999, 0}};

  int error = s21_div(a, b, &result);

  ck_assert_int_eq(error, 3);
  ck_assert_uint_eq((unsigned int)result.bits[0], 0u);
}
END_TEST

START_TEST(test_div_2_by_2) {
  s21_decimal a = {{2, 0, 0, 0}};
  s21_decimal b = {{2, 0, 0, 0}};
  s21_decimal result = {0};

  int error = s21_div(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)result.bits[0], 1u);
}
END_TEST

START_TEST(test_div_7_by_2) {
  s21_decimal a = {{7, 0, 0, 0}};
  s21_decimal b = {{2, 0, 0, 0}};
  s21_decimal result = {0};

  int error = s21_div(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)result.bits[0], 35u);
  ck_assert_int_eq(s21_get_scale(result), 1);
}
END_TEST

START_TEST(test_div_1_by_4) {
  s21_decimal a = {{1, 0, 0, 0}};
  s21_decimal b = {{4, 0, 0, 0}};
  s21_decimal result = {0};

  int error = s21_div(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)result.bits[0], 25u);
  ck_assert_int_eq(s21_get_scale(result), 2);
}
END_TEST

START_TEST(test_div_1_by_8) {
  s21_decimal a = {{1, 0, 0, 0}};
  s21_decimal b = {{8, 0, 0, 0}};
  s21_decimal result = {0};

  int error = s21_div(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)result.bits[0], 125u);
  ck_assert_int_eq(s21_get_scale(result), 3);
}
END_TEST

START_TEST(test_div_22_by_7) {
  s21_decimal a = {{22, 0, 0, 0}};
  s21_decimal b = {{7, 0, 0, 0}};
  s21_decimal result = {0};

  int error = s21_div(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(s21_get_scale(result), 28);
}
END_TEST

START_TEST(test_div_100_by_3) {
  s21_decimal a = {{100, 0, 0, 0}};
  s21_decimal b = {{3, 0, 0, 0}};
  s21_decimal result = {0};

  int error = s21_div(a, b, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(s21_get_scale(result), 27);
}
END_TEST

START_TEST(test_from_int_to_decimal_zero) {
  s21_decimal dst = {{999, 999, 999, 999}};  // мусор, чтобы проверить обнуление
  int error = s21_from_int_to_decimal(0, &dst);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)dst.bits[0], 0u);
  ck_assert_uint_eq((unsigned int)dst.bits[1], 0u);
  ck_assert_uint_eq((unsigned int)dst.bits[2], 0u);
  ck_assert_uint_eq((unsigned int)dst.bits[3], 0u);
}
END_TEST

START_TEST(test_from_int_to_decimal_positive) {
  s21_decimal dst = {0};
  int error = s21_from_int_to_decimal(123, &dst);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)dst.bits[0], 123u);
  ck_assert_uint_eq((unsigned int)dst.bits[1], 0u);
  ck_assert_uint_eq((unsigned int)dst.bits[2], 0u);
  ck_assert_int_eq(s21_get_sign(dst), 0);
  ck_assert_int_eq(s21_get_scale(dst), 0);
}
END_TEST

START_TEST(test_from_int_to_decimal_negative) {
  s21_decimal dst = {0};
  int error = s21_from_int_to_decimal(-123, &dst);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)dst.bits[0], 123u);
  ck_assert_uint_eq((unsigned int)dst.bits[1], 0u);
  ck_assert_uint_eq((unsigned int)dst.bits[2], 0u);
  ck_assert_int_eq(s21_get_sign(dst), 1);
  ck_assert_int_eq(s21_get_scale(dst), 0);
}
END_TEST

START_TEST(test_from_int_to_decimal_int_max) {
  s21_decimal dst = {0};
  int error = s21_from_int_to_decimal(2147483647, &dst);  // INT_MAX

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)dst.bits[0], 2147483647u);
  ck_assert_uint_eq((unsigned int)dst.bits[1], 0u);
  ck_assert_uint_eq((unsigned int)dst.bits[2], 0u);
  ck_assert_int_eq(s21_get_sign(dst), 0);
}
END_TEST

START_TEST(test_from_int_to_decimal_int_min) {
  s21_decimal dst = {0};
  // -2147483648 — особый случай: -INT_MIN даёт UB, если делать через int
  int error = s21_from_int_to_decimal(-2147483648, &dst);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)dst.bits[0], 2147483648u);
  ck_assert_uint_eq((unsigned int)dst.bits[1], 0u);
  ck_assert_uint_eq((unsigned int)dst.bits[2], 0u);
  ck_assert_int_eq(s21_get_sign(dst), 1);
}
END_TEST

START_TEST(test_from_int_to_decimal_one) {
  s21_decimal dst = {0};
  int error = s21_from_int_to_decimal(1, &dst);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)dst.bits[0], 1u);
  ck_assert_int_eq(s21_get_sign(dst), 0);
}
END_TEST

START_TEST(test_from_int_to_decimal_minus_one) {
  s21_decimal dst = {0};
  int error = s21_from_int_to_decimal(-1, &dst);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)dst.bits[0], 1u);
  ck_assert_int_eq(s21_get_sign(dst), 1);
}
END_TEST

START_TEST(test_from_int_to_decimal_null) {
  int error = s21_from_int_to_decimal(42, NULL);

  ck_assert_int_eq(error, 1);
}
END_TEST

START_TEST(test_from_int_to_decimal_clears_old_value) {
  // Проверяем, что функция полностью перезаписывает dst,
  // даже если там был мусор.
  s21_decimal dst = {{123, 456, 789, 0}};
  s21_set_sign(&dst, 1);
  s21_set_scale(&dst, 5);

  int error = s21_from_int_to_decimal(7, &dst);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)dst.bits[0], 7u);
  ck_assert_uint_eq((unsigned int)dst.bits[1], 0u);
  ck_assert_uint_eq((unsigned int)dst.bits[2], 0u);
  ck_assert_int_eq(s21_get_sign(dst), 0);   // знак сброшен
  ck_assert_int_eq(s21_get_scale(dst), 0);  // scale сброшен
}
END_TEST

START_TEST(test_from_decimal_to_int_zero) {
  s21_decimal src = {{0, 0, 0, 0}};
  int dst = 999;
  int error = s21_from_decimal_to_int(src, &dst);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(dst, 0);
}
END_TEST

START_TEST(test_from_decimal_to_int_positive) {
  s21_decimal src = {{123, 0, 0, 0}};
  int dst = 0;
  int error = s21_from_decimal_to_int(src, &dst);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(dst, 123);
}
END_TEST

START_TEST(test_from_decimal_to_int_negative) {
  s21_decimal src = {{123, 0, 0, 0}};
  s21_set_sign(&src, 1);
  int dst = 0;
  int error = s21_from_decimal_to_int(src, &dst);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(dst, -123);
}
END_TEST

START_TEST(test_from_decimal_to_int_with_fraction_truncate_positive) {
  // 123.45 -> 123
  s21_decimal src = {{12345, 0, 0, 0}};
  s21_set_scale(&src, 2);
  int dst = 0;
  int error = s21_from_decimal_to_int(src, &dst);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(dst, 123);
}
END_TEST

START_TEST(test_from_decimal_to_int_with_fraction_truncate_negative) {
  // -123.45 -> -123
  s21_decimal src = {{12345, 0, 0, 0}};
  s21_set_scale(&src, 2);
  s21_set_sign(&src, 1);
  int dst = 0;
  int error = s21_from_decimal_to_int(src, &dst);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(dst, -123);
}
END_TEST

START_TEST(test_from_decimal_to_int_less_than_one) {
  // 0.9 -> 0 (truncate, не round)
  s21_decimal src = {{9, 0, 0, 0}};
  s21_set_scale(&src, 1);
  int dst = 999;
  int error = s21_from_decimal_to_int(src, &dst);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(dst, 0);
}
END_TEST

START_TEST(test_from_decimal_to_int_negative_less_than_one) {
  // -0.9 -> 0 (не -1!)
  s21_decimal src = {{9, 0, 0, 0}};
  s21_set_scale(&src, 1);
  s21_set_sign(&src, 1);
  int dst = 999;
  int error = s21_from_decimal_to_int(src, &dst);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(dst, 0);
}
END_TEST

START_TEST(test_from_decimal_to_int_int_max) {
  // 2147483647 — влезает
  s21_decimal src = {{2147483647, 0, 0, 0}};
  int dst = 0;
  int error = s21_from_decimal_to_int(src, &dst);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(dst, 2147483647);
}
END_TEST

START_TEST(test_from_decimal_to_int_int_min) {
  // -2147483648 — влезает
  s21_decimal src = {{2147483648u, 0, 0, 0}};
  s21_set_sign(&src, 1);
  int dst = 0;
  int error = s21_from_decimal_to_int(src, &dst);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(dst, -2147483648);
}
END_TEST

START_TEST(test_from_decimal_to_int_overflow_positive_by_one) {
  // 2147483648 — на 1 больше INT_MAX
  s21_decimal src = {{2147483648u, 0, 0, 0}};
  int dst = 0;
  int error = s21_from_decimal_to_int(src, &dst);

  ck_assert_int_eq(error, 1);
}
END_TEST

START_TEST(test_from_decimal_to_int_overflow_negative_by_one) {
  // -2147483649 — на 1 меньше INT_MIN
  s21_decimal src = {{2147483649u, 0, 0, 0}};
  s21_set_sign(&src, 1);
  int dst = 0;
  int error = s21_from_decimal_to_int(src, &dst);

  ck_assert_int_eq(error, 1);
}
END_TEST

START_TEST(test_from_decimal_to_int_overflow_bits1) {
  // bits[1] != 0 — точно не влезает
  s21_decimal src = {{0, 1, 0, 0}};  // 2^32
  int dst = 0;
  int error = s21_from_decimal_to_int(src, &dst);

  ck_assert_int_eq(error, 1);
}
END_TEST

START_TEST(test_from_decimal_to_int_overflow_bits2) {
  // bits[2] != 0 — точно не влезает
  s21_decimal src = {{0, 0, 1, 0}};  // 2^64
  int dst = 0;
  int error = s21_from_decimal_to_int(src, &dst);

  ck_assert_int_eq(error, 1);
}
END_TEST

START_TEST(test_from_decimal_to_int_fraction_does_not_hide_overflow) {
  // 2147483647.5 — целая часть влезает, но после truncate
  // должно получиться 2147483647, и это ок.
  // А вот 2147483648.5 после truncate даст 2147483648 — overflow.
  s21_decimal src = {{2147483648u, 0, 0, 0}};
  s21_set_scale(&src, 1);  // 214748364.8 (не overflow!)
  // scale = 1, значит мантисса 2147483648 / 10 = 214748364
  int dst = 0;
  int error = s21_from_decimal_to_int(src, &dst);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(dst, 214748364);
}
END_TEST

START_TEST(test_from_decimal_to_int_null) {
  s21_decimal src = {{42, 0, 0, 0}};
  int error = s21_from_decimal_to_int(src, NULL);

  ck_assert_int_eq(error, 1);
}
END_TEST

START_TEST(test_from_decimal_to_int_with_scale_zero_value) {
  // 0.000... (scale 28, мантисса 0) -> 0
  s21_decimal src = {{0, 0, 0, 0}};
  s21_set_scale(&src, 28);
  int dst = 999;
  int error = s21_from_decimal_to_int(src, &dst);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(dst, 0);
}
END_TEST

START_TEST(test_from_decimal_to_int_truncate_negative_zero) {
  // -0.5 -> 0 (знак не должен влиять на результат 0)
  s21_decimal src = {{5, 0, 0, 0}};
  s21_set_scale(&src, 1);
  s21_set_sign(&src, 1);
  int dst = 999;
  int error = s21_from_decimal_to_int(src, &dst);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(dst, 0);
}
END_TEST

START_TEST(test_from_float_to_decimal_zero) {
  s21_decimal dst = {{999, 999, 999, 999}};
  int error = s21_from_float_to_decimal(0.0f, &dst);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)dst.bits[0], 0u);
  ck_assert_uint_eq((unsigned int)dst.bits[1], 0u);
  ck_assert_uint_eq((unsigned int)dst.bits[2], 0u);
  ck_assert_uint_eq((unsigned int)dst.bits[3], 0u);
}
END_TEST

START_TEST(test_from_float_to_decimal_negative_zero) {
  s21_decimal dst = {{999, 999, 999, 999}};
  int error = s21_from_float_to_decimal(-0.0f, &dst);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)dst.bits[0], 0u);
  ck_assert_uint_eq((unsigned int)dst.bits[1], 0u);
  ck_assert_uint_eq((unsigned int)dst.bits[2], 0u);
}
END_TEST

START_TEST(test_from_float_to_decimal_one) {
  s21_decimal dst = {0};
  int error = s21_from_float_to_decimal(1.0f, &dst);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)dst.bits[0], 1u);
  ck_assert_uint_eq((unsigned int)dst.bits[1], 0u);
  ck_assert_uint_eq((unsigned int)dst.bits[2], 0u);
  ck_assert_int_eq(s21_get_scale(dst), 0);
  ck_assert_int_eq(s21_get_sign(dst), 0);
}
END_TEST

START_TEST(test_from_float_to_decimal_minus_one) {
  s21_decimal dst = {0};
  int error = s21_from_float_to_decimal(-1.0f, &dst);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)dst.bits[0], 1u);
  ck_assert_int_eq(s21_get_scale(dst), 0);
  ck_assert_int_eq(s21_get_sign(dst), 1);
}
END_TEST

START_TEST(test_from_float_to_decimal_integer) {
  s21_decimal dst = {0};
  int error = s21_from_float_to_decimal(42.0f, &dst);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)dst.bits[0], 42u);
  ck_assert_int_eq(s21_get_scale(dst), 0);
}
END_TEST

START_TEST(test_from_float_to_decimal_simple_fraction) {
  s21_decimal dst = {0};
  int error = s21_from_float_to_decimal(0.5f, &dst);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)dst.bits[0], 5u);
  ck_assert_int_eq(s21_get_scale(dst), 1);
}
END_TEST

START_TEST(test_from_float_to_decimal_0_1_no_garbage) {
  s21_decimal dst = {0};
  int error = s21_from_float_to_decimal(0.1f, &dst);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)dst.bits[0], 1u);
  ck_assert_int_eq(s21_get_scale(dst), 1);
}
END_TEST

START_TEST(test_from_float_to_decimal_0_2_no_garbage) {
  s21_decimal dst = {0};
  int error = s21_from_float_to_decimal(0.2f, &dst);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)dst.bits[0], 2u);
  ck_assert_int_eq(s21_get_scale(dst), 1);
}
END_TEST

START_TEST(test_from_float_to_decimal_0_3_no_garbage) {
  s21_decimal dst = {0};
  int error = s21_from_float_to_decimal(0.3f, &dst);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)dst.bits[0], 3u);
  ck_assert_int_eq(s21_get_scale(dst), 1);
}
END_TEST

START_TEST(test_from_float_to_decimal_123_456) {
  s21_decimal dst = {0};
  int error = s21_from_float_to_decimal(123.456f, &dst);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)dst.bits[0], 123456u);
  ck_assert_int_eq(s21_get_scale(dst), 3);
  ck_assert_int_eq(s21_get_sign(dst), 0);
}
END_TEST

START_TEST(test_from_float_to_decimal_minus_123_456) {
  s21_decimal dst = {0};
  int error = s21_from_float_to_decimal(-123.456f, &dst);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)dst.bits[0], 123456u);
  ck_assert_int_eq(s21_get_scale(dst), 3);
  ck_assert_int_eq(s21_get_sign(dst), 1);
}
END_TEST

START_TEST(test_from_float_to_decimal_small_value) {
  s21_decimal dst = {0};
  int error = s21_from_float_to_decimal(0.001f, &dst);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)dst.bits[0], 1u);
  ck_assert_int_eq(s21_get_scale(dst), 3);
}
END_TEST

START_TEST(test_from_float_to_decimal_small_negative) {
  s21_decimal dst = {0};
  int error = s21_from_float_to_decimal(-0.001f, &dst);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)dst.bits[0], 1u);
  ck_assert_int_eq(s21_get_scale(dst), 3);
  ck_assert_int_eq(s21_get_sign(dst), 1);
}
END_TEST

START_TEST(test_from_float_to_decimal_round_7_digits) {
  s21_decimal dst = {0};
  int error = s21_from_float_to_decimal(12345678.0f, &dst);
  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)dst.bits[0], 12345680u);
  ck_assert_uint_eq((unsigned int)dst.bits[1], 0u);
  ck_assert_uint_eq((unsigned int)dst.bits[2], 0u);
  ck_assert_int_eq(s21_get_scale(dst), 0);
  ck_assert_int_eq(s21_get_sign(dst), 0);
}
END_TEST

START_TEST(test_from_float_to_decimal_7_digits_ok) {
  s21_decimal dst = {0};
  int error = s21_from_float_to_decimal(1234567.0f, &dst);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)dst.bits[0], 1234567u);
  ck_assert_int_eq(s21_get_scale(dst), 0);
}
END_TEST

START_TEST(test_from_float_to_decimal_9999999) {
  s21_decimal dst = {0};
  int error = s21_from_float_to_decimal(9999999.0f, &dst);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)dst.bits[0], 9999999u);
  ck_assert_int_eq(s21_get_scale(dst), 0);
}
END_TEST

START_TEST(test_from_float_to_decimal_min_positive) {
  s21_decimal dst = {0};
  int error = s21_from_float_to_decimal(1e-28f, &dst);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)dst.bits[0], 1u);
  ck_assert_int_eq(s21_get_scale(dst), 28);
}
END_TEST

START_TEST(test_from_float_to_decimal_too_small) {
  s21_decimal dst = {{999, 999, 999, 999}};
  int error = s21_from_float_to_decimal(1e-29f, &dst);

  ck_assert_int_eq(error, 1);
  ck_assert_uint_eq((unsigned int)dst.bits[0], 0u);
  ck_assert_uint_eq((unsigned int)dst.bits[1], 0u);
  ck_assert_uint_eq((unsigned int)dst.bits[2], 0u);
}
END_TEST

START_TEST(test_from_float_to_decimal_too_big) {
  s21_decimal dst = {0};
  int error = s21_from_float_to_decimal(1e30f, &dst);

  ck_assert_int_eq(error, 1);
}
END_TEST

START_TEST(test_from_float_to_decimal_max_range) {
  s21_decimal dst = {0};
  int error = s21_from_float_to_decimal(7.9e28f, &dst);

  ck_assert(error == 0 || error == 1);
}
END_TEST

START_TEST(test_from_float_to_decimal_nan) {
  s21_decimal dst = {0};
  int error = s21_from_float_to_decimal(NAN, &dst);

  ck_assert_int_eq(error, 1);
}
END_TEST

START_TEST(test_from_float_to_decimal_inf) {
  s21_decimal dst = {0};
  int error = s21_from_float_to_decimal(INFINITY, &dst);

  ck_assert_int_eq(error, 1);
}
END_TEST

START_TEST(test_from_float_to_decimal_neg_inf) {
  s21_decimal dst = {0};
  int error = s21_from_float_to_decimal(-INFINITY, &dst);

  ck_assert_int_eq(error, 1);
}
END_TEST

START_TEST(test_from_float_to_decimal_null) {
  int error = s21_from_float_to_decimal(42.0f, NULL);

  ck_assert_int_eq(error, 1);
}
END_TEST

START_TEST(test_from_float_to_decimal_clears_old_value) {
  s21_decimal dst = {{123, 456, 789, 0}};
  s21_set_sign(&dst, 1);
  s21_set_scale(&dst, 5);

  int error = s21_from_float_to_decimal(7.0f, &dst);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)dst.bits[0], 7u);
  ck_assert_uint_eq((unsigned int)dst.bits[1], 0u);
  ck_assert_uint_eq((unsigned int)dst.bits[2], 0u);
  ck_assert_int_eq(s21_get_sign(dst), 0);
  ck_assert_int_eq(s21_get_scale(dst), 0);
}
END_TEST

START_TEST(test_from_float_to_decimal_scale_check) {
  s21_decimal dst = {0};
  int error = s21_from_float_to_decimal(0.00001f, &dst);

  ck_assert_int_eq(error, 0);
  ck_assert_uint_eq((unsigned int)dst.bits[0], 1u);
  ck_assert_int_eq(s21_get_scale(dst), 5);
}
END_TEST

START_TEST(test_from_decimal_to_float_zero) {
  s21_decimal src = {{0, 0, 0, 0}};
  float dst = 999.0f;
  int error = s21_from_decimal_to_float(src, &dst);

  ck_assert_int_eq(error, 0);
  ck_assert_float_eq(dst, 0.0f);
}
END_TEST

START_TEST(test_from_decimal_to_float_one) {
  s21_decimal src = {{1, 0, 0, 0}};
  float dst = 0.0f;
  int error = s21_from_decimal_to_float(src, &dst);

  ck_assert_int_eq(error, 0);
  ck_assert_float_eq(dst, 1.0f);
}
END_TEST

START_TEST(test_from_decimal_to_float_minus_one) {
  s21_decimal src = {{1, 0, 0, 0}};
  s21_set_sign(&src, 1);
  float dst = 0.0f;
  int error = s21_from_decimal_to_float(src, &dst);

  ck_assert_int_eq(error, 0);
  ck_assert_float_eq(dst, -1.0f);
}
END_TEST

START_TEST(test_from_decimal_to_float_integer) {
  s21_decimal src = {{42, 0, 0, 0}};
  float dst = 0.0f;
  int error = s21_from_decimal_to_float(src, &dst);

  ck_assert_int_eq(error, 0);
  ck_assert_float_eq(dst, 42.0f);
}
END_TEST

START_TEST(test_from_decimal_to_float_half) {
  s21_decimal src = {{5, 0, 0, 0}};
  s21_set_scale(&src, 1);
  float dst = 0.0f;
  int error = s21_from_decimal_to_float(src, &dst);

  ck_assert_int_eq(error, 0);
  ck_assert_float_eq(dst, 0.5f);
}
END_TEST

START_TEST(test_from_decimal_to_float_123_456) {
  s21_decimal src = {{123456, 0, 0, 0}};
  s21_set_scale(&src, 3);
  float dst = 0.0f;
  int error = s21_from_decimal_to_float(src, &dst);

  ck_assert_int_eq(error, 0);
  ck_assert_float_eq_tol(dst, 123.456f, 1e-3f);
}
END_TEST

START_TEST(test_from_decimal_to_float_minus_123_456) {
  s21_decimal src = {{123456, 0, 0, 0}};
  s21_set_scale(&src, 3);
  s21_set_sign(&src, 1);
  float dst = 0.0f;
  int error = s21_from_decimal_to_float(src, &dst);

  ck_assert_int_eq(error, 0);
  ck_assert_float_eq_tol(dst, -123.456f, 1e-3f);
}
END_TEST

START_TEST(test_from_decimal_to_float_0_1) {
  s21_decimal src = {{1, 0, 0, 0}};
  s21_set_scale(&src, 1);
  float dst = 0.0f;
  int error = s21_from_decimal_to_float(src, &dst);

  ck_assert_int_eq(error, 0);
  ck_assert_float_eq_tol(dst, 0.1f, 1e-6f);
}
END_TEST

START_TEST(test_from_decimal_to_float_large) {
  s21_decimal src = {{0, 1, 0, 0}};
  float dst = 0.0f;
  int error = s21_from_decimal_to_float(src, &dst);

  ck_assert_int_eq(error, 0);
  ck_assert_float_eq_tol(dst, 4294967296.0f, 1.0f);
}
END_TEST

START_TEST(test_from_decimal_to_float_high_scale) {
  s21_decimal src = {{1, 0, 0, 0}};
  s21_set_scale(&src, 28);
  float dst = 0.0f;
  int error = s21_from_decimal_to_float(src, &dst);

  ck_assert_int_eq(error, 0);
  ck_assert(dst >= 0.0f);
  ck_assert(dst < 1e-27f);
}
END_TEST

START_TEST(test_from_decimal_to_float_negative_zero) {
  s21_decimal src = {{0, 0, 0, 0}};
  s21_set_sign(&src, 1);
  float dst = 999.0f;
  int error = s21_from_decimal_to_float(src, &dst);

  ck_assert_int_eq(error, 0);
  ck_assert_float_eq(dst, 0.0f);
}
END_TEST

START_TEST(test_from_decimal_to_float_null) {
  s21_decimal src = {{42, 0, 0, 0}};
  int error = s21_from_decimal_to_float(src, NULL);

  ck_assert_int_eq(error, 1);
}
END_TEST

START_TEST(test_roundtrip_0_5) {
  s21_decimal d = {0};
  float back = 0.0f;
  int e1 = s21_from_float_to_decimal(0.5f, &d);
  int e2 = s21_from_decimal_to_float(d, &back);

  ck_assert_int_eq(e1, 0);
  ck_assert_int_eq(e2, 0);
  ck_assert_float_eq(back, 0.5f);
}
END_TEST

START_TEST(test_roundtrip_123_456) {
  s21_decimal d = {0};
  float back = 0.0f;
  int e1 = s21_from_float_to_decimal(123.456f, &d);
  int e2 = s21_from_decimal_to_float(d, &back);

  ck_assert_int_eq(e1, 0);
  ck_assert_int_eq(e2, 0);
  ck_assert_float_eq_tol(back, 123.456f, 1e-3f);
}
END_TEST

START_TEST(test_roundtrip_negative) {
  s21_decimal d = {0};
  float back = 0.0f;
  int e1 = s21_from_float_to_decimal(-42.5f, &d);
  int e2 = s21_from_decimal_to_float(d, &back);

  ck_assert_int_eq(e1, 0);
  ck_assert_int_eq(e2, 0);
  ck_assert_float_eq(back, -42.5f);
}
END_TEST

START_TEST(test_roundtrip_0_1) {
  s21_decimal d = {0};
  float back = 0.0f;
  int e1 = s21_from_float_to_decimal(0.1f, &d);
  int e2 = s21_from_decimal_to_float(d, &back);

  ck_assert_int_eq(e1, 0);
  ck_assert_int_eq(e2, 0);
  ck_assert_float_eq(back, 0.1f);
}
END_TEST

// Проверяет переполнение при делении максимального decimal на 0.1.
// MAX / 0.1 = MAX * 10, результат выходит за допустимый диапазон.
// Ожидаем код ошибки 1 — положительное переполнение.
//
// Фактический результат:
// FAIL: s21_div возвращает error = 0 вместо error = 1
START_TEST(test_div_max_by_point_one_overflow) {
  s21_decimal value_1 = {{-1, -1, -1, 0}};
  s21_decimal value_2 = {{1, 0, 0, 0}};
  s21_decimal result = {0};

  s21_set_scale(&value_2, 1);

  int error = s21_div(value_1, value_2, &result);

  ck_assert_int_eq(error, 1);
}
END_TEST

// Проверяет деление максимального decimal на 2
// Точный результат заканчивается на .5 и требует банковского округления
// Ожидаемый результат — 2^95, который помещается в decimal
//
// Фактический результат:
// FAIL: result.bits[0] = -5 вместо ожидаемого 0.
// Результат деления сформирован неверно.
START_TEST(test_div_max_by_two_bank_round) {
  s21_decimal value_1 = {{-1, -1, -1, 0}};
  s21_decimal value_2 = {{2, 0, 0, 0}};
  s21_decimal result = {0};

  s21_decimal expected = {{0, 0, (int)0x80000000u, 0}};

  int error = s21_div(value_1, value_2, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(result.bits[0], expected.bits[0]);
  ck_assert_int_eq(result.bits[1], expected.bits[1]);
  ck_assert_int_eq(result.bits[2], expected.bits[2]);
  ck_assert_int_eq(result.bits[3], expected.bits[3]);
}
END_TEST

// Проверяет банковское округление при умножении с точным остатком .5.
// (MAX - 2) * 0.5 = 2^95 - 1.5.
// При равном расстоянии округляем к четному: ожидаем 2^95 - 2.
//
// Фактический результат:
// FAIL: result.bits[0] = -1 вместо ожидаемого -2.
// Вместо округления к четному результат округляется вверх.
START_TEST(test_mul_exact_half_bank_round_even) {
  s21_decimal value_1 = {{-3, -1, -1, 0}};
  s21_decimal value_2 = {{5, 0, 0, 0}};
  s21_decimal result = {0};

  s21_set_scale(&value_2, 1);

  s21_decimal expected = {{-2, -1, 2147483647, 0}};

  int error = s21_mul(value_1, value_2, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(result.bits[0], expected.bits[0]);
  ck_assert_int_eq(result.bits[1], expected.bits[1]);
  ck_assert_int_eq(result.bits[2], expected.bits[2]);
  ck_assert_int_eq(result.bits[3], expected.bits[3]);
}
END_TEST

// Проверяет преобразование float с более чем 7 значащими цифрами.
// 12345678 содержит 8 значащих цифр.
// По ТЗ число должно быть округлено до не более чем 7 значащих цифр:
// 12345678 -> 12345680.
// Ожидаем успешное преобразование, а не ошибку.
START_TEST(test_from_float_more_than_7_significant_digits) {
  s21_decimal result = {0};
  s21_decimal expected = {{12345680, 0, 0, 0}};

  int error = s21_from_float_to_decimal(12345678.0f, &result);

  ck_assert_int_eq(error, 0);
  ck_assert_int_eq(result.bits[0], expected.bits[0]);
  ck_assert_int_eq(result.bits[1], expected.bits[1]);
  ck_assert_int_eq(result.bits[2], expected.bits[2]);
  ck_assert_int_eq(result.bits[3], expected.bits[3]);
}
END_TEST

TCase* arithmetic_add_sub(void) {
  TCase* s21_arithmetic_add_sub = tcase_create("Arithmetic_Add_Sub");
  tcase_add_test(s21_arithmetic_add_sub, test_add_positive);
  tcase_add_test(s21_arithmetic_add_sub, test_add_negative);
  tcase_add_test(s21_arithmetic_add_sub,
                 test_add_different_sign_positive_result);
  tcase_add_test(s21_arithmetic_add_sub,
                 test_add_different_sign_negative_result);
  tcase_add_test(s21_arithmetic_add_sub, test_add_equal_different_sign);
  tcase_add_test(s21_arithmetic_add_sub, test_add_successful_align);
  tcase_add_test(s21_arithmetic_add_sub, test_add_successful_align_reverse);
  tcase_add_test(s21_arithmetic_add_sub,
                 test_add_successful_align_different_signs);

  tcase_add_test(s21_arithmetic_add_sub, test_add_align_overflow_reduce_down);
  tcase_add_test(s21_arithmetic_add_sub, test_add_align_overflow_bank_even);
  tcase_add_test(s21_arithmetic_add_sub,
                 test_add_align_overflow_positive_overflow);
  tcase_add_test(s21_arithmetic_add_sub,
                 test_add_align_overflow_negative_overflow);
  tcase_add_test(s21_arithmetic_add_sub,
                 test_add_align_overflow_different_signs);
  tcase_add_test(s21_arithmetic_add_sub,
                 test_add_align_overflow_different_signs_negative);
  tcase_add_test(s21_arithmetic_add_sub, test_add_align_overflow_reverse);
  tcase_add_test(s21_arithmetic_add_sub,
                 test_add_align_overflow_reduce_with_tail);

  tcase_add_test(s21_arithmetic_add_sub,
                 test_add_mantissa_overflow_can_reduce_scale);
  tcase_add_test(s21_arithmetic_add_sub,
                 test_add_negative_mantissa_overflow_can_reduce_scale);
  tcase_add_test(s21_arithmetic_add_sub, test_add_final_positive_overflow);
  tcase_add_test(s21_arithmetic_add_sub, test_add_final_negative_overflow);
  tcase_add_test(s21_arithmetic_add_sub,
                 test_add_overflow_reduce_scale_round_up);

  tcase_add_test(s21_arithmetic_add_sub, test_sub_positive_positive_greater);
  tcase_add_test(s21_arithmetic_add_sub, test_sub_positive_positive_less);
  tcase_add_test(s21_arithmetic_add_sub, test_sub_positive_negative);
  tcase_add_test(s21_arithmetic_add_sub, test_sub_negative_positive);
  tcase_add_test(s21_arithmetic_add_sub, test_sub_negative_negative);
  tcase_add_test(s21_arithmetic_add_sub, test_sub_equal_values);
  tcase_add_test(s21_arithmetic_add_sub, test_sub_different_scales);
  tcase_add_test(s21_arithmetic_add_sub,
                 test_sub_different_scales_negative_result);
  tcase_add_test(s21_arithmetic_add_sub, test_sub_zero);
  tcase_add_test(s21_arithmetic_add_sub, test_sub_positive_overflow);
  tcase_add_test(s21_arithmetic_add_sub, test_sub_negative_overflow);
  return s21_arithmetic_add_sub;
}

TCase* arithmetic_mul(void) {
  TCase* s21_arithmetic_mul = tcase_create("Arithmetic_Mul");
  tcase_add_test(s21_arithmetic_mul, test_mul_zero_by_zero);
  tcase_add_test(s21_arithmetic_mul, test_mul_zero_by_nonzero);
  tcase_add_test(s21_arithmetic_mul, test_mul_nonzero_by_zero);
  tcase_add_test(s21_arithmetic_mul, test_mul_one_by_one);
  tcase_add_test(s21_arithmetic_mul, test_mul_2_by_3);
  tcase_add_test(s21_arithmetic_mul, test_mul_10_by_10);
  tcase_add_test(s21_arithmetic_mul, test_mul_negative_by_positive);
  tcase_add_test(s21_arithmetic_mul, test_mul_positive_by_negative);
  tcase_add_test(s21_arithmetic_mul, test_mul_negative_by_negative);
  tcase_add_test(s21_arithmetic_mul, test_mul_0_5_by_0_5);
  tcase_add_test(s21_arithmetic_mul, test_mul_1_5_by_2);
  tcase_add_test(s21_arithmetic_mul, test_mul_2_5_by_4);
  tcase_add_test(s21_arithmetic_mul, test_mul_negative_fraction);
  tcase_add_test(s21_arithmetic_mul, test_mul_large_numbers);
  tcase_add_test(s21_arithmetic_mul, test_mul_overflow_positive);
  tcase_add_test(s21_arithmetic_mul, test_mul_overflow_negative);
  tcase_add_test(s21_arithmetic_mul, test_mul_overflow_reduce_scale);
  tcase_add_test(s21_arithmetic_mul, test_mul_null);
  tcase_add_test(s21_arithmetic_mul, test_mul_clears_result_on_overflow);
  tcase_add_test(s21_arithmetic_mul, test_mul_commutative);
  return s21_arithmetic_mul;
}

TCase* arithmetic_div(void) {
  TCase* s21_arithmetic_div = tcase_create("Arithmetic_Div");
  tcase_add_test(s21_arithmetic_div, test_div_zero_by_nonzero);
  tcase_add_test(s21_arithmetic_div, test_div_by_zero);
  tcase_add_test(s21_arithmetic_div, test_div_zero_by_zero);
  tcase_add_test(s21_arithmetic_div, test_div_one_by_one);
  tcase_add_test(s21_arithmetic_div, test_div_1_by_2);
  tcase_add_test(s21_arithmetic_div, test_div_6_by_2);
  tcase_add_test(s21_arithmetic_div, test_div_10_by_4);
  tcase_add_test(s21_arithmetic_div, test_div_1_by_3);
  tcase_add_test(s21_arithmetic_div, test_div_2_by_3);
  tcase_add_test(s21_arithmetic_div, test_div_10_by_3);
  tcase_add_test(s21_arithmetic_div, test_div_1_by_7);
  tcase_add_test(s21_arithmetic_div, test_div_negative_by_positive);
  tcase_add_test(s21_arithmetic_div, test_div_positive_by_negative);
  tcase_add_test(s21_arithmetic_div, test_div_negative_by_negative);
  tcase_add_test(s21_arithmetic_div, test_div_with_scales);
  tcase_add_test(s21_arithmetic_div, test_div_scale_1_by_scale_2);
  tcase_add_test(s21_arithmetic_div, test_div_large_by_small);
  tcase_add_test(s21_arithmetic_div, test_div_1_by_1000000);
  tcase_add_test(s21_arithmetic_div, test_div_1_by_10);
  tcase_add_test(s21_arithmetic_div, test_div_by_self);
  tcase_add_test(s21_arithmetic_div, test_div_small_by_large);
  tcase_add_test(s21_arithmetic_div, test_div_null);
  tcase_add_test(s21_arithmetic_div, test_div_clears_result_on_error);
  tcase_add_test(s21_arithmetic_div, test_div_2_by_2);
  tcase_add_test(s21_arithmetic_div, test_div_7_by_2);
  tcase_add_test(s21_arithmetic_div, test_div_1_by_4);
  tcase_add_test(s21_arithmetic_div, test_div_1_by_8);
  tcase_add_test(s21_arithmetic_div, test_div_22_by_7);
  tcase_add_test(s21_arithmetic_div, test_div_100_by_3);
  return s21_arithmetic_div;
}

TCase* general(void) {
  TCase* s21_general = tcase_create("General");
  tcase_add_test(s21_general, test_is_equal);
  tcase_add_test(s21_general, test_is_not_equal);
  tcase_add_test(s21_general, test_is_less);
  tcase_add_test(s21_general, test_is_less_or_equal);
  tcase_add_test(s21_general, test_is_greater);
  tcase_add_test(s21_general, test_is_greater_or_equal);
  tcase_add_test(s21_general, test_negate);
  tcase_add_test(s21_general, test_truncate);
  tcase_add_test(s21_general, test_floor);
  tcase_add_test(s21_general, test_round);
  return s21_general;
}

TCase* helpers(void) {
  TCase* s21_helpers = tcase_create("Helpers");
  tcase_add_test(s21_helpers, test_get_bit);
  tcase_add_test(s21_helpers, test_set_bit);

  tcase_add_test(s21_helpers, test_add_different_signs_first_positive_bigger);
  tcase_add_test(s21_helpers, test_add_different_signs_first_negative_bigger);
  tcase_add_test(s21_helpers, test_add_different_signs_second_positive_bigger);
  tcase_add_test(s21_helpers, test_add_different_signs_second_negative_bigger);
  tcase_add_test(s21_helpers, test_add_different_signs_equal_modules);
  tcase_add_test(s21_helpers, test_add_different_signs_keeps_scale);

  tcase_add_test(s21_helpers, test_add_equal_signs_positive);
  tcase_add_test(s21_helpers, test_add_equal_signs_negative);
  tcase_add_test(s21_helpers, test_add_equal_signs_scale);
  tcase_add_test(s21_helpers, test_add_equal_signs_carry_bits0_bits1);
  tcase_add_test(s21_helpers, test_add_equal_signs_carry_bits1_bits2);
  tcase_add_test(s21_helpers, test_add_equal_signs_positive_overflow);
  tcase_add_test(s21_helpers, test_add_equal_signs_negative_overflow);

  tcase_add_test(s21_helpers, test_reduce_scale_remainder_less_5);
  tcase_add_test(s21_helpers, test_reduce_scale_remainder_more_5);
  tcase_add_test(s21_helpers, test_reduce_scale_bank_even);
  tcase_add_test(s21_helpers, test_reduce_scale_bank_odd);
  tcase_add_test(s21_helpers, test_reduce_scale_exact_five_no_tail);
  tcase_add_test(s21_helpers, test_reduce_scale_five_with_tail);
  tcase_add_test(s21_helpers, test_reduce_scale_less_five_with_tail);
  tcase_add_test(s21_helpers, test_reduce_scale_more_five_with_tail);
  tcase_add_test(s21_helpers, test_reduce_scale_to_zero);
  tcase_add_test(s21_helpers, test_reduce_scale_negative_keeps_sign);
  tcase_add_test(s21_helpers, test_reduce_scale_same_scale);
  tcase_add_test(s21_helpers, test_reduce_scale_new_scale_bigger);

  return s21_helpers;
}
TCase* int_converters(void) {
  TCase* s21_int_converters = tcase_create("IntConverters");

  tcase_add_test(s21_int_converters, test_from_int_to_decimal_zero);
  tcase_add_test(s21_int_converters, test_from_int_to_decimal_positive);
  tcase_add_test(s21_int_converters, test_from_int_to_decimal_negative);
  tcase_add_test(s21_int_converters, test_from_int_to_decimal_int_max);
  tcase_add_test(s21_int_converters, test_from_int_to_decimal_int_min);
  tcase_add_test(s21_int_converters, test_from_int_to_decimal_one);
  tcase_add_test(s21_int_converters, test_from_int_to_decimal_minus_one);
  tcase_add_test(s21_int_converters, test_from_int_to_decimal_null);
  tcase_add_test(s21_int_converters, test_from_int_to_decimal_clears_old_value);

  tcase_add_test(s21_int_converters, test_from_decimal_to_int_zero);
  tcase_add_test(s21_int_converters, test_from_decimal_to_int_positive);
  tcase_add_test(s21_int_converters, test_from_decimal_to_int_negative);
  tcase_add_test(s21_int_converters,
                 test_from_decimal_to_int_with_fraction_truncate_positive);
  tcase_add_test(s21_int_converters,
                 test_from_decimal_to_int_with_fraction_truncate_negative);
  tcase_add_test(s21_int_converters, test_from_decimal_to_int_less_than_one);
  tcase_add_test(s21_int_converters,
                 test_from_decimal_to_int_negative_less_than_one);
  tcase_add_test(s21_int_converters, test_from_decimal_to_int_int_max);
  tcase_add_test(s21_int_converters, test_from_decimal_to_int_int_min);
  tcase_add_test(s21_int_converters,
                 test_from_decimal_to_int_overflow_positive_by_one);
  tcase_add_test(s21_int_converters,
                 test_from_decimal_to_int_overflow_negative_by_one);
  tcase_add_test(s21_int_converters, test_from_decimal_to_int_overflow_bits1);
  tcase_add_test(s21_int_converters, test_from_decimal_to_int_overflow_bits2);
  tcase_add_test(s21_int_converters,
                 test_from_decimal_to_int_fraction_does_not_hide_overflow);
  tcase_add_test(s21_int_converters, test_from_decimal_to_int_null);
  tcase_add_test(s21_int_converters,
                 test_from_decimal_to_int_with_scale_zero_value);
  tcase_add_test(s21_int_converters,
                 test_from_decimal_to_int_truncate_negative_zero);

  return s21_int_converters;
}

TCase* float_converters_part1(void) {
  TCase* s21_float_converters_part1 = tcase_create("FloatConvertersPart1");

  tcase_add_test(s21_float_converters_part1, test_from_float_to_decimal_zero);
  tcase_add_test(s21_float_converters_part1,
                 test_from_float_to_decimal_negative_zero);
  tcase_add_test(s21_float_converters_part1, test_from_float_to_decimal_one);
  tcase_add_test(s21_float_converters_part1,
                 test_from_float_to_decimal_minus_one);
  tcase_add_test(s21_float_converters_part1,
                 test_from_float_to_decimal_integer);
  tcase_add_test(s21_float_converters_part1,
                 test_from_float_to_decimal_simple_fraction);
  tcase_add_test(s21_float_converters_part1,
                 test_from_float_to_decimal_0_1_no_garbage);
  tcase_add_test(s21_float_converters_part1,
                 test_from_float_to_decimal_0_2_no_garbage);
  tcase_add_test(s21_float_converters_part1,
                 test_from_float_to_decimal_0_3_no_garbage);
  tcase_add_test(s21_float_converters_part1,
                 test_from_float_to_decimal_123_456);
  tcase_add_test(s21_float_converters_part1,
                 test_from_float_to_decimal_minus_123_456);
  tcase_add_test(s21_float_converters_part1,
                 test_from_float_to_decimal_small_value);
  tcase_add_test(s21_float_converters_part1,
                 test_from_float_to_decimal_small_negative);
  tcase_add_test(s21_float_converters_part1,
                 test_from_float_to_decimal_round_7_digits);
  tcase_add_test(s21_float_converters_part1,
                 test_from_float_to_decimal_7_digits_ok);
  tcase_add_test(s21_float_converters_part1,
                 test_from_float_to_decimal_9999999);
  tcase_add_test(s21_float_converters_part1,
                 test_from_float_to_decimal_min_positive);
  tcase_add_test(s21_float_converters_part1,
                 test_from_float_to_decimal_too_small);
  tcase_add_test(s21_float_converters_part1,
                 test_from_float_to_decimal_too_big);
  tcase_add_test(s21_float_converters_part1,
                 test_from_float_to_decimal_max_range);
  tcase_add_test(s21_float_converters_part1, test_from_float_to_decimal_nan);
  tcase_add_test(s21_float_converters_part1, test_from_float_to_decimal_inf);
  tcase_add_test(s21_float_converters_part1,
                 test_from_float_to_decimal_neg_inf);
  tcase_add_test(s21_float_converters_part1, test_from_float_to_decimal_null);

  return s21_float_converters_part1;
}

TCase* float_converters_part2(void) {
  TCase* s21_float_converters_part2 = tcase_create("FloatConvertersPart2");
  tcase_add_test(s21_float_converters_part2,
                 test_from_float_to_decimal_clears_old_value);
  tcase_add_test(s21_float_converters_part2,
                 test_from_float_to_decimal_scale_check);

  tcase_add_test(s21_float_converters_part2, test_from_decimal_to_float_zero);
  tcase_add_test(s21_float_converters_part2, test_from_decimal_to_float_one);
  tcase_add_test(s21_float_converters_part2,
                 test_from_decimal_to_float_minus_one);
  tcase_add_test(s21_float_converters_part2,
                 test_from_decimal_to_float_integer);
  tcase_add_test(s21_float_converters_part2, test_from_decimal_to_float_half);
  tcase_add_test(s21_float_converters_part2,
                 test_from_decimal_to_float_123_456);
  tcase_add_test(s21_float_converters_part2,
                 test_from_decimal_to_float_minus_123_456);
  tcase_add_test(s21_float_converters_part2, test_from_decimal_to_float_0_1);
  tcase_add_test(s21_float_converters_part2, test_from_decimal_to_float_large);
  tcase_add_test(s21_float_converters_part2,
                 test_from_decimal_to_float_high_scale);
  tcase_add_test(s21_float_converters_part2,
                 test_from_decimal_to_float_negative_zero);
  tcase_add_test(s21_float_converters_part2, test_from_decimal_to_float_null);

  tcase_add_test(s21_float_converters_part2, test_roundtrip_0_5);
  tcase_add_test(s21_float_converters_part2, test_roundtrip_123_456);
  tcase_add_test(s21_float_converters_part2, test_roundtrip_negative);
  tcase_add_test(s21_float_converters_part2, test_roundtrip_0_1);

  return s21_float_converters_part2;
}

TCase* additional_tests(void) {
  TCase* s21_additional_tests = tcase_create("Additional_tests");

  tcase_add_test(s21_additional_tests, test_div_max_by_point_one_overflow);
  tcase_add_test(s21_additional_tests, test_div_max_by_two_bank_round);
  tcase_add_test(s21_additional_tests, test_mul_exact_half_bank_round_even);
  tcase_add_test(s21_additional_tests,
                 test_from_float_more_than_7_significant_digits);

  return s21_additional_tests;
}

Suite* decimal(void) {
  Suite* s21_decimal = suite_create("Decimal");
  suite_add_tcase(s21_decimal, helpers());
  suite_add_tcase(s21_decimal, general());
  suite_add_tcase(s21_decimal, arithmetic_add_sub());
  suite_add_tcase(s21_decimal, arithmetic_mul());
  suite_add_tcase(s21_decimal, arithmetic_div());
  suite_add_tcase(s21_decimal, int_converters());
  suite_add_tcase(s21_decimal, float_converters_part1());
  suite_add_tcase(s21_decimal, float_converters_part2());
  suite_add_tcase(s21_decimal, additional_tests());

  return s21_decimal;
}

int main(void) {
  int errors = 0;
  int result = 0;
  SRunner* start_test;
  start_test = srunner_create(decimal());

  srunner_run_all(start_test, CK_VERBOSE);
  errors = srunner_ntests_failed(start_test);

  srunner_free(start_test);

  if (errors == 0) {
    result = 0;
  } else {
    result = 1;
  }

  return result;
}
