#include <GoPTX/GoPTX.h>
#include <stdio.h>
int main(int argc, char **argv) {
  if (2 > argc || argc > 5) {
    fprintf(stderr,
            "usage: %s <first.ptx> <second.ptx> [suffix_first=\"\"] "
            "[suffix_second=\"\"]\n",
            argv[0]);
    return 0;
  }
  char *ptx_first;
  GoPTX_fread(&ptx_first, argv[1]);
  char *ptx_second;
  GoPTX_fread(&ptx_second, argv[2]);
  const char empty_string[] = "";
  const char *suffix_first = empty_string, *suffix_second = empty_string;
  if (argc >= 4)
    suffix_first = argv[3];
  if (argc >= 5)
    suffix_second = argv[4];
  char *ptx_concated;
  GoPTX_link(&ptx_concated, ptx_first, ptx_second, suffix_first,
                  suffix_second);
  fprintf(stdout, "%s", ptx_concated);
  GoPTX_free(ptx_first);
  GoPTX_free(ptx_second);
  GoPTX_free(ptx_concated);
  return 0;
}