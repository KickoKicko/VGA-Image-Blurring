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

volatile char *VGA = (volatile char *)0x08000000;
int *edgecaptureSwitch = (int *)0x0400001C;
int *switchData = (int *)0x04000010;
int *edgecaptureButton = (int *)0x040000dC;

int btn_counter = 1;

int loadingBool = 0;

/* Add your code here for initializing interrupts. */
void init(void)
{
  int *interruptmaskSwitch = (int *)0x04000018;
  *interruptmaskSwitch = 0b1111111111;
  int *interruptmaskButton = (int *)0x040000d8;
  *interruptmaskButton = 0b1111111111;
  enable_interrupt();
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

void updateVGADisplay(const char *arr){
  for (int y = 0; y < 240; y++)
  {
    for (int x = 0; x < 320; x++)
    {
      int srcIndex = ((239 - y) * 320) + x; 

      int dstIndex = (y * 320) + x;

      VGA[dstIndex] = arr[srcIndex];
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
  updateVGADisplay(LoadingOutput_bmp);
  loadingBool = 0;
}

void initiatePicture(int kernel, int kernelSize)
{
  displayLoading();
  resetPixelData();
  blurring(output_bmp, kernel, kernelSize);
  updateVGADisplay(output_bmp);
}

int main(void)
{
  clearVGADisplay();
  updateVGADisplay(0, 0);
  init();
  while (1)
  {
    // make sure the program is running
  }
}
