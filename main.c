#include "cli.h"
#include "help.h"
#include "ls.h"
#include "play.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
  cli_parse_result_t cli_result = cli_parse(argc, argv);
  cli_params_t params;

  if (cli_result.success) {
    params = cli_result.params;
  } else {
    fprintf(stderr, "error parsing command-line arguments: %s\n",
            cli_result.error_message.str);

    // Just to appease ASAN, Valgrind and friends
    cow_free(cli_result.error_message);
    return 1;
  }

  switch (params.action) {
  case ACTION_LS:
    fofee_ls();
    return 0;
  case ACTION_PLAY:
    return fofee_play(params);
  default:
    fofee_help(argc, argv);
    return 0;
  }
}
