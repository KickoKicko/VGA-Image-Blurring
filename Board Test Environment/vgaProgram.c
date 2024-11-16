#include <stdint.h>
#include "pixeldata.h"

extern void print(const char *);
extern void print_dec(unsigned int);
extern void display_string(char *);
extern void enable_interrupt(void);

/* Add your code here for initializing interrupts. */
void labinit(void)
{
  // enable_interrupt();
}

uint8_t tempCreatePixelData()
{
  uint8_t tempPixelData[320 * 240];
  for (int i = 0; i < 320 * 240; i++)
  {
    tempPixelData[i] = i;
  }
  return *tempPixelData;
}

void updateVGADisplay(uint8_t *pixelData)
{
  volatile char *VGA = (volatile char *)0x08000000;
  for (int i = 0; i < 320 * 240; i++)
  {
    VGA[i] = flowers_bmp[i];
  }
}

char readJtag()
{
  volatile char *JTAG = (volatile char *)0x04000040;
  return JTAG[0];
}

/* Your code goes into main as well as any needed functions. */
int main(void)
{
  // updateVGADisplay(tempCreatePixelData());
  print(readJtag());
}