#include "cow.h"
#include <stdlib.h>

cow_t cow_mk_owned(char *str) { return (cow_t){.is_owned = true, .str = str}; }
cow_t cow_mk_ref(char *str) { return (cow_t){.is_owned = false, .str = str}; }

void cow_free(cow_t cow) {
  if (cow.is_owned) {
    free(cow.str);
  }
}
