#pragma once

#include <stdio.h>
#include <stdlib.h>

#include "macro.h"

static inline void free_char(char *str) {
  if (str != NULL) {
    free(str);
  }
}

DEFINE_TRIVIAL_CLEANUP_FUNC(char *, free_char)
#define SCOPED_PTR_char __attribute__((cleanup(free_charp))) char *

#define ALLOCATION_FAILURE_MSG "Failed to allocate.\n"

///
/// Allocate or die.
///
/// @param size The size in bytes to allocate.
/// @return A pointer to the allocated memory.
///
static inline void *emalloc(size_t size) {
  void *ret = malloc(size);
  if (ret == NULL) {
    fprintf(stderr, ALLOCATION_FAILURE_MSG);
    exit(EXIT_FAILURE);
  }
  return ret;
}

///
/// Allocate and zero or die.
///
/// @param nmemb The number of elements to allocate.
/// @param size The size in bytes of each element.
/// @return A pointer to the allocated memory.
///
static inline void *ecalloc(size_t nmemb, size_t size) {
  void *ret = calloc(nmemb, size);
  if (ret == NULL) {
    fprintf(stderr, ALLOCATION_FAILURE_MSG);
    exit(EXIT_FAILURE);
  }
  return ret;
}
