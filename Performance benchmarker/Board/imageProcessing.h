#include <stdint.h>
int generateGaussianKernel(volatile int kernelRadie, int *filterMatrix);
void generateSharpenKernel(volatile int kernelRadie, int *filterMatrix);
uint8_t blurringKernel(uint8_t arr[], int filterMatrix[], int total, int arrSize);
int *matrixGenerator(int blurType, int kernelSize, volatile int kernelRadie, int *sumPointer, int *filterMatrix);
void paddingKernel(volatile int kernelRadie, uint8_t *pixels, int x2, int y2, uint8_t *oldPixelData);
void blurring(uint8_t *pixelData, int blurType, volatile int kernelRadie);

