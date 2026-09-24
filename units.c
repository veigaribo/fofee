#include "units.h"
#include <ctype.h>
#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <strings.h>

static bool is_a_number(double d) {
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
  while (isspace(src[0])) {
    ++src;
  }

  // Allow unitless zero
  if (src[0] == '\0') {
    if (scalar == 0) {
      *result = 0;
      return PARSE_OK;
    } else {
      return PARSE_INVALID_UNIT;
    }
  }

  if (strcasecmp("ms", src) == 0) {
    return check_bounds_u16(scalar, result);
  }

  if (strcasecmp("s", src) == 0) {
    return check_bounds_u16(scalar * 1000, result);
  }

  if (strcasecmp("min", src) == 0) {
    return check_bounds_u16(scalar * 1000 * 60, result);
  }

  return PARSE_INVALID_UNIT;
}

parse_result_t parse_direction(char *src, uint16_t *result) {
  while (isspace(src[0])) {
    ++src;
  }

  if (isdigit(src[0])) {
    goto parse_digit;
  }

  if (strcasecmp("up", src) == 0) {
    *result = 0x8000;
    return PARSE_OK;
  }

  if (strcasecmp("down", src) == 0) {
    *result = 0x0000;
    return PARSE_OK;
  }

  if (strcasecmp("left", src) == 0) {
    *result = 0x4000;
    return PARSE_OK;
  }

  if (strcasecmp("right", src) == 0) {
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
  while (isspace(src[0])) {
    ++src;
  }

  // Allow unitless zero
  if (src[0] == '\0') {
    if (scalar == 0) {
      *result = 0;
      return PARSE_OK;
    } else {
      return PARSE_INVALID_UNIT;
    }
  }

  if (strcasecmp("deg", src) == 0) {
    scalar = fmod(scalar, 360.0);
    *result = (uint16_t)((-(scalar + 90) / 360.0) * 0xFFFF);
    return PARSE_OK;
  }

  if (strcasecmp("rad", src) == 0) {
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
  while (isspace(src[0])) {
    ++src;
  }

  if (strcasecmp("%", src) == 0) {
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

  return check_bounds(numerator * max, min, max, result);
}
