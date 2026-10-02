#include "units.h"
#include <ctype.h>
#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

static inline void gobble_whitespace(char **src) {
  char *head = *src;
  while (isspace(head[0])) {
    ++head;
  }
  *src = head;
}

static inline bool is_a_number(double d) {
  return d != NAN && d != INFINITY && d != -INFINITY && d != HUGE_VAL;
}

static parse_result_t check_bounds_u16(double value, uint16_t *result) {
  if (value >= 0 && value <= UINT16_MAX) {
    *result = (uint16_t)value;
    return PARSE_OK;
  } else {
    return PARSE_SCALAR_OOB;
  }
}

// Check if `candidate` starts with `literal` and is followed exclusively by
// whitespace (if anything).
static inline int strautoncasecmp(const char *const literal,
                                  const char *candidate) {
  size_t len = strlen(literal);
  int equals = strncasecmp(literal, candidate, len);

  if (equals == 0) {
    const char *suffix = candidate + len;
    gobble_whitespace((char **)&suffix);

    if (suffix[0] == '\0') {
      return equals;
    } else {
      return -1;
    }
  }

  return equals;
}

parse_result_t parse_time(char *src, uint16_t *result) {
  char *end;
  double scalar = strtod(src, &end), value;

  if (end == src) {
    return PARSE_INVALID_SCALAR;
  }

  if (!is_a_number(scalar)) {
    return PARSE_INVALID_SCALAR;
  }

  src = end;
  gobble_whitespace(&src);

  // Allow unitless zero
  if (src[0] == '\0') {
    if (scalar == 0) {
      *result = 0;
      return PARSE_OK;
    } else {
      return PARSE_INVALID_UNIT;
    }
  }

  if (strautoncasecmp("ms", src) == 0) {
    return check_bounds_u16(scalar, result);
  }

  if (strautoncasecmp("s", src) == 0) {
    return check_bounds_u16(scalar * 1000, result);
  }

  if (strautoncasecmp("min", src) == 0) {
    return check_bounds_u16(scalar * 1000 * 60, result);
  }

  return PARSE_INVALID_UNIT;
}

parse_result_t parse_direction(char *src, uint16_t *result) {
  gobble_whitespace(&src);

  if (isdigit(src[0])) {
    goto parse_digit;
  }

  if (strautoncasecmp("up", src) == 0) {
    *result = 0x8000;
    return PARSE_OK;
  }

  if (strautoncasecmp("down", src) == 0) {
    *result = 0x0000;
    return PARSE_OK;
  }

  if (strautoncasecmp("left", src) == 0) {
    *result = 0x4000;
    return PARSE_OK;
  }

  if (strautoncasecmp("right", src) == 0) {
    *result = 0xC000;
    return PARSE_OK;
  }

  char *end;
  double scalar;

parse_digit:
  scalar = strtod(src, &end);

  if (end == src) {
    return PARSE_INVALID_SCALAR;
  }

  if (!is_a_number(scalar)) {
    return PARSE_INVALID_SCALAR;
  }

  src = end;
  gobble_whitespace(&src);

  // Allow unitless zero
  if (src[0] == '\0') {
    if (scalar == 0) {
      *result = 0;
      return PARSE_OK;
    } else {
      return PARSE_INVALID_UNIT;
    }
  }

  if (strautoncasecmp("deg", src) == 0) {
    scalar = fmod(scalar, 360.0);
    *result = (uint16_t)((-(scalar + 90) / 360.0) * 0xFFFF);
    return PARSE_OK;
  }

  if (strautoncasecmp("rad", src) == 0) {
    scalar = fmod(scalar, 2 * M_PI);
    *result = (uint16_t)((-(scalar + M_PI_2) / (2.0 * M_PI)) * 0xFFFF);
    return PARSE_OK;
  }

  return PARSE_INVALID_UNIT;
}

static parse_result_t check_bounds(double value, int64_t min, int64_t max,
                                   int64_t *result) {
  if (value >= min && value <= max) {
    *result = (int64_t)value;
    return PARSE_OK;
  } else {
    return PARSE_SCALAR_OOB;
  }
}

// In every case where this is needed we cannot allow > max (e.g. 200%)
parse_result_t parse_relative(char *src, uint32_t max, int64_t *result) {
  int64_t min = -((int64_t)max) - 1;
  char *end;
  double numerator = strtod(src, &end), denominator = 1.0, value;

  if (end == src) {
    return PARSE_INVALID_SCALAR;
  }

  if (!is_a_number(numerator)) {
    return PARSE_INVALID_SCALAR;
  }

  src = end;
  gobble_whitespace(&src);

  if (src[0] == '\0') {
    // Unitless
    return check_bounds(numerator * max, min, max, result);
  }

  if (strautoncasecmp("%", src) == 0) {
    return check_bounds((numerator / 100) * max, min, max, result);
  }

  if (strncasecmp("/", src, 1) == 0) {
    denominator = strtod(src + 1, &end);

    if (end == src + 1) {
      return PARSE_INVALID_SCALAR;
    }

    if (!is_a_number(numerator)) {
      return PARSE_INVALID_SCALAR;
    }

    return check_bounds((numerator / denominator) * max, min, max, result);
  }

  return PARSE_INVALID_UNIT;
}

#define X(Arg)                                                                 \
  case Arg:                                                                    \
    return #Arg;

char *parse_result_get_name(parse_result_t result) {
  switch (result) { X_PARSE_RESULT }
  return "INVALID";
}

#undef X
