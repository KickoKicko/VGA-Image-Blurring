#include <stddef.h>
#include "customHelper.h"

#define M_EULER 2.718281828459045235360287471352
#define M_PI 3.14159265358979323846

// Helper function to compute the absolute value
double absolute(double x)
{
  return x < 0 ? -x : x;
}

// Approximation of e^x using Taylor series
double custom_exp(double x)
{
  double term = 1.0; // First term of the series
  double sum = 1.0;  // Start with the first term
  for (int n = 1; n <= 20; ++n)
  { // Iterate for a fixed number of terms
    term *= x / n;
    sum += term;
  }
  return sum;
}

// Approximation of ln(x) using an iterative method (Newton's method)
double custom_ln(double x)
{
  if (x <= 0)
  {
    return -1; // Return an invalid value
  }

  double guess = x - 1.0; // Initial guess
  for (int i = 0; i < 20; ++i)
  { // Perform Newton's method iterations
    guess -= (custom_exp(guess) - x) / custom_exp(guess);
  }
  return guess;
}

// Power function x^y = e^(y * ln(x))
double custom_pow(double base, double exponent)
{
  // Handle edge cases
  if (base == 0)
  {
    if (exponent == 0)
    {
      // 0^0 is generally undefined, but often treated as 1
      return 1;
    }
    return 0; // 0 raised to any other power is 0
  }

  if (base < 0 && (int)exponent != exponent)
  {
    // Negative base with non-integer exponent is not supported in real numbers
    return -1; // Return an invalid value
  }

  // Compute x^y = e^(y * ln(x))
  return custom_exp(exponent * custom_ln(base));
}

// Custom implementation of fmin
int my_min(int a, int b)
{
  return (a < b) ? a : b;
}

// Custom implementation of fmax
int my_max(int a, int b)
{
  return (a > b) ? a : b;
}

double custom_round(double x)
{
  // Separate positive and negative cases
  if (x >= 0)
  {
    // Add 0.5 and cast to an integer to round up
    return (double)((long long)(x + 0.5));
  }
  else
  {
    // Subtract 0.5 and cast to an integer to round down
    return (double)((long long)(x - 0.5));
  }
}

void *memset(void *ptr, int value, size_t num)
{
  unsigned char *p = ptr; // Cast the pointer to an unsigned char pointer for byte-wise operations
  for (size_t i = 0; i < num; i++)
  {
    p[i] = (unsigned char)value; // Set each byte to the specified value
  }
  return ptr; // Return the original pointer for convenience, similar to the standard memset
}