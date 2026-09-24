#include "play.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

ssize_t mk_periodic_effect(cli_params_t params, struct ff_effect *effect) {
  assert(params.effect == FF_PERIODIC);

  if (params.waveform == 0) {
    fprintf(stderr, "waveform mandatory for periodic effect\n");
    return -1;
  }

  if (params.period == 0) {
    fprintf(stderr, "period mandatory for periodic effect\n");
    return -1;
  }

  if (params.magnitude == 0) {
    fprintf(stderr, "magnitude mandatory for periodic effect\n");
    return -1;
  }

  *effect = (struct ff_effect){
    .type = FF_PERIODIC,
    .id = -1,
    .direction = params.direction,
    .trigger =
      {
        .button = 0,
        .interval = 0,
      },
    .replay =
      {
        .delay = params.delay,
        .length = params.duration,
      },
    .u =
      {
        .periodic =
          {
            .waveform = params.waveform,
            .period = params.period,
            .magnitude = params.magnitude,
            .offset = params.offset,
            .phase = params.phase,
            .envelope =
              {
                .attack_length = params.attack_length,
                .attack_level = params.attack_level,
                .fade_length = params.fade_length,
                .fade_level = params.fade_level,
              },
          },
      },
  };

  return 0;
}

int fofee_play(cli_params_t params) {
  if (params.dev == NULL) {
    fprintf(stderr, "missing device with which to play\n");
    return 1;
  }

  if (params.effect == 0) {
    fprintf(stderr, "missing effect type\n");
    return 1;
  }

  if (params.duration == 0) {
    fprintf(stderr, "missing duration\n");
    return 1;
  }

  int fd = open(params.dev, O_RDWR);

  if (fd < 0) {
    fprintf(stderr, "error reading device '%s': %s\n", params.dev,
            strerror(errno));
    return 1;
  }

  ssize_t result;

  // TODO: Implement others
  assert(params.effect == FF_PERIODIC);

  struct ff_effect effect = {0};
  result = mk_periodic_effect(params, &effect);

  if (result < 0) {
    return 1;
  }

  struct input_event play;
  struct input_event stop;

  result = ioctl(fd, EVIOCSFF, &effect);

  if (result < 0) {
    fprintf(stderr, "error uploading effect\n");
    return 1;
  }

  play.type = EV_FF;
  play.code = effect.id;
  play.value = 1;

  result = write(fd, (const void *)&play, sizeof(play));

  if (result < 0) {
    fprintf(stderr, "error playing effect\n");
    return 1;
  }

  // Keep the process and the file descriptors alive for the duration of the
  // effect.
  usleep(params.delay * 1000 + params.duration * 1000);

  stop.type = EV_FF;
  stop.code = effect.id;
  stop.value = 0;

  result = write(fd, (const void *)&stop, sizeof(stop));

  if (result < 0) {
    fprintf(stderr, "error stopping effect\n");
    return 1;
  }

  return 0;
}
