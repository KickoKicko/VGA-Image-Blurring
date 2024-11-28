#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include "pixeldata.h"
#include "staticPixeldata.h"
#include "loadingPixelData.h"
#include "dtekv-lib.h"

extern void display_string(char *);
extern void enable_interrupt(void);

int *edgecaptureSwitch = (int *)0x0400001C;
int *switchData = (int *)0x04000010;
int *edgecaptureButton = (int *)0x040000dC;

int btn_counter = 1;

int loadingBool = 0;

void delay(unsigned int ms)
{
  volatile unsigned int i, j;
  for (i = 0; i < ms; i++)
  {
    // Adjust the loop counts as necessary for your processor speed
    for (j = 0; j < 1300; j++)
    {
      // Empty loop to create the delay
    }
  }
}

/* Below is the function that will be called when an interrupt is triggered. */
void handle_interrupt(unsigned cause)
{
  switch (cause)
  {
  case 17:
    break;
  case 18:
    btn_counter++;
    *edgecaptureButton = 0b1;

    if (btn_counter % 2 == 0 && loadingBool == 0)
    {
      int kernel = (*switchData >> 8) & 0b11;
      int kernelSize = *switchData & 0b111;
      initiatePicture(kernel, kernelSize);
      print_dec(kernel);
      print_dec(kernelSize);
    }
    break;
  default:
    display_string("what??");
    break;
  }
}

/* Add your code here for initializing interrupts. */
void labinit(void)
{
  int *interruptmaskSwitch = (int *)0x04000018;
  *interruptmaskSwitch = 0b1111111111;
  int *interruptmaskButton = (int *)0x040000d8;
  *interruptmaskButton = 0b1111111111;
  enable_interrupt();
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

// några ändringar har gjorts i denna kod:
// 1. kernelRadie är nu av typ volatile int
// 2. lagt till en early return när kernelradie = 0
// 3. ändrat sista kopieringen från temppixeldata till pixeldata så att vi inte behöver memcpy

//                                               ändring 1
void blurring(uint8_t *pixelData, int blurType, volatile int kernelRadie)
{
  // ändring 2
  if (kernelRadie == 0)
  {
    return 0;
  }

  int outputHeight = 240;
  int outputWidth = 320;

  // Statically allocate memory for the temporary pixel data
  uint8_t tempPixelData[240 * 320];

  for (int y = 0; y < outputHeight; y++)
  {
    for (int x = 0; x < outputWidth; x++)
    {
      int position = x + (y * outputWidth);
      if (x <= kernelRadie - 1 || y <= kernelRadie - 1 || x >= outputWidth - kernelRadie || y >= outputHeight - kernelRadie)
      {
        tempPixelData[position] = pixelData[position];
      }
      else
      {
        int kernelSize = (kernelRadie * 2 + 1) * (kernelRadie * 2 + 1);
        uint8_t temp[kernelSize];
        int count = 0;
        for (int i = -kernelRadie; i <= kernelRadie; i++)
        {
          for (int j = -kernelRadie; j <= kernelRadie; j++)
          {
            temp[count] = pixelData[position + j + (i * outputWidth)];
            count++;
          }
        }

        if (blurType == 0)
        {
          int filterMatrix[kernelSize];
          for (size_t i = 0; i < kernelSize; i++)
          {
            filterMatrix[i] = 1;
          }
          tempPixelData[position] = blurringKernel(temp, filterMatrix, kernelSize, kernelSize);
        }
        else if (blurType == 1)
        { // Gaussian
          if (kernelRadie == 1)
          {
            int filterMatrix[] = {1, 2, 1, 2, 4, 2, 1, 2, 1};
            tempPixelData[position] = blurringKernel(temp, filterMatrix, 16, kernelSize);
          }
          if (kernelRadie == 2)
          {
            int filterMatrix[] = {1, 4, 6, 4, 1, 4, 16, 24, 16, 4, 6, 24, 36, 24, 6, 4, 16, 24, 16, 4, 1, 4, 6, 4, 1};
            tempPixelData[position] = blurringKernel(temp, filterMatrix, 256, kernelSize);
          }
        }
        else if (blurType == 2)
        { // Sharpen
          if (kernelRadie == 1)
          {
            int filterMatrix[] = {0, -1, 0, -1, 5, -1, 0, -1, 0};
            tempPixelData[position] = blurringKernel(temp, filterMatrix, 1, kernelSize);
          }
          if (kernelRadie == 2)
          {
            int filterMatrix[] = {0, -1, -1, -1, 0, -1, -1, -1, -1, -1, -1, -1, 20, -1, -1, -1, -1, -1, -1, -1, 0, -1, -1, -1, 0};
            tempPixelData[position] = blurringKernel(temp, filterMatrix, 1, kernelSize);
          }
        }
        else if (blurType == 3)
        { // Motion
          int filterMatrix[kernelSize];
          for (int i = 0; i < kernelRadie * 2 + 1; i++)
          {
            for (size_t j = 0; j < kernelRadie * 2 + 1; j++)
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
          tempPixelData[position] = blurringKernel(temp, filterMatrix, (kernelRadie * 2 + 1), kernelSize);
        }
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
void updateVGADisplay(int kernelType, int kernelRadie)
{
  volatile char *VGA = (volatile char *)0x08000000;
  blurring(output_bmp, kernelType, kernelRadie);
  for (int y = 0; y < 240; y++)
  {
    for (int x = 0; x < 320; x++)
    {
      // Calculate source index in the BMP array
      int srcIndex = ((239 - y) * 320) + x; // 54 for alignment

      // Calculate destination index in the VGA buffer
      int dstIndex = (y * 320) + x;

      // Copy pixel data from BMP to VGA
      VGA[dstIndex] = output_bmp[srcIndex];
    }
  }
}

void resetPixelData(void)
{
  for (int i = 0; i < 76800; i += 2)
  {
    output_bmp[i] = static_output_bmp[i];
    output_bmp[i + 1] = static_output_bmp[i + 1];
  }
}

int get_sw(void)
{
  volatile int *sw = (volatile int *)0x04000010;
  int value = *sw & 0x3FF;
  return value;
}

void clearVGADisplay()
{
  volatile char *VGA = (volatile char *)0x08000000;
  for (int i = 0; i < 320 * 240; i++)
  {
    VGA[i] = 0x00;
  }
}

void displayLoading()
{
  loadingBool = 1;
  volatile char *VGA = (volatile char *)0x08000000;
  for (int y = 100; y < 130; y++)
  {
    for (int x = 100; x < 200; x++)
    {
      // Calculate source index in the BMP array
      int srcIndex = ((239 - y) * 320) + x + 54; // 54 for alignment

      // Calculate destination index in the VGA buffer
      int dstIndex = (y * 320) + x;

      // Copy pixel data from BMP to VGA
      VGA[dstIndex] = LoadingOutput_bmp[srcIndex];
    }
  }
  loadingBool = 0;
}

void initiatePicture(int kernel, int kernelSize)
{
  displayLoading();
  resetPixelData();
  updateVGADisplay(kernel, kernelSize);
}

/* Your code goes into main as well as any needed functions. */
int main(void)
{
  clearVGADisplay();
  updateVGADisplay(0, 0);
  labinit();
  while (1)
  {
    // print_dec(get_sw());
  }
  // displayLoading();
  // updateVGADisplay(3, 1);
  // delay(3000);
  // displayLoading();
  // updateVGADisplay(3, 2);
  // delay(3000);
  // displayLoading();
  // updateVGADisplay(3, 3);
  // delay(3000);
  // displayLoading();
  // updateVGADisplay(0, 1);
  // delay(3000);
  // displayLoading();
  // updateVGADisplay(0, 2);
  // delay(3000);
  // displayLoading();
  // updateVGADisplay(0, 3);
}
