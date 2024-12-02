#include <stdint.h>
#include <stddef.h>
#include "imageProcessing.h"
#include "customHelper.h"

#define M_EULER 2.718281828459045235360287471352
#define M_PI 3.14159265358979323846

int outputHeight = 240;
int outputWidth = 320;

int generateGaussianKernel(volatile int kernelRadie, int *filterMatrix)
{
  double sigma = 0.84089642;
  int sum = 0;
  int count = 0;
  // Generate the kernel values
  double multiplier = 1 / (custom_pow(M_EULER, (-((kernelRadie * kernelRadie) * 2) / (2 * sigma * sigma))) / (2 * M_PI * sigma * sigma));
  for (int y = -kernelRadie; y <= kernelRadie; y++)
  {
    for (int x = -kernelRadie; x <= kernelRadie; x++)
    {
      filterMatrix[count] = custom_round(multiplier * custom_pow(M_EULER, (-((x * x) + (y * y)) / (2 * sigma * sigma))) / (2 * M_PI * sigma * sigma));
      sum += filterMatrix[count];
      count++;
    }
  }
  return sum;
}

int fakeGenerateGaussianKernel1(int *filterMatrix)
{
  filterMatrix[0] = 1;
  filterMatrix[1] = 2;
  filterMatrix[2] = 1;
  filterMatrix[3] = 2;
  filterMatrix[4] = 4;
  filterMatrix[5] = 2;
  filterMatrix[6] = 1;
  filterMatrix[7] = 2;
  filterMatrix[8] = 1;
  return 16;
}

int fakeGenerateGaussianKernel2(int *filterMatrix)
{
  filterMatrix[0] = 1;
  filterMatrix[1] = 8;
  filterMatrix[2] = 17;
  filterMatrix[3] = 8;
  filterMatrix[4] = 1;
  filterMatrix[5] = 8;
  filterMatrix[6] = 70;
  filterMatrix[7] = 141;
  filterMatrix[8] = 70;
  filterMatrix[9] = 8;
  filterMatrix[10] = 17;
  filterMatrix[11] = 141;
  filterMatrix[12] = 286;
  filterMatrix[13] = 141;
  filterMatrix[14] = 17;
  filterMatrix[15] = 8;
  filterMatrix[16] = 70;
  filterMatrix[17] = 141;
  filterMatrix[18] = 70;
  filterMatrix[19] = 8;
  filterMatrix[20] = 1;
  filterMatrix[21] = 8;
  filterMatrix[22] = 17;
  filterMatrix[23] = 8;
  filterMatrix[24] = 1;
  return 1266;
}

int fakeGenerateGaussianKernel3(int *filterMatrix)
{
  filterMatrix[0] = 1;
  filterMatrix[1] = 34;
  filterMatrix[2] = 286;
  filterMatrix[3] = 581;
  filterMatrix[4] = 286;
  filterMatrix[5] = 34;
  filterMatrix[6] = 1;
  filterMatrix[7] = 34;
  filterMatrix[8] = 1177;
  filterMatrix[9] = 9822;
  filterMatrix[10] = 19920;
  filterMatrix[11] = 9822;
  filterMatrix[12] = 1177;
  filterMatrix[13] = 34;
  filterMatrix[14] = 286;
  filterMatrix[15] = 9822;
  filterMatrix[16] = 81937;
  filterMatrix[17] = 166178;
  filterMatrix[18] = 81937;
  filterMatrix[19] = 9822;
  filterMatrix[20] = 286;
  filterMatrix[21] = 581;
  filterMatrix[22] = 19920;
  filterMatrix[23] = 166178;
  filterMatrix[24] = 337028;
  filterMatrix[25] = 166178;
  filterMatrix[26] = 19920;
  filterMatrix[27] = 581;
  filterMatrix[28] = 286;
  filterMatrix[29] = 9822;
  filterMatrix[30] = 81937;
  filterMatrix[31] = 166178;
  filterMatrix[32] = 81937;
  filterMatrix[33] = 9822;
  filterMatrix[34] = 286;
  filterMatrix[35] = 34;
  filterMatrix[36] = 1177;
  filterMatrix[37] = 9822;
  filterMatrix[38] = 19920;
  filterMatrix[39] = 9822;
  filterMatrix[40] = 1177;
  filterMatrix[41] = 34;
  filterMatrix[42] = 1;
  filterMatrix[43] = 34;
  filterMatrix[44] = 286;
  filterMatrix[45] = 581;
  filterMatrix[46] = 286;
  filterMatrix[47] = 34;
  filterMatrix[48] = 1;
  return 1497340;
}

void generateSharpenKernel(volatile int kernelRadie, int *filterMatrix)
{
  int count = 0;
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

  blueTotal = my_max(0, my_min(3, blueTotal));
  greenTotal = my_max(0, my_min(7, greenTotal));
  redTotal = my_max(0, my_min(7, redTotal));
  return blueTotal + (greenTotal << 2) + (redTotal << 5);
}

int *matrixGenerator(int blurType, int kernelSize, volatile int kernelRadie, int *sumPointer, int *filterMatrix)
{
  // Initialize the filterMatrix based on the blur type
  if (blurType == 0) // BoxBlur
  {
    for (int i = 0; i < kernelSize; i++)
    {
      filterMatrix[i] = 1;
    }
  }
  else if (blurType == 1) // GaussianBlur
  {
    if (kernelRadie == 1)
    { // All of these three functions return hardcoded gaussian matrices since the board can not use float values which is needed for the gaussian calculations.
      *sumPointer = fakeGenerateGaussianKernel1(filterMatrix);
    }
    else if (kernelRadie == 2)
    {
      *sumPointer = fakeGenerateGaussianKernel2(filterMatrix);
    }
    else if (kernelRadie == 3)
    {
      *sumPointer = fakeGenerateGaussianKernel3(filterMatrix);
    }
    else
    {
      return 0;
    }
  }
  else if (blurType == 2) // Sharpen
  {
    generateSharpenKernel(kernelRadie, filterMatrix);
  }
  else if (blurType == 3) // MotionBlur
  {
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

void blurring(uint8_t *pixelData, int blurType, volatile int kernelRadie)
{
  if (kernelRadie == 0)
  {
    return;
  }

  // Statically allocate memory for the temporary pixel data
  uint8_t tempPixelData[240 * 320];
  int kernelSize = (kernelRadie * 2 + 1) * (kernelRadie * 2 + 1);
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
      int sum = 0;
      int filterMatrix[kernelSize];
      if (!matrixGenerator(blurType, kernelSize, kernelRadie, &sum, filterMatrix))
      {
        return;
      }
      if (blurType == 0)
      { // Box
        tempPixelData[position] = blurringKernel(temp, filterMatrix, kernelSize, kernelSize);
      }
      else if (blurType == 1)
      { // Gaussian
        tempPixelData[position] = blurringKernel(temp, filterMatrix, sum, kernelSize);
      }
      else if (blurType == 2)
      { // Sharpen
        tempPixelData[position] = blurringKernel(temp, filterMatrix, 1, kernelSize);
      }
      else if (blurType == 3)
      { // Motion
        tempPixelData[position] = blurringKernel(temp, filterMatrix, kernelRadie * 2 + 1, kernelSize);
      }
    }
  }

  // ändring 3
  for (int i = 0; i < 76800; i += 2)
  {
    pixelData[i] = tempPixelData[i];
    pixelData[i + 1] = tempPixelData[i + 1]; // to trick the compiler into not using memcpy
  }
}