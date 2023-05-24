#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <errno.h>

#include "min.h"

typedef uint64_t u64;
typedef uint8_t u8;

#define WARN_ON(cond) if (cond) { fprintf(stdout, "ERROR: " # cond "\n"); exit(1); }
#define pr_trace(...)

#include "kq.h"

void kq_print(struct kq *reg, size_t sz)
{
  u8 *buf = malloc(sz);
  WARN_ON(buf == NULL);
  kq_read(reg, buf, sz);
  printf("read %lu: %.*s\n", sz, (int)sz, buf);
  free(buf);
}

void kq_writes(struct kq *reg, char *str)
{
  int ret = kq_write(reg, (u8 *)str, strlen(str));
  WARN_ON(ret < 0);
}

int main() {
  u8 buf[10] = {};
  struct kq q = {};
  q.log_size = 4;
  WARN_ON(kq_write(&q, (u8*)"abcd", 4) < 0);
  WARN_ON(kq_read(&q, buf, sizeof(buf)) < 0);
  printf("read %s\n", buf);
  WARN_ON(kq_write(&q, (u8*)"0123456789AB", 4) < 0);
  kq_print(&q, 3);
  kq_print(&q, 1);
  kq_writes(&q, "abcdefghik");
  kq_print(&q, 10);
}
