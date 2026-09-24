#include "cli.h"
#include "common.h"
#include "units.h"
#include <assert.h>
#include <linux/input.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define X(Arg)                                                                 \
  case ACTION_##Arg:                                                           \
    return #Arg;                                                               \
    break;

char *repr_cli_action(cli_action_t action) {
  switch (action) { X_CLI_ACTIONS }

  assert(false);
  return "?";
}

#undef X

typedef struct {
  int argc;
  char **argv;

  int current_arg;
} parse_state_t;

#define ParseState(Argc, Argv, CurrentArg)                                     \
  ((parse_state_t){.argc = Argc, .argv = Argv, .current_arg = CurrentArg})

static char *state_get_current_arg(parse_state_t state) {
  return state.argv[state.current_arg];
}

static char *state_advance(parse_state_t *state) {
  return state->argv[state->current_arg++];
}

static char *state_peek(parse_state_t state) {
  return state.argv[state.current_arg + 1];
}

static bool state_is_finished(parse_state_t state) {
  return state.current_arg == state.argc;
}

static bool state_is_last(parse_state_t state) {
  return state.current_arg + 1 == state.argc;
}

typedef enum {
  ERROR_UNRECOGNIZED, // Not what I expected at all (maybe try something else)
  ERROR_INVALID,      // Looks like what I expect but it's wrong (abort)
} parse_error_t;

#define DefParseResult(Name, V)                                                \
  typedef struct {                                                             \
    bool success;                                                              \
                                                                               \
    union {                                                                    \
      struct {                                                                 \
        parse_state_t state;                                                   \
        V value;                                                               \
      };                                                                       \
      struct {                                                                 \
        cow_t error_message;                                                   \
        uint8_t error_code;                                                    \
      };                                                                       \
    };                                                                         \
  } Name

#define ParseSuccess(T, S, V) ((T){.success = true, .state = S, .value = V})
#define ParseFail(T, Code, Msg)                                                \
  ((T){.success = false, .error_code = Code, .error_message = Msg})

static uint8_t get_digits_in_number(uint64_t d) {
  uint8_t acc = 0;
  while (d > 0) {
    ++acc;
    d /= 10;
  }
  return acc;
}

#define MkErrStr1S(VarName, Format, S)                                         \
  char *__format = Format;                                                     \
  char *__s = S;                                                               \
  size_t __length = strlen(__format) - 2 + strlen(__s) + 1;                    \
  /* We are not bothering with freeing this */                                 \
  char *VarName = malloc(__length);                                            \
  snprintf(VarName, __length, __format, __s);

#define MkErrStr2D(VarName, Format, D1, D2)                                    \
  char *__format = Format;                                                     \
  int64_t __d1 = D1;                                                           \
  int64_t __d2 = D2;                                                           \
  size_t __length = strlen(__format) - 2 + get_digits_in_number(__d1) - 2 +    \
                    get_digits_in_number(__d2) + 1;                            \
  /* We are not bothering with freeing this */                                 \
  char *VarName = malloc(__length);                                            \
  snprintf(VarName, __length, __format, __d1, __d2);

DefParseResult(parse_result_void_t, uint8_t); // Ideally it wouldn't have a V
                                              // at all, but that's not possible
                                              // like this
DefParseResult(parse_result_effect_t, uint8_t);
DefParseResult(parse_result_u16_t, uint16_t);
DefParseResult(parse_result_i16_t, int16_t);
DefParseResult(parse_result_i64_t, int64_t);
DefParseResult(parse_result_action_t, cli_action_t);
DefParseResult(parse_result_str_t, char *);

// cli_parse_effect_arg

#define X(Arg)                                                                 \
  if (strcasecmp(#Arg, arg) == 0) {                                            \
    return ParseSuccess(parse_result_effect_t, state, FF_##Arg);               \
  }

static parse_result_effect_t cli_parse_effect(parse_state_t state) {
  char *arg = state_advance(&state);
  X_EFFECTS;

  MkErrStr1S(msg, "unrecognized effect `%s`", arg);
  return ParseFail(parse_result_effect_t, ERROR_UNRECOGNIZED,
                   cow_mk_owned(msg));
}

#undef X

// cli_parse_time_arg

static parse_result_u16_t cli_parse_time(parse_state_t state) {
  char *arg = state_advance(&state), *end;
  uint16_t time_ms;
  parse_result_t parse_result = parse_time(arg, &time_ms);

  if (parse_result == PARSE_OK) {
    return ParseSuccess(parse_result_u16_t, state, time_ms);
  } else if (parse_result == PARSE_SCALAR_OOB) {
    MkErrStr1S(
      msg, "value of `%s` out of bounds. must be between 0 and 65535ms (~1min)",
      arg);
    return ParseFail(parse_result_u16_t, ERROR_INVALID, cow_mk_owned(msg));
  } else {
    MkErrStr1S(msg, "failed to parse duration `%s`", arg);
    return ParseFail(parse_result_u16_t, ERROR_UNRECOGNIZED, cow_mk_owned(msg));
  }
}

// cli_parse_direction_arg

static parse_result_u16_t cli_parse_direction(parse_state_t state) {
  char *arg = state_advance(&state), *end;
  uint16_t direction;
  parse_result_t parse_result = parse_direction(arg, &direction);

  if (parse_result == PARSE_OK) {
    return ParseSuccess(parse_result_u16_t, state, direction);
  } else {
    MkErrStr1S(msg, "failed to parse direction `%s`", arg);
    return ParseFail(parse_result_u16_t, ERROR_UNRECOGNIZED, cow_mk_owned(msg));
  }
}

// cli_parse_relative_arg

static parse_result_i64_t cli_parse_relative(parse_state_t state,
                                             uint32_t max) {
  char *arg = state_advance(&state), *end;
  int64_t value;
  parse_result_t parse_result = parse_relative(arg, max, &value);

  if (parse_result == PARSE_OK) {
    return ParseSuccess(parse_result_i64_t, state, value);
  } else if (parse_result == PARSE_SCALAR_OOB) {
    MkErrStr1S(msg,
               "value of `%s` out of bounds. must be between -100%% and +100%%",
               arg);
    return ParseFail(parse_result_i64_t, ERROR_INVALID, cow_mk_owned(msg));
  } else {
    MkErrStr1S(msg, "failed to parse relative value `%s`", arg);
    return ParseFail(parse_result_i64_t, ERROR_UNRECOGNIZED, cow_mk_owned(msg));
  }
}

// cli_parse_str

static parse_result_str_t cli_parse_str(parse_state_t state) {
  char *arg = state_advance(&state);
  return ParseSuccess(parse_result_str_t, state, arg);
}

// cli_parse_waveform_arg

#define X(Arg)                                                                 \
  if (strcasecmp(#Arg, arg) == 0) {                                            \
    return ParseSuccess(parse_result_effect_t, state, FF_##Arg);               \
  }

static parse_result_effect_t cli_parse_waveform(parse_state_t state) {
  char *arg = state_advance(&state);
  X_WAVEFORMS;

  MkErrStr1S(msg, "unrecognized waveform `%s`", arg);
  return ParseFail(parse_result_effect_t, ERROR_UNRECOGNIZED,
                   cow_mk_owned(msg));
}

#undef X

// cli_parse_flag

static parse_result_void_t cli_parse_flag(parse_state_t state,
                                          cli_params_t *params) {
  char *arg = state_advance(&state);
  bool is_flag = arg[0] == '-' && arg[1] != '\0';

  if (!is_flag)
    return ParseFail(parse_result_void_t, ERROR_UNRECOGNIZED,
                     cow_mk_ref("not a flag"));

  // Support both -f and --f
  ptrdiff_t flag_name_start = 1;
  if (arg[1] == '-') {
    ++flag_name_start;
  }

  char *flag_name = arg + flag_name_start;

  if (strcmp("help", flag_name) == 0) {
    params->action = ACTION_HELP; // Nothing will overwrite this
    return ParseSuccess(parse_result_void_t, state, 0);
  }

  if (strcmp("effect", flag_name) == 0) {
    if (state_is_finished(state)) {
      return ParseFail(parse_result_void_t, ERROR_INVALID,
                       cow_mk_ref("expected value for flag --effect"));
    }

    parse_result_effect_t result = cli_parse_effect(state);

    if (result.success) {
      params->effect = result.value;
      return ParseSuccess(parse_result_void_t, result.state, 0);
    } else {
      return ParseFail(parse_result_void_t, ERROR_INVALID,
                       result.error_message);
    }
  }

  if (strcmp("delay", flag_name) == 0) {
    if (state_is_finished(state)) {
      return ParseFail(parse_result_void_t, ERROR_INVALID,
                       cow_mk_ref("expected value for flag --delay"));
    }

    parse_result_u16_t result = cli_parse_time(state);

    if (result.success) {
      params->delay = result.value;
      return ParseSuccess(parse_result_void_t, result.state, 0);
    } else {
      return ParseFail(parse_result_void_t, ERROR_INVALID,
                       result.error_message);
    }
  }

  if (strcmp("duration", flag_name) == 0) {
    if (state_is_finished(state)) {
      return ParseFail(parse_result_void_t, ERROR_INVALID,
                       cow_mk_ref("expected value for flag --duration"));
    }

    parse_result_u16_t result = cli_parse_time(state);

    if (result.success) {
      params->duration = result.value;
      return ParseSuccess(parse_result_void_t, result.state, 0);
    } else {
      return ParseFail(parse_result_void_t, ERROR_INVALID,
                       result.error_message);
    }
  }

  if (strcmp("direction", flag_name) == 0) {
    if (state_is_finished(state)) {
      return ParseFail(parse_result_void_t, ERROR_INVALID,
                       cow_mk_ref("expected value for flag --direction"));
    }

    parse_result_u16_t result = cli_parse_direction(state);

    if (result.success) {
      params->direction = result.value;
      return ParseSuccess(parse_result_void_t, result.state, 0);
    } else {
      return ParseFail(parse_result_void_t, ERROR_INVALID,
                       result.error_message);
    }
  }

  // Periodic flags

  if (strcmp("waveform", flag_name) == 0) {
    if (state_is_finished(state)) {
      return ParseFail(parse_result_void_t, ERROR_INVALID,
                       cow_mk_ref("expected value for flag --waveform"));
    }

    parse_result_effect_t result = cli_parse_waveform(state);

    if (result.success) {
      params->waveform = result.value;
      return ParseSuccess(parse_result_void_t, result.state, 0);
    } else {
      return ParseFail(parse_result_void_t, ERROR_INVALID,
                       result.error_message);
    }
  }

  if (strcmp("period", flag_name) == 0) {
    if (state_is_finished(state)) {
      return ParseFail(parse_result_void_t, ERROR_INVALID,
                       cow_mk_ref("expected value for flag --period"));
    }

    parse_result_u16_t result = cli_parse_time(state);

    if (result.success) {
      params->period = result.value;
      return ParseSuccess(parse_result_void_t, result.state, 0);
    } else {
      return ParseFail(parse_result_void_t, ERROR_INVALID,
                       result.error_message);
    }
  }

  if (strcmp("magnitude", flag_name) == 0) {
    if (state_is_finished(state)) {
      return ParseFail(parse_result_void_t, ERROR_INVALID,
                       cow_mk_ref("expected value for flag --magnitude"));
    }

    parse_result_i64_t result = cli_parse_relative(state, INT16_MAX);

    if (result.success) {
      params->magnitude = (int16_t)result.value;
      return ParseSuccess(parse_result_void_t, result.state, 0);
    } else {
      return ParseFail(parse_result_void_t, ERROR_INVALID,
                       result.error_message);
    }
  }

  if (strcmp("offset", flag_name) == 0) {
    if (state_is_finished(state)) {
      return ParseFail(parse_result_void_t, ERROR_INVALID,
                       cow_mk_ref("expected value for flag --offset"));
    }

    parse_result_i64_t result = cli_parse_relative(state, INT16_MAX);

    if (result.success) {
      params->offset = (int16_t)result.value;
      return ParseSuccess(parse_result_void_t, result.state, 0);
    } else {
      return ParseFail(parse_result_void_t, ERROR_INVALID,
                       result.error_message);
    }
  }

  if (strcmp("phase", flag_name) == 0) {
    if (state_is_finished(state)) {
      return ParseFail(parse_result_void_t, ERROR_INVALID,
                       cow_mk_ref("expected value for flag --phase"));
    }

    parse_result_i64_t result = cli_parse_relative(state, UINT16_MAX);

    if (result.success) {
      params->phase = (uint16_t)result.value;
      return ParseSuccess(parse_result_void_t, result.state, 0);
    } else {
      return ParseFail(parse_result_void_t, ERROR_INVALID,
                       result.error_message);
    }
  }

  if (strcmp("attack-length", flag_name) == 0) {
    if (state_is_finished(state)) {
      return ParseFail(parse_result_void_t, ERROR_INVALID,
                       cow_mk_ref("expected value for flag --attack-length"));
    }

    parse_result_u16_t result = cli_parse_time(state);

    if (result.success) {
      params->attack_length = result.value;
      return ParseSuccess(parse_result_void_t, result.state, 0);
    } else {
      return ParseFail(parse_result_void_t, ERROR_INVALID,
                       result.error_message);
    }
  }

  if (strcmp("attack-level", flag_name) == 0) {
    if (state_is_finished(state)) {
      return ParseFail(parse_result_void_t, ERROR_INVALID,
                       cow_mk_ref("expected value for flag --attack-level"));
    }

    parse_result_i64_t result = cli_parse_relative(state, UINT16_MAX);

    if (result.success) {
      params->attack_level = (uint16_t)result.value;
      return ParseSuccess(parse_result_void_t, result.state, 0);
    } else {
      return ParseFail(parse_result_void_t, ERROR_INVALID,
                       result.error_message);
    }
  }

  if (strcmp("fade-length", flag_name) == 0) {
    if (state_is_finished(state)) {
      return ParseFail(parse_result_void_t, ERROR_INVALID,
                       cow_mk_ref("expected value for flag --fade-length"));
    }

    parse_result_u16_t result = cli_parse_time(state);

    if (result.success) {
      params->fade_length = result.value;
      return ParseSuccess(parse_result_void_t, result.state, 0);
    } else {
      return ParseFail(parse_result_void_t, ERROR_INVALID,
                       result.error_message);
    }
  }

  if (strcmp("fade-level", flag_name) == 0) {
    if (state_is_finished(state)) {
      return ParseFail(parse_result_void_t, ERROR_INVALID,
                       cow_mk_ref("expected value for flag --fade-level"));
    }

    parse_result_i64_t result = cli_parse_relative(state, UINT16_MAX);

    if (result.success) {
      params->fade_level = (uint16_t)result.value;
      return ParseSuccess(parse_result_void_t, result.state, 0);
    } else {
      return ParseFail(parse_result_void_t, ERROR_INVALID,
                       result.error_message);
    }
  }

  MkErrStr1S(msg, "unrecognized flag `%s`", arg);
  return ParseFail(parse_result_void_t, ERROR_INVALID, cow_mk_owned(msg));
}

static parse_result_action_t cli_parse_action(parse_state_t state) {
  char *arg = state_advance(&state);

  if (strcasecmp("help", arg) == 0) {
    return ParseSuccess(parse_result_action_t, state, ACTION_HELP);
  }

  if (strcasecmp("ls", arg) == 0) {
    return ParseSuccess(parse_result_action_t, state, ACTION_LS);
  }

  if (strcasecmp("play", arg) == 0) {
    return ParseSuccess(parse_result_action_t, state, ACTION_PLAY);
  }

  MkErrStr1S(msg, "unrecognized action `%s`", arg);
  return ParseFail(parse_result_action_t, ERROR_UNRECOGNIZED,
                   cow_mk_owned(msg));
}

// cli_parse

cli_parse_result_t cli_parse(int argc, char *argv[]) {
  cli_params_t params = {0};
  parse_state_t state = ParseState(argc, argv, 1);

  while (!state_is_finished(state)) {
    // Is it a flag?
    parse_result_void_t flag_result = cli_parse_flag(state, &params);

    if (flag_result.success) {
      state = flag_result.state;
      continue;
    } else if (flag_result.error_code == ERROR_INVALID) {
      return (cli_parse_result_t){.success = false,
                                  .error_message = flag_result.error_message};
    }

    if (params.action == ACTION_NONE) {
      // First positional argument? Action
      parse_result_action_t action_result = cli_parse_action(state);

      if (action_result.success) {
        params.action = action_result.value;
        state = action_result.state;
        continue;
      }

      return (cli_parse_result_t){.success = false,
                                  .error_message = action_result.error_message};
    } else if (params.dev == NULL) {
      // Second positional argument? Device
      parse_result_str_t dev_result = cli_parse_str(state);

      if (dev_result.success) {
        params.dev = dev_result.value;
        state = dev_result.state;
        continue;
      }

      return (cli_parse_result_t){.success = false,
                                  .error_message = dev_result.error_message};
    }

    MkErrStr1S(msg, "unexpected argument `%s`", state_get_current_arg(state));
    return (cli_parse_result_t){.success = false, .error_message = msg};
  }

  return (cli_parse_result_t){
    .success = true,
    .params = params,
  };
}
