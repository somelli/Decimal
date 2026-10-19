GCC = gcc
FLAGS = -Wall -Wextra -Werror -std=c11
LDFLAGS = -lm
LIB = s21_decimal.a
AR = ar
OBJS = s21_decimal.o s21_bits.o s21_mantissa.o s21_div10.o s21_scale.o \
       s21_rounding.o s21_add_internal.o s21_mul_internal.o \
       s21_div_internal.o s21_float_internal.o
CHECK_FLAGS = $(shell pkg-config --cflags --libs check)
GCOV_FLAGS = --coverage
ALL_C = test_s21_decimal.c s21_decimal.c s21_bits.c s21_mantissa.c s21_div10.c \
        s21_scale.c s21_rounding.c s21_add_internal.c s21_mul_internal.c \
        s21_div_internal.c s21_float_internal.c
.PHONY: clean test all gcov_report

all: $(LIB)

$(LIB): $(OBJS)
	$(AR) rcs $(LIB) $(OBJS)

s21_decimal.o: s21_decimal.c s21_decimal.h
	$(GCC) $(FLAGS) -c s21_decimal.c -o s21_decimal.o

s21_bits.o: s21_bits.c s21_bits.h s21_decimal.h
	$(GCC) $(FLAGS) -c s21_bits.c -o s21_bits.o

s21_mantissa.o: s21_mantissa.c s21_mantissa.h s21_bits.h s21_decimal.h
	$(GCC) $(FLAGS) -c s21_mantissa.c -o s21_mantissa.o

s21_div10.o: s21_div10.c s21_div10.h s21_decimal.h
	$(GCC) $(FLAGS) -c s21_div10.c -o s21_div10.o

s21_rounding.o: s21_rounding.c s21_rounding.h s21_mantissa.h s21_decimal.h
	$(GCC) $(FLAGS) -c s21_rounding.c -o s21_rounding.o

s21_scale.o: s21_scale.c s21_scale.h s21_bits.h s21_mantissa.h s21_div10.h s21_rounding.h s21_decimal.h
	$(GCC) $(FLAGS) -c s21_scale.c -o s21_scale.o

s21_add_internal.o: s21_add_internal.c s21_add_internal.h s21_bits.h s21_mantissa.h s21_div10.h s21_rounding.h s21_decimal.h
	$(GCC) $(FLAGS) -c s21_add_internal.c -o s21_add_internal.o

s21_mul_internal.o: s21_mul_internal.c s21_mul_internal.h s21_div10.h s21_decimal.h
	$(GCC) $(FLAGS) -c s21_mul_internal.c -o s21_mul_internal.o

s21_div_internal.o: s21_div_internal.c s21_div_internal.h s21_bits.h s21_mantissa.h s21_rounding.h s21_decimal.h
	$(GCC) $(FLAGS) -c s21_div_internal.c -o s21_div_internal.o

s21_float_internal.o: s21_float_internal.c s21_float_internal.h s21_bits.h s21_decimal.h
	$(GCC) $(FLAGS) -c s21_float_internal.c -o s21_float_internal.o

clean:
	rm -f $(LIB) $(OBJS) test coverage.info *.gcda *.gcno
	rm -rf report

test: $(LIB)
	$(GCC) $(FLAGS) test_s21_decimal.c $(LIB) $(CHECK_FLAGS) $(LDFLAGS) -o test
	./test

valgrind: $(LIB)
	$(GCC) $(FLAGS) test_s21_decimal.c $(LIB) $(CHECK_FLAGS) $(LDFLAGS) -o test
	valgrind --tool=memcheck --leak-check=full \
	--show-leak-kinds=all --track-origins=yes \
	./test

gcov_report:
	$(GCC) $(FLAGS) $(GCOV_FLAGS) $(ALL_C) $(CHECK_FLAGS) $(LDFLAGS) -o test
	./test
	lcov --capture --directory . --output-file coverage.info
	lcov --remove coverage.info "*/test_s21_decimal.c" --output-file coverage.info
	genhtml coverage.info --output-directory report