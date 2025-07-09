#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum GoPTX_strategy {
  GoPTX_strategy_default = 0,
  GoPTX_strategy_vfuse = 1,
  GoPTX_strategy_hfuse = 2,
  GoPTX_strategy_GoPTX_latency = 3,
  GoPTX_strategy_GoPTX_concat = 4
} GoPTX_strategy;

typedef struct GoPTX_config {
  GoPTX_strategy strategy;
  int no_avoid_deadlock;
  int no_avoid_aligned;
} GoPTX_config;

void GoPTX_free(void *ptr);

void GoPTX_strdup(char **ptr, const char *s);

void GoPTX_fread(char **ptr, const char *file_name);

void GoPTX_link(char **ptr, const char *ptx_first, const char *ptx_second,
                const char *suffix_first, const char *suffix_second);

void GoPTX_merge(char **ptr, const char *ptx, const char *regex_merged,
                 const char *regex_first, const char *regex_second,
                 GoPTX_config config);

#ifdef __cplusplus
}
#endif