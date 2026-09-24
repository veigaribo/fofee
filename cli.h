#ifndef _FOFEE_CLI
#define _FOFEE_CLI

#include "cow.h"
#include <stdbool.h>
#include <stdint.h>

#define X_CLI_ACTIONS                                                          \
  X(NONE)                                                                      \
  X(HELP)                                                                      \
  X(LS)                                                                        \
  X(PLAY)

#define X(Arg) ACTION_##Arg,

// ACTION_NONE, ACTION_HELP, etc.
typedef enum { X_CLI_ACTIONS } cli_action_t;
#undef X

char *repr_cli_action(cli_action_t action);

typedef struct {
  cli_action_t action;
  uint16_t delay;
  uint16_t duration;
  uint16_t direction;

  // Play
  struct {
    char *dev;
    uint8_t effect;
    // int16_t level;

    // Periodic
    // TODO: Verify that these values are as the comments describe. Very little
    // documentation out there.
    uint16_t waveform;
    uint16_t period;   // In ms, apparently
    int16_t magnitude; // Assuming relative to INT16_MAX
    int16_t offset;    // Assuming relative to INT16_MAX
    uint16_t phase;    // Assuming in ms

    uint16_t attack_length; // Assuming in ms
    uint16_t attack_level; // Assuming relative to UINT16_MAX
    uint16_t fade_length; // Assuming in ms
    uint16_t fade_level; // Assuming relative to UINT16_MAX
  };
} cli_params_t;

typedef struct {
  bool success;
  union {
    cli_params_t params;
    cow_t error_message;
  };
} cli_parse_result_t;

cli_parse_result_t cli_parse(int arg, char *argv[]);

#endif // !_FOFEE_CLI
