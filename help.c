#include "help.h"
#include <stdio.h>

const char *TIME_ARG_DESCRIPTION = "Accepts `ms`, `s`, and `min`";
const char *DIRECTION_ARG_DESCRIPTION =
  "Accepts the values `up`, `down`, `left`, `right`, and values in degrees "
  "`deg` and in radians `rad`, in which case 0 is right and positive values "
  "wind counterclockwise";
const char *RELATIVE_ARG_DESCRIPTION =
  "Accepts `%`, `a/b` fractions and naked scalars, in which case `1.0` = "
  "`100%`. May be negative";

void fofee_help(int argc, char *argv[]) {
  char *p0 = argc > 0 ? argv[0] : "fofee";

  printf("Usage: %s [FLAGS] [ACTION] [DEVICE]\n", p0);
  printf("\n");
  printf("Action must be one of: `help`, `ls`, `play`.\n");
  printf("    `help`: Shows this message.\n");
  printf(
    "    `ls`: Scans and lists devices supporting force feedback features.\n");
  printf("    `play`: Plays a force feedback effect on a device. WARNING: Your "
         "device will move on its own.\n");
  printf("\n");
  printf(
    "Device must be a device pseudo-file, typically located on `/dev/input/`. "
    "Run `%s ls` for options. Only required for the `play` action.\n",
    p0);
  printf("\n");
  printf("Flags per action:\n");
  printf("\n");
  printf("`help`\n");
  printf("No flags recognized.\n");
  printf("\n");
  printf("`ls`\n");
  printf("No flags recognized.\n");
  printf("\n");
  printf("`play`\n");
  printf(
    "Currently, only periodic effects are properly supported (if you can call it that).\n\n");
  printf(
    "    `--effect {EFFECT}`: Specifies the effect type. Must be one of "
    "HAPTIC, "
    "RUMBLE, PERIODIC, CONSTANT, SPRING, FRICTION, DAMPER, INERTIA or RAMP. "
    "Run `%s ls` for a list of effect types your device claims to support.\n",
    p0);
  printf("    `--delay {DURATION}`: Specifies an amount of time that must "
         "elapse before the "
         "effect starts. %s.\n",
         TIME_ARG_DESCRIPTION);
  printf(
    "    `--duration {DURATION}`: Specifies the duration of the effect. %s.\n",
    TIME_ARG_DESCRIPTION);
  printf("    `--direction {DIRECTION}`: Specifies a direction for the effect. "
         "%s.\n",
         DIRECTION_ARG_DESCRIPTION);
  printf(
    "    `--waveform {WAVEFORM}`: Mandatory for periodic effects, this "
    "describes the "
    "overall shape of the waves. Must be one of SQUARE, TRIANGLE, SINE, "
    "SAW_UP, SAW_DOWN or CUSTOM, although CUSTOM is not properly supported. "
    "Run `%s ls` for a list of waveforms your device claims to support.\n",
    p0);
  printf("    `--period {DURATION}`: Mandatory for periodic effects, specifies "
         "the period "
         "of the effect. %s.\n",
         TIME_ARG_DESCRIPTION);
  printf(
    "    `--magnitude {VALUE}`: Mandatory for periodic effects, specifies the "
    "magnitude "
    "of the effect. %s.\n",
    RELATIVE_ARG_DESCRIPTION);
  printf("    `--attack-length {DURATION}`: See "
         "<https://docs.kernel.org/_images/shape.svg>. %s.\n",
         TIME_ARG_DESCRIPTION);
  printf("    `--attack-level {VALUE}`: See "
         "<https://docs.kernel.org/_images/shape.svg>. "
         "%s.\n",
         RELATIVE_ARG_DESCRIPTION);
  printf("    `--fade-length {DURATION}`: See "
         "<https://docs.kernel.org/_images/shape.svg>. %s.\n",
         TIME_ARG_DESCRIPTION);
  printf("    `--fade-level {VALUE}`: See "
         "<https://docs.kernel.org/_images/shape.svg>. "
         "%s.\n",
         RELATIVE_ARG_DESCRIPTION);
  printf("    `--offset {VALUE}`: For periodic effects, this represents a "
         "vertical shift in "
         "the wave pattern of the effect. %s.\n",
         RELATIVE_ARG_DESCRIPTION);
  printf(
    "    `--phase {VALUE}`: For periodic effects, specifies a horizontal shift "
    "in the wave pattern of the effect. Value is relative to its period. %s.\n",
    RELATIVE_ARG_DESCRIPTION);
  printf("\n");

  printf(
    "`fofee` is a tool to play force feedback effects, such as rumbling, "
    "in supported devices. It definitely hasn't been tested well enough.\n");
  printf(
    "Make sure to keep hold of your device to prevent possible damages.\n");
}
