extern void print(const char *);
extern void print_dec(unsigned int);
extern void display_string(char *);
extern void enable_interrupt(void);

/* Add your code here for initializing interrupts. */
void labinit(void)
{
  // enable_interrupt();
}

int vgaTest()
{
  volatile char *VGA = (volatile char *)0x08000000;
  int count = 0;
  char c = 0;
  for (int i = 0; i < 320 * 240 * 2; i++)
  {
    VGA[i] = i;
  }
}

/* Your code goes into main as well as any needed functions. */
int main(void)
{
  vgaTest();
}