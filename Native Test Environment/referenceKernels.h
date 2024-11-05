#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

void sharpen(uint8_t *pixelData, int width, int height, float strength);
void convertToGrayscale(uint8_t *pixelData, int width, int height);
void gaussianBlur(uint8_t *pixelData, int width, int height, int kernelSize, double sigma);
void motionBlur(uint8_t *pixelData, int width, int height, int blurLength);
void boxBlur(uint8_t *pixelData, int width, int height, int blurSize);
