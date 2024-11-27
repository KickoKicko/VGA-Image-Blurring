#include <stdint.h>
#include "pixeldata.h"

extern void print(const char *);
extern void print_dec(unsigned int);
extern void display_string(char *);
extern void enable_interrupt(void);

void handle_interrupt()
{
}

/* Add your code here for initializing interrupts. */
void labinit(void)
{
  // enable_interrupt();
}

void updateVGADisplay()
{
  volatile char *VGA = (volatile char *)0x08000000;

  for (int y = 0; y < 240; y++)
  {
    for (int x = 0; x < 320; x++)
    {
      // Calculate source index in the BMP array
      int srcIndex = ((239 - y) * 320) + x + 1162; // 54 for alignment

      // Calculate destination index in the VGA buffer
      int dstIndex = (y * 320) + x;

      // Copy pixel data from BMP to VGA
      VGA[dstIndex] = output_bmp[srcIndex];
    }
  }
}

/* Your code goes into main as well as any needed functions. */
int main(void)
{
  updateVGADisplay();
}