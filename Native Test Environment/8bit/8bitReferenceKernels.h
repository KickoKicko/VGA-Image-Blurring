#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

void motionBlur(uint8_t *pixelData, int width, int height, int rowSize, uint8_t *colorPalette, int blurLength, float angle);
void generalBlur(uint8_t *pixelData, int width, int height, int rowSize, uint8_t *colorPalette, int blurRadius);