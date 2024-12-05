#include <stddef.h>
#include "customHelper.h"

// Custom implementation of fmin
int min(int a, int b)
{
  return (a < b) ? a : b;
}

// Custom implementation of fmax
int max(int a, int b)
{
  return (a > b) ? a : b;
}