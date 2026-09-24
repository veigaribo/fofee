#ifndef _FOFEE_COMMON
#define _FOFEE_COMMON

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define BITS_TO_BYTES(x)                                                       \
  (((x) + 8 * sizeof(uint8_t) - 1) / (8 * sizeof(uint8_t)))

static inline bool is_bit_set(void *bits, size_t bitn) {
  uint8_t *bytes = (uint8_t *)bits;
  return (bytes[bitn / 8] >> (bitn % 8)) & 1;
}

#define X_EFFECTS                                                              \
  X(HAPTIC)                                                                    \
  X(RUMBLE)                                                                    \
  X(PERIODIC) X(CONSTANT) X(SPRING) X(FRICTION) X(DAMPER) X(INERTIA) X(RAMP)

#define X_WAVEFORMS                                                            \
  X(SQUARE)                                                                    \
  X(TRIANGLE)                                                                  \
  X(SINE) X(SAW_UP) X(SAW_DOWN) X(CUSTOM)

#endif // !_FOFEE_COMMON
