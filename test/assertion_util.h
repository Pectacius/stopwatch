// Assertion utilities for unit testing

#ifndef LIBSTOPWATCH_TEST_ASSERTION_UTIL_H_
#define LIBSTOPWATCH_TEST_ASSERTION_UTIL_H_

#include <math.h>

double relative_error(long long val1, long long val2) {
  return fabs((double) (val1 - val2) / (double) val1);
}

#endif //LIBSTOPWATCH_TEST_ASSERTION_UTIL_H_
