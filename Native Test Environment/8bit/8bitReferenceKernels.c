#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "8bitReferenceKernels.h"

#define M_PI 3.14159265358979323846

// Function to apply motion blur
void motionBlur(uint8_t *pixelData, int width, int height, int rowSize, uint8_t *colorPalette, int blurLength, float angle)
{
    // Temporary array to store the blurred data
    uint8_t *tempData = (uint8_t *)malloc(rowSize * height);

    // Convert angle to radians
    float rad = angle * M_PI / 180.0f;
    float cosAngle = cos(rad);
    float sinAngle = sin(rad);

    // Iterate over each pixel
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int sumR = 0, sumG = 0, sumB = 0;
            int count = 0;

            // Apply the motion blur along the line direction based on the angle
            for (int i = -blurLength / 2; i <= blurLength / 2; i++)
            {
                // Calculate the position of the blurred pixel in the direction of the motion
                int nx = (int)(x + i * cosAngle);
                int ny = (int)(y + i * sinAngle);

                // Ensure the new pixel position is within bounds
                if (nx >= 0 && nx < width && ny >= 0 && ny < height)
                {
                    uint8_t pixelIndex = pixelData[ny * rowSize + nx];
                    uint8_t *color = &colorPalette[pixelIndex * 4]; // Palette is in BGRA format

                    // Sum the BGR components (assuming color is BGRA)
                    sumB += color[0];
                    sumG += color[1];
                    sumR += color[2];
                    count++;
                }
            }

            // Compute the average color for the motion blur
            uint8_t avgB = sumB / count;
            uint8_t avgG = sumG / count;
            uint8_t avgR = sumR / count;

            // Find the closest color in the palette
            int closestIndex = 0;
            int minDistance = 256 * 256 * 3; // Max possible distance (RGB space)

            for (int i = 0; i < 256; i++)
            {
                uint8_t *paletteColor = &colorPalette[i * 4];
                int distB = avgB - paletteColor[0];
                int distG = avgG - paletteColor[1];
                int distR = avgR - paletteColor[2];
                int distance = distB * distB + distG * distG + distR * distR;

                if (distance < minDistance)
                {
                    minDistance = distance;
                    closestIndex = i;
                }
            }

            // Set the blurred pixel index to the closest palette entry
            tempData[y * rowSize + x] = closestIndex;
        }
    }

    // Copy the blurred data back to the original pixel data
    memcpy(pixelData, tempData, rowSize * height);
    free(tempData);
}

// Function to apply general blur
void generalBlur(uint8_t *pixelData, int width, int height, int rowSize, uint8_t *colorPalette, int blurRadius)
{
    // Temporary array to store the blurred data
    uint8_t *tempData = (uint8_t *)malloc(rowSize * height);

    // Iterate over each pixel
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int sumR = 0, sumG = 0, sumB = 0;
            int count = 0;

            // Sum up color values in the blur radius (within bounds)
            for (int ky = -blurRadius; ky <= blurRadius; ky++)
            {
                for (int kx = -blurRadius; kx <= blurRadius; kx++)
                {
                    int ny = y + ky;
                    int nx = x + kx;
                    if (nx >= 0 && nx < width && ny >= 0 && ny < height) // Only add pixels within bounds
                    {
                        uint8_t pixelIndex = pixelData[ny * rowSize + nx];
                        uint8_t *color = &colorPalette[pixelIndex * 4]; // Palette is in BGRA format

                        // Sum the BGR components (assuming color is BGRA)
                        sumB += color[0];
                        sumG += color[1];
                        sumR += color[2];
                        count++;
                    }
                }
            }

            // Compute the average color
            uint8_t avgB = sumB / count;
            uint8_t avgG = sumG / count;
            uint8_t avgR = sumR / count;

            // Find the closest color in the palette
            int closestIndex = 0;
            int minDistance = 256 * 256 * 3; // Max possible distance (RGB space)

            for (int i = 0; i < 256; i++)
            {
                uint8_t *paletteColor = &colorPalette[i * 4];
                int distB = avgB - paletteColor[0];
                int distG = avgG - paletteColor[1];
                int distR = avgR - paletteColor[2];
                int distance = distB * distB + distG * distG + distR * distR;

                if (distance < minDistance)
                {
                    minDistance = distance;
                    closestIndex = i;
                }
            }

            // Set the blurred pixel index to the closest palette entry
            tempData[y * rowSize + x] = closestIndex;
        }
    }

    // Copy the blurred data back to the original pixel data
    memcpy(pixelData, tempData, rowSize * height);
    free(tempData);
}