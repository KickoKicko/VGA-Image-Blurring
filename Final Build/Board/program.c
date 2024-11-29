#include <stdint.h>
#include <stdlib.h>

#include "pixeldata.h"
#include "staticPixeldata.h"
#include "loadingPixelData.h"

#include "dtekv-lib.h"
#include "customHelper.h"
#include "imageProcessing.h"

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

int main(void)
{
  clearVGADisplay();
  updateVGADisplay(0, 0);
  labinit();
  while (1)
  {
    // make sure the program is running
  }
}
