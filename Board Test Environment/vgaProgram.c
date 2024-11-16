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

  // Loop through every pixel in the screen (320x240)
  for (int i = 65; i < (320 * 240) + 65; i++)
  {
    VGA[i - 65] = flowers_bmp[i];
  }
}

/* Your code goes into main as well as any needed functions. */
int main(void)
{
  updateVGADisplay();
}