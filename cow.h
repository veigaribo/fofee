#ifndef _FOFEE_COW
#define _FOFEE_COW

#include <stdbool.h>

// Contains a string that is either owned (needs to be freed) or just a
// reference (does not need to be freed by you). `cow_free` handles both cases.

typedef struct {
  char *str;
  bool is_owned;
} cow_t;

cow_t cow_mk_owned(char *str);
cow_t cow_mk_ref(char *str);

void cow_free(cow_t cow);

#endif // !_FOFEE_COW
