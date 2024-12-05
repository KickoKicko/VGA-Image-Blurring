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

void displayKernels(int display_number, int value)
{
  volatile int *display = (volatile int *)0x04000050;
  display += (4 * display_number);
  *display = value;
}

int convert_segment(char number)
{
  switch (number)
  {
  case 0:
    return 0b11000000;
  case 1:
    return 0b11111001;
  case 2:
    return 0b10100100;
  case 3:
    return 0b10110000;
  case 4:
    return 0b10011001;
  case 5:
    return 0b10010010;
  case 6:
    return 0b10000010;
  case 7:
    return 0b11111000;
  case 8:
    return 0b10000000;
  case 9:
    return 0b10010000;
  default:
    return 0b11111111;
  }
}

int getKernel(void)
{
  int kernel = (*switchData >> 7) & 0b111;
  if (kernel > 4)
  {
    kernel = 4;
  }
  return kernel;
}

void handle_interrupt(unsigned cause)
{
  if (cause == 18)
  {
    btn_counter++;
    *edgecaptureButton = 0b1;

    if (btn_counter % 2 == 0)
    {
      int kernel = getKernel();
      int kernelSize = *switchData & 0b111;
      blurring(output_bmp, kernel, kernelSize);
    }
  }
}

void displayMenu(void)
{
  int kernel = getKernel();
  int kernelSize = *switchData & 0b111;
  displayKernels(5, convert_segment(kernel));
  displayKernels(4, 0b10111111);
  displayKernels(3, 0b10111111);
  displayKernels(2, 0b10111111);
  displayKernels(1, 0b10111111);
  displayKernels(0, convert_segment(kernelSize));
}

int main(void)
{
  init();
  //unsigned int foo_time;
  //asm volatile("csrw mcycle, x0");
  blurring(output_bmp, 0, 0);
  //asm("csrr %0, mcycle" : "=r"(foo_time));
  //print_dec(foo_time);
  while (1)
  {
    displayMenu();
    // make sure the program is running
  }
}
