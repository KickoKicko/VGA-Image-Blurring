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

int *edgecaptureSwitch = (int *)0x0400001C;
int *switchData = (int *)0x04000010;
int *edgecaptureButton = (int *)0x040000dC;

int btn_counter = 1;

void init(void)
{
  int *interruptmaskSwitch = (int *)0x04000018;
  *interruptmaskSwitch = 0b1111111111;
  int *interruptmaskButton = (int *)0x040000d8;
  *interruptmaskButton = 0b1111111111;
  enable_interrupt();
}

void handle_interrupt(unsigned cause)
{
  if (cause == 18)
  {
    btn_counter++;
    *edgecaptureButton = 0b1;

    if (btn_counter % 2 == 0)
    {
      int kernel = (*switchData >> 8) & 0b11;
      int kernelSize = *switchData & 0b111;
      blurring(output_bmp, kernel, kernelSize);
    }
  }
}

int main(void)
{
  init();
  blurring(output_bmp, 0, 0);
  while (1)
  {
    // make sure the program is running
  }
}
