#include <stdint.h>
#include <stddef.h>
#include "imageProcessing.h"
#include "customHelper.h"
#include "staticKernels.h"
#include "dtekv-lib.h"

const int outputHeight = 240;
const int outputWidth = 320;

volatile char *VGA = (volatile char *)0x08000000;

// works with floating values, however not on the board
// #define M_EULER 2.718281828459045235360287471352
// #define M_PI 3.14159265358979323846
// int generateGaussianKernel(volatile int kernelRadie, int *filterMatrix)
// {
//   double sigma = 0.84089642;
//   int sum = 0;
//   int count = 0;
//   // Generate the kernel values
//   double multiplier = 1 / (pow(M_EULER, (-((kernelRadie * kernelRadie) * 2) / (2 * sigma * sigma))) / (2 * M_PI * sigma * sigma));
//   for (int y = -kernelRadie; y <= kernelRadie; y++)
//   {
//     for (int x = -kernelRadie; x <= kernelRadie; x++)
//     {
//       filterMatrix[count] = roundf(multiplier * pow(M_EULER, (-((x * x) + (y * y)) / (2 * sigma * sigma))) / (2 * M_PI * sigma * sigma));
//       sum += filterMatrix[count];
//       count++;
//     }
//   }
//   return sum;
// }

void set_leds(int value)
{
  volatile int *LED = (volatile int *)0x04000000;
  *LED = value & 0x3FF;
}

void generateSharpenKernel(volatile int kernelRadie, int *filterMatrix)
{
  int count = 0;
  if (kernelRadie == 0)
  {
    filterMatrix[0] = 1;
    return;
  }
  // Generate the kernel values
  for (int y = -kernelRadie; y <= kernelRadie; y++)
  {
    for (int x = -kernelRadie; x <= kernelRadie; x++)
    {
      if (x == 0 && y == 0)
      {
        filterMatrix[count] = 5;
      }
      else if ((x == 1 && y == 0) || (x == -1 && y == 0) || (x == 0 && y == 1) || (x == 0 && y == -1))
      {
        filterMatrix[count] = -1;
      }
      else
      {
        filterMatrix[count] = 0;
      }
      count++;
    }
  }
}

uint8_t blurringKernel(uint8_t arr[], int filterMatrix[], int total, int arrSize)
{
  int blueTotal = 0;
  int greenTotal = 0;
  int redTotal = 0;
  for (int i = 0; i < arrSize; i++)
  {
    blueTotal += filterMatrix[i] * ((arr[i] >> 0) & 3);
    greenTotal += filterMatrix[i] * ((arr[i] >> 2) & 7);
    redTotal += filterMatrix[i] * ((arr[i] >> 5) & 7);
  }
  blueTotal = (blueTotal + (total / 2)) / total;
  greenTotal = (greenTotal + (total / 2)) / total;
  redTotal = (redTotal + (total / 2)) / total;

  blueTotal = max(0, min(3, blueTotal));
  greenTotal = max(0, min(7, greenTotal));
  redTotal = max(0, min(7, redTotal));
  return blueTotal + (greenTotal << 2) + (redTotal << 5);
}

int *matrixGenerator(int blurType, int kernelSize, volatile int kernelRadie, int *sumPointer, int *filterMatrix)
{
  // Initialize the filterMatrix based on the blur type
  switch (blurType)
  {
  case 0: // BoxBlur
    for (int i = 0; i < kernelSize; i++)
    {
      filterMatrix[i] = 1;
    }
    break;

  case 1: // GaussianBlur
    switch (kernelRadie)
    {
    case 0:
      *sumPointer = staticKernel0(filterMatrix);
      break;
    case 1:
      *sumPointer = staticGaussianKernel1(filterMatrix);
      break;
    case 2:
      *sumPointer = staticGaussianKernel2(filterMatrix);
      break;
    case 3:
      *sumPointer = staticGaussianKernel3(filterMatrix);
      break;
    default:
      return 0;
    }
    break;

  case 2: // Sharpen
    generateSharpenKernel(kernelRadie, filterMatrix);
    break;

  case 3: // MotionBlur
    for (int i = 0; i < kernelRadie * 2 + 1; i++)
    {
      for (int j = 0; j < kernelRadie * 2 + 1; j++)
      {
        if (i == kernelRadie)
        {
          filterMatrix[i * (kernelRadie * 2 + 1) + j] = 1;
        }
        else
        {
          filterMatrix[i * (kernelRadie * 2 + 1) + j] = 0;
        }
      }
    }
    break;
  case 4: // EdgeDetection
    switch (kernelRadie)
    {
    case 0:
      *sumPointer = staticKernel0(filterMatrix);
      break;
    case 1:
      *sumPointer = staticEdgeDetectionKernel1(filterMatrix);
      break;
    case 2:
      *sumPointer = staticEdgeDetectionKernel2(filterMatrix);
      break;
    default:
      return 0;
    }
    break;

  default:
    return 0;
  }

  return filterMatrix;
}

void paddingKernel(volatile int kernelRadie, uint8_t *pixels, int x2, int y2, uint8_t *oldPixelData)
{
  int count = 0;
  for (int y = kernelRadie; y >= -kernelRadie; y--)
  {
    for (int x = -kernelRadie; x <= kernelRadie; x++)
    {
      if (x + x2 < outputWidth && y + y2 < outputHeight && x + x2 >= 0 && y + y2 >= 0)
      {
        pixels[count] = oldPixelData[x + x2 + ((y + y2) * outputWidth)];
      }
      else if (y + y2 < outputHeight && y + y2 >= 0)
      {
        pixels[count] = oldPixelData[(-1 * x) + x2 + ((y + y2) * outputWidth)];
      }
      else if (x + x2 < outputWidth && x + x2 >= 0)
      {
        pixels[count] = oldPixelData[x + x2 + (((-1 * y) + y2) * outputWidth)];
      }
      else
      {
        pixels[count] = oldPixelData[(-1 * x) + x2 + (((-1 * y) + y2) * outputWidth)];
      }
      count++;
    }
  }
}

int calculate_led_value(int currentRow, int totalRows)
{
  double progress = (double)currentRow / totalRows;
  int value = (int)(progress * 10);
  int ledBinary = (1 << value) - 1;
  return ledBinary;
}

void startBenchmarking()
{
  // unsigned int cycles;
  asm volatile("csrw mcycle, x0");
  asm volatile("csrw minstret, x0");
  asm volatile("csrw mhpmcounter3 , x0");
  asm volatile("csrw mhpmcounter4 , x0");
  asm volatile("csrw mhpmcounter5 , x0");
  asm volatile("csrw mhpmcounter6 , x0");
  asm volatile("csrw mhpmcounter7 , x0");
  asm volatile("csrw mhpmcounter8 , x0");
  asm volatile("csrw mhpmcounter9 , x0");
  asm volatile("csrw time , x0");
}

void bechmarkResults(int blurType, int kernelRadie)
{
  unsigned int cycles;
  unsigned int instret;
  unsigned int mhpmcounter3;
  unsigned int mhpmcounter4;
  unsigned int mhpmcounter5;
  unsigned int mhpmcounter6;
  unsigned int mhpmcounter7;
  unsigned int mhpmcounter8;
  unsigned int mhpmcounter9;

  asm volatile(
      "csrr %0, mcycle\n" // Read mcycle
      "csrr %1, minstret\n"
      "csrr %2, mhpmcounter3\n"
      "csrr %3, mhpmcounter4\n"
      "csrr %4, mhpmcounter5\n"
      "csrr %5, mhpmcounter6\n"
      "csrr %6, mhpmcounter7\n"
      "csrr %7, mhpmcounter8\n"
      "csrr %8, mhpmcounter9\n"

      : "=r"(cycles), "=r"(instret), "=r"(mhpmcounter3), "=r"(mhpmcounter4), "=r"(mhpmcounter5), "=r"(mhpmcounter6), "=r"(mhpmcounter7), "=r"(mhpmcounter8), "=r"(mhpmcounter9));
  if (blurType == 0)
  {
    print("\n Box blur");
  }
  else if (blurType == 1)
  {
    print("\n Gaussian blur");
  }
  else if (blurType == 2)
  {
    print("\n Sharpen");
  }
  else if (blurType == 3)
  {
    print("\n Motion blur");
  }
  else if (blurType == 4)
  {
    print("\n Edge detection");
  }

  print("\n KernelRadie:");
  print_dec(kernelRadie);
  print("\n Cycles:");
  print_dec(cycles);
  print("\n Instructions:");
  print_dec(instret);
  print("\n Memory instructions:");
  print_dec(mhpmcounter3);
  print("\n I-chache misses:");
  print_dec(mhpmcounter4);
  print("\n D-chache misses:");
  print_dec(mhpmcounter5);
  print("\n I-chache stalls:");
  print_dec(mhpmcounter6);
  print("\n D-chache stalls:");
  print_dec(mhpmcounter7);
  print("\n Data hazard stalls:");
  print_dec(mhpmcounter8);
  print("\n ALU operations:");
  print_dec(mhpmcounter9);
  print("\n");
}

void blurring(uint8_t *pixelData, int blurType, volatile int kernelRadie)
{
  startBenchmarking();
  int sum = 0;
  int kernelSize = (kernelRadie * 2 + 1) * (kernelRadie * 2 + 1);
  int filterMatrix[kernelSize];

  if (!matrixGenerator(blurType, kernelSize, kernelRadie, &sum, filterMatrix))
  {
    return;
  }
  for (int y = 0; y < outputHeight; y++)
  {
    for (int x = 0; x < outputWidth; x++)
    {
      int position = x + (y * outputWidth);

      uint8_t temp[kernelSize];
      if (x <= kernelRadie - 1 || y <= kernelRadie - 1 || x >= outputWidth - kernelRadie || y >= outputHeight - kernelRadie)
      {
        paddingKernel(kernelRadie, temp, x, y, pixelData); // Blurring the outer lines through mirrored padding
      }
      else
      {
        int count = 0;
        for (int i = -kernelRadie; i <= kernelRadie; i++)
        {
          for (int j = -kernelRadie; j <= kernelRadie; j++)
          {
            temp[count] = pixelData[position + j + (i * outputWidth)];
            count++;
          }
        }
      }

      position = x + ((outputHeight - y) * outputWidth);
      switch (blurType)
      {
      case 0: // Box
        VGA[position] = blurringKernel(temp, filterMatrix, kernelSize, kernelSize);
        break;

      case 1: // Gaussian
        VGA[position] = blurringKernel(temp, filterMatrix, sum, kernelSize);
        break;

      case 2: // Sharpen
        VGA[position] = blurringKernel(temp, filterMatrix, 1, kernelSize);
        break;

      case 3: // Motion
        VGA[position] = blurringKernel(temp, filterMatrix, kernelRadie * 2 + 1, kernelSize);
        break;

      case 4: // EdgeDetection
        VGA[position] = blurringKernel(temp, filterMatrix, sum, kernelSize);
        break;

      default:
        break;
      }
    }
    // Update LED progress display
    int ledValue = calculate_led_value(y, outputHeight);
    set_leds(ledValue);
  }
  set_leds(0b1111111111);
  bechmarkResults(blurType, kernelRadie);
}