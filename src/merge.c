#include <GoPTX/GoPTX.h>
#include <stdio.h>
#include <stdlib.h>
int main(int argc, char **argv) {
  if (4 > argc || argc > 8) {
    fprintf(stderr,
            "usage: %s <your.ptx> <regex_merged> <regex_first> <regex_second> "
            "[strategy=0]\n",
            argv[0]);
    return 0;
  }
  char *ptx;
  GoPTX_fread(&ptx, argv[1]);
  const char *regex_merged = argv[2];
  const char *regex_first = argv[3];
  const char *regex_second = argv[4];
  GoPTX_config config;
  config.strategy = GoPTX_strategy_default;
  config.no_avoid_aligned = 0;
  config.no_avoid_deadlock = 0;
  if (argc >= 6)
    config.strategy = atoi(argv[5]);
  char *ptx_merged;
  GoPTX_merge(&ptx_merged, ptx, regex_merged, regex_first, regex_second,
              config);
  fprintf(stdout, "%s", ptx_merged);
  GoPTX_free(ptx);
  GoPTX_free(ptx_merged);
  return 0;
}