#include "s21_div10.h"

// универсальное деление массива на 10 с заданным начальным остатком
int s21_div_array_by_10_ex(unsigned int* arr, int size,
                           unsigned long long init_rem) {
  unsigned long long remainder = init_rem;
  for (int i = size - 1; i >= 0; i--) {
    unsigned long long dividend = (remainder << 32) + arr[i];
    arr[i] = (unsigned int)(dividend / 10);
    remainder = dividend % 10;
  }
  return (int)remainder;
}

// деление массива на 10 (init_rem = 0)
int s21_div_array_by_10(unsigned int* arr, int size) {
  return s21_div_array_by_10_ex(arr, size, 0);
}

// деление мантиссы s21_decimal на 10
int s21_div_mantissa_by_10(s21_decimal value, s21_decimal* result) {
  *result = value;
  return s21_div_array_by_10_ex((unsigned int*)result->bits, 3, 0);
}

// деление переполненной 97-битной мантиссы на 10
int s21_div_overflow_mantissa_by_10(s21_decimal* value) {
  return s21_div_array_by_10_ex((unsigned int*)value->bits, 3, 1);
}
