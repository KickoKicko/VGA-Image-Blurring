/* main.c

   This file written 2024 by Artur Podobas and Pedro Antunes

   For copyright and licensing, see file COPYING */

/* Below functions are external and found in other files. */
#include <stdlib.h>

extern void print(const char *);
extern void print_dec(unsigned int);
extern void display_string(char *);
extern void time2string(char *, int);
extern void tick(int *);
extern void delay(int);
extern int nextprime(int);
extern void enable_interrupt(void);

int mytime = 0;
char textstring[] = "text, more text, and even more text!";

int total_seconds = 0;

int prime = 1234567;

int time_counter = 0;

/* Add your code here for initializing interrupts. */
void labinit(void)
{
  int *control = (int *)0x04000024;
  int *periodl = (int *)0x04000028;
  int *periodh = (int *)0x0400002C;
  *periodl = 0xc6bf & 0xffff;
  *periodh = 0x2d & 0xffff;
  *control = 0b111;
  enable_interrupt();
}

void set_leds(int led_mask)
{
  volatile int *LED = (volatile int *)0x04000000;
  *LED = led_mask & 0x3FF;
}

void set_displays(int display_number, int value)
{
  volatile int *display = (volatile int *)0x04000050;
  display += (4 * display_number);
  *display = value;
}

int get_sw(void)
{
  volatile int *sw = (volatile int *)0x04000010;
  int value = *sw & 0x3FF;
  return value;
}

// int get_sw(void)
// {
//   volatile int *sw = (volatile int *)0x04000010;
//   int value = (*sw >> 1) & 0x3FF;
//   return value;
// }

int get_btn(void)
{
  volatile int *btn = (volatile int *)0x040000d0;
  int status = *btn & 0x1;
  return status;
}

int convert_segment(char number)
{
  switch (number)
  {
  case 0:
    return 0b01000000;
  case 1:
    return 0b01111001;
  case 2:
    return 0b00100100;
  case 3:
    return 0b00110000;
  case 4:
    return 0b00011001;
  case 5:
    return 0b00010010;
  case 6:
    return 0b00000010;
  case 7:
    return 0b01111000;
  case 8:
    return 0b00000000;
  case 9:
    return 0b00010000;
  default:
    return 0b11111111;
  }
}

void set_all_displays(int s)
{
  set_displays(0, convert_segment(s % 10));        // unit total_seconds
  set_displays(1, convert_segment((s / 10) % 6));  // ten total_seconds
  set_displays(2, convert_segment((s / 60) % 10)); // unit minutes
  set_displays(3, convert_segment((s / 600) % 6)); // ten minutes

  if (((s / 36000) % 3) >= 2)
  {
    set_displays(4, convert_segment((s / 3600) % 5)); // unit hours
  }
  else
  {
    set_displays(4, convert_segment((s / 3600) % 10)); // unit hours
  }

  set_displays(5, convert_segment((s / 36000) % 3)); // ten hours
}

char which_pair(void)
{
  int sw = get_sw();
  int sw_mask = sw & 0b1110000000;
  switch (sw_mask)
  {
  case (0b0000000000):
    return -1;
  case (0b0100000000):
    return 's';
  case (0b1000000000):
    return 'm';
  case (0b1100000000):
    return 'h';
  default:
    return 0;
  }
}

void display_timer_live()
{
  total_seconds++;
  if (total_seconds >= 86400)
    total_seconds = 0;
  set_all_displays(total_seconds);
}

/* Below is the function that will be called when an interrupt is triggered. */
void handle_interrupt(unsigned cause)
{
  volatile int *status = (int *)0x04000020;
  *status = 0;

  time_counter++;
  if (time_counter >= 10)
  {
    time_counter = 0;
    display_timer_live();
  }
}

void display_timer_from_switches(void)
{
  int masked_value = get_sw() & 0b111111;
  switch (which_pair())
  {
  case 'h':
    masked_value %= 24;
    total_seconds %= 3600;
    total_seconds += 3600 * masked_value;
    break;
  case 'm':
    int minutes = ((total_seconds / 60) % 60);
    masked_value %= 60;
    total_seconds -= 60 * minutes;
    total_seconds += 60 * masked_value;
    break;
  case 's':
    int seconds = ((total_seconds) % 60);
    masked_value %= 60;
    total_seconds -= seconds;
    total_seconds += masked_value;
    break;
  case 0:
    return;
  default:
    break;
  }
  set_all_displays(total_seconds);
}

int get_timeout(void)
{
  volatile int *status = (int *)0x04000020;

  if ((*status & 0b1) == 1)
  {
    *status = 0;
    return 1;
  }
  else
  {
    return 0;
  }
}

void set_displays_zero(void)
{
  for (int i = 0; i < 7; i++)
  {
    set_displays(i, convert_segment(0));
  }
}

int vgaTest()
{
  volatile char *VGA = (volatile char *)0x08000000;
  int colorValue1 = 0x0000000f;
  char colorValue2 = 0b11100000;
  char colorValue3 = 0b00110011;
  char colorValue4 = 0b11110011;
  char colorValue5 = 0b11111111;

  int count = 0;
  char c = 0;
  for (int i = 0; i < 320 * 240; i++)
  {
    VGA[i] = i;
    c++;
  }
  // for (int i = 320 * 240; i < 320 * 240 * 2; i++)
  // {
  //   VGA[i] = 0xff;
  // }

  // unsigned int y_ofs = 0;
  // volatile int *VGA_CTRL = (volatile int *)0x04000100;
  // while (1)
  // {
  //   *(VGA_CTRL + 1) = (unsigned int)(VGA + y_ofs * 320);
  //   *(VGA_CTRL + 0) = 0;
  //   y_ofs = (y_ofs + 1) % 240;
  //   for (int i = 0; i < 1000000; i++)
  //     asm volatile("nop");
  // }
}

/* Your code goes into main as well as any needed functions. */
int main(void)
{
  // labinit();
  // set_displays_zero();
  // while (1)
  // {
  //   print("Prime : ");
  //   prime = nextprime(prime);
  //   print_dec(prime);
  //   print("\n");

  // if we want to be able to change switches during run, note: ito is disabled when holding the button, can be changed by removing the lines with control in them
  // if (get_btn())
  // {
  //   int *control = (int *)0x04000024;
  //   *control = 0b110;
  //   display_timer_from_switches();
  // }
  // else
  // {
  //   int *control = (int *)0x04000024;
  //   *control = 0b111;
  // }
  //}
  vgaTest();
}