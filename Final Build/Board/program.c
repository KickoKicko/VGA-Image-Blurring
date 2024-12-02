#include <stdint.h>
#include <stdlib.h>

#include "pixeldata.h"
#include "loadingPixelData.h"

#include "dtekv-lib.h"
#include "imageProcessing.h"

extern void display_string(char *);
extern void enable_interrupt(void);

extern const int outputHeight;
extern const int outputWidth;

volatile char *VGA = (volatile char *)0x08000000;
int *edgecaptureSwitch = (int *)0x0400001C;
int *switchData = (int *)0x04000010;
int *edgecaptureButton = (int *)0x040000dC;

int btn_counter = 1;

int loadingBool = 0;

unsigned char staticPixelData[240 * 320];

void init(void)
{
  int *interruptmaskSwitch = (int *)0x04000018;
  *interruptmaskSwitch = 0b1111111111;
  int *interruptmaskButton = (int *)0x040000d8;
  *interruptmaskButton = 0b1111111111;
  enable_interrupt();
}

void createStaticPixelData(unsigned char *pixelData)
{
  for (int i = 0; i < outputHeight * outputWidth; i += 2)
  {
    staticPixelData[i] = pixelData[i];
    staticPixelData[i + 1] = pixelData[i + 1]; // to trick the compiler into not using memcpy
  }
}

void updateVGADisplay(unsigned char *arr, int startX, int endX, int startY, int endY)
{
  for (int y = startY; y < endY; y++)
  {
    for (int x = startX; x < endX; x++)
    {
      int srcIndex = ((outputHeight - 1 - y) * outputWidth) + x;

      int dstIndex = (y * outputWidth) + x;

      VGA[dstIndex] = arr[srcIndex];
    }
  }
}

void resetPixelData(void)
{
  for (int i = 0; i < 76800; i += 2)
  {
    output_bmp[i] = staticPixelData[i];
    output_bmp[i + 1] = staticPixelData[i + 1];
  }
}

void clearVGADisplay()
{
  for (int i = 0; i < 320 * 240; i++)
  {
    VGA[i] = 0x00;
  }
}

void displayLoading()
{
  loadingBool = 1;
  updateVGADisplay(LoadingOutput_bmp, 100, 200, 100, 130);
  loadingBool = 0;
}

void initiatePicture(int kernel, int kernelSize)
{
  // displayLoading();
  // resetPixelData();
  blurring(output_bmp, kernel, kernelSize);
  // updateVGADisplay(output_bmp, 0, 320, 0, 240);
}

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

int main(void)
{
  createStaticPixelData(output_bmp);
  clearVGADisplay();
  // blurring(output_bmp, 0, 0);
  // updateVGADisplay(output_bmp, 0, outputWidth, 0, outputHeight);
  init();
  while (1)
  {
    // make sure the program is running
  }
}
