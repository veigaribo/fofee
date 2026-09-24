#include "common.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#ifdef UDEV
#include <systemd/sd-device.h>
#endif // UDEV

#define X(Arg) #Arg,
const char *FEATURE_NAMES[] = {X_EFFECTS};
const char *WAVEFORM_NAMES[] = {X_WAVEFORMS};

const char *DEFAULT_DEVICE_HUMAN_READABLE_NAME = "NOT FOUND";

static void print_device_human_readable(const char *dev_path) {
#ifdef UDEV
  int result;

  sd_device *dev;
  result = sd_device_new_from_devname(&dev, dev_path);

  if (result < 0) {
    printf("%s", DEFAULT_DEVICE_HUMAN_READABLE_NAME);
  }

  const char *value;
  sd_device_get_property_value(dev, "ID_VENDOR", &value);
  printf("  Vendor: %s\n", value);
  sd_device_get_property_value(dev, "ID_MODEL", &value);
  printf("  Model: %s\n", value);

  sd_device_unref(dev);
#else
  // Silent
#endif // UDEV
}

void fofee_ls() {
  uint8_t features[BITS_TO_BYTES(FF_CNT)] = {0};
  int result;

  const char *const dev_path_prefix = "/dev/input/event";
  size_t dev_path_max_len = strlen(dev_path_prefix) + 4 + 1;
  char dev_path_buf[dev_path_max_len];

  for (size_t dev_i = 0; dev_i <= 9999; ++dev_i) {
    snprintf(dev_path_buf, dev_path_max_len, "%s%zu", dev_path_prefix, dev_i);
    int dev_fd = open(dev_path_buf, O_RDONLY);

    if (dev_fd < 0) {
      if (errno == ENOENT) {
        break;
      }

      if (errno != EACCES) {
        fprintf(stderr, "error reading device '%s': %s\n", dev_path_buf,
                strerror(errno));
      }

      continue;
    }

    result = ioctl(dev_fd, EVIOCGBIT(EV_FF, sizeof features), features);
    close(dev_fd);

    if (result < 0) {
      if (errno == EINVAL) {
        continue;
      }

      fprintf(stderr, "error querying device '%s': %s\n", dev_path_buf,
              strerror(errno));
      continue;
    }

    printf("%s\n", dev_path_buf);

    print_device_human_readable(dev_path_buf);

    printf("  Features: ");
    bool is_first = true;

    for (size_t feature = FF_EFFECT_MIN; feature <= FF_EFFECT_MAX; ++feature) {
      size_t offset = feature - FF_EFFECT_MIN;

      if (is_bit_set(features, feature)) {
        const char *name = FEATURE_NAMES[offset];

        if (is_first) {
          printf("%s", name);
          is_first = false;
        } else {
          printf(", %s", name);
        }
      }
    }

    if (!is_bit_set(features, FF_PERIODIC)) {
      printf("\n");
      continue;
    }

    printf("\n  Periodic: ");
    is_first = true;

    for (size_t wf = FF_WAVEFORM_MIN; wf <= FF_WAVEFORM_MAX; ++wf) {
      size_t offset = wf - FF_WAVEFORM_MIN;

      if (is_bit_set(features, wf)) {
        const char *name = WAVEFORM_NAMES[offset];

        if (is_first) {
          printf("%s", name);
          is_first = false;
        } else {
          printf(", %s", name);
        }
      }
    }

    printf("\n");
  }
}
