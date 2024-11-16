#include <stdint.h>
#include "pixeldata.h"

extern void print(const char *);
extern void print_dec(unsigned int);
extern void display_string(char *);
extern void enable_interrupt(void);

void handle_interrupt(){

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
    for (int i = 0; i < 320 * 240; i++)
    {
        // Set the pixel color (cycle through the palette)
        VGA[i] = flowers_bmp[i];  // Use modulo to cycle through the 256 colors
    }
}

/* Your code goes into main as well as any needed functions. */
int main(void)
{
  updateVGADisplay();
}