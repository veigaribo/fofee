#ifndef _FOFEE_UNITS
#define _FOFEE_UNITS

#include <stdint.h>
#include <sys/types.h>

// Parsers for things like "180deg", "50%", "5s".

typedef enum {
  PARSE_OK = 0,
  PARSE_INVALID_SCALAR = -1,
  PARSE_INVALID_UNIT = -2,
  PARSE_SCALAR_OOB = -3,
} parse_result_t;

#define X_PARSE_RESULT                                                         \
  X(PARSE_OK) X(PARSE_INVALID_SCALAR) X(PARSE_INVALID_UNIT) X(PARSE_SCALAR_OOB)

// Result in milliseconds
parse_result_t parse_time(char *src, uint16_t *result); // ms, s, min...

// Result is an uint16_t as follows:
// 0x0000 -> down
// 0x4000 -> left
// 0x8000 -> up
// 0xC000 -> right
//
// Input angles map to:
// 0 deg -> right (0xC000)
// 90 deg -> up (0x8000)
// 180 deg -> left (0x4000)
// 270 deg -> down (0x0000)
parse_result_t parse_direction(char *src,
                               uint16_t *result); // up, 90deg, 1rad...

// Result: 100% -> max, 0% -> 0, -100% -> -max
parse_result_t parse_relative(char *src, uint32_t max,
                              int64_t *result); // 0.5, 50%, 1/2...

char *parse_result_get_name(parse_result_t result);

#endif // !_FOFEE_UNITS
