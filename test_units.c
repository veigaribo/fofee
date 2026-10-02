#include "units.h"
#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

int time_test(int target, ...) {
  va_list args;
  uint16_t ms;
  parse_result_t parse_result;
  int result = 0;

  va_start(args, target);
  while (true) {
    while (true) {
      char *example = va_arg(args, char *);
      if (example == NULL)
        break;

      parse_result = parse_time(example, &ms);

      if (parse_result != PARSE_OK) {
        fprintf(stderr, "failed to parse time `%s`: %d. expected to be %d.\n",
                example, parse_result, target);
        result = 1;
        continue;
      }

      if (ms != target) {
        fprintf(stderr, "time `%s` expected to be %d is %d instead.\n", example,
                target, ms);
        result = 1;
      }
    }

    target = va_arg(args, int);
    if (target == -1)
      break;
  }

  va_end(args);
  return result;
}

int time_test_result(char *src, parse_result_t expect) {
  uint16_t ms;
  parse_result_t parse_result = parse_time(src, &ms);
  if (parse_result != expect) {
    fprintf(stderr, "expected result %s for time `%s`; instead got %s.\n",
            parse_result_get_name(expect), src,
            parse_result_get_name(parse_result));
    return 1;
  }
  return 0;
}

int direction_test(int target, ...) {
  va_list args;
  uint16_t direction;
  parse_result_t parse_result;
  int result = 0;

  va_start(args, target);
  while (true) {
    while (true) {
      char *example = va_arg(args, char *);
      if (example == NULL)
        break;

      parse_result = parse_direction(example, &direction);

      if (parse_result != PARSE_OK) {
        fprintf(stderr,
                "failed to parse direction `%s`: %d. expected to be %x.\n",
                example, parse_result, target);
        result = 1;
        continue;
      }

      if (abs(direction - target) > 0x10) {
        fprintf(stderr, "direction `%s` expected to be %x is %x instead.\n",
                example, target, direction);
        result = 1;
      }
    }

    target = va_arg(args, int);
    if (target == -1)
      break;
  }

  va_end(args);
  return result;
}

int direction_test_result(char *src, parse_result_t expect) {
  uint16_t direction;
  parse_result_t parse_result = parse_direction(src, &direction);
  if (parse_result != expect) {
    fprintf(stderr, "expected result %s for direction `%s`; instead got %s.\n",
            parse_result_get_name(expect), src,
            parse_result_get_name(parse_result));
    return 1;
  }
  return 0;
}

int relative_test(uint32_t max, int target, ...) {
  va_list args;
  int64_t value;
  parse_result_t parse_result;
  int result = 0;

  va_start(args, target);
  while (true) {
    while (true) {
      char *example = va_arg(args, char *);
      if (example == NULL)
        break;

      parse_result = parse_relative(example, max, &value);

      if (parse_result != PARSE_OK) {
        fprintf(
          stderr,
          "failed to parse relative `%s`: %d. expected to be %d out of %d.\n",
          example, parse_result, target, max);
        result = 1;
        continue;
      }

      if (value != target) {
        fprintf(stderr,
                "relative `%s` expected to be %d is %ld out of %d instead.\n",
                example, target, value, max);
        result = 1;
      }
    }

    target = va_arg(args, int);
    if (target == -1)
      break;
  }

  va_end(args);
  return result;
}

int relative_test_result(char *src, parse_result_t expect) {
  int64_t value;
  parse_result_t parse_result = parse_relative(src, 0, &value);
  if (parse_result != expect) {
    fprintf(stderr, "expected result %s for relative `%s`; instead got %s.\n",
            parse_result_get_name(expect), src,
            parse_result_get_name(parse_result));
    return 1;
  }
  return 0;
}

int main() {
  uint64_t ms;
  uint16_t dir;
  int16_t rel;
  int result = 0;

  // clang-format off
  result |= time_test(
    24,            "24ms", "24 MS", NULL,
    24 * 1000,     "24s", "24 S", NULL,
    1 * 1000 * 60, "1min", "1 MIN", NULL,

    24,            " 24ms", " 24 MS", NULL,
    24 * 1000,     " 24s", " 24 S", NULL,
    1 * 1000 * 60, " 1min", " 1 MIN", NULL,

    24,            "24ms ", " 24 MS", NULL,
    24 * 1000,     "24s ", " 24 S", NULL,
    1 * 1000 * 60, "1min ", " 1 MIN", NULL,
    -1);

  result |= direction_test(
    0x8000, "up",    "UP",    "90deg",  "-270deg", "1.5707963267948966rad", NULL,
    0x0000, "down",  "DOWN",  "270deg", "-90deg",  "4.71238898038469rad", NULL,
    0x4000, "left",  "LEFT",  "180deg", "-180deg", "3.141592653589793rad", NULL,
    0xC000, "right", "RIGHT", "0deg",   "-0deg",   "0rad", NULL,

    0x8000, " up",    " UP",    " 90deg",  " -270deg", " 1.5707963267948966rad", NULL,
    0x0000, " down",  " DOWN",  " 270deg", " -90deg",  " 4.71238898038469rad", NULL,
    0x4000, " left",  " LEFT",  " 180deg", " -180deg", " 3.141592653589793rad", NULL,
    0xC000, " right", " RIGHT", " 0deg",   " -0deg",   " 0rad", NULL,

    0x8000, "up ",    "UP ",    "90deg ",  "-270deg ", "1.5707963267948966rad ", NULL,
    0x0000, "down ",  "DOWN ",  "270deg ", "-90deg ",  "4.71238898038469rad ", NULL,
    0x4000, "left ",  "LEFT ",  "180deg ", "-180deg ", "3.141592653589793rad ", NULL,
    0xC000, "right ", "RIGHT ", "0deg ",   "-0deg ",   "0rad ", NULL,
    -1);

  result |= relative_test(1000,
    500, "50%", "1/2", "2/4", "0.5", NULL,
    250, "25%", "1/4", "3/12", "0.25", NULL,

    500, " 50%", " 1/2", " 2/4",  " 0.5", NULL,
    250, " 25%", " 1/4", " 3/12", " 0.25", NULL,

    500, "50% ", "1/2 ", "2/4 ",  "0.5 ", NULL,
    250, "25% ", "1/4 ", "3/12 ", "0.25 ", NULL,
    -1);
  // clang-format on

  result |= time_test_result("a4mst", PARSE_INVALID_SCALAR);
  result |= time_test_result("24mst", PARSE_INVALID_UNIT);
  result |= time_test_result("24ms t", PARSE_INVALID_UNIT);

  result |= direction_test_result("a4deg", PARSE_INVALID_SCALAR);
  result |= direction_test_result("24degt", PARSE_INVALID_UNIT);
  result |= direction_test_result("24deg t", PARSE_INVALID_UNIT);

  result |= relative_test_result("a4%", PARSE_INVALID_SCALAR);
  result |= relative_test_result("24%t", PARSE_INVALID_UNIT);
  result |= relative_test_result("24% t", PARSE_INVALID_UNIT);

  if (result == 0) {
    printf("OK\n");
  }

  return result;
}
