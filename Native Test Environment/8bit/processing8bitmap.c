#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "8bitReferenceKernels.h"

#define M_PI 3.14159265358979323846

#pragma pack(push, 1)
typedef struct
{
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
} BMPHeader;

typedef struct
{
    uint32_t headerSize;
    int32_t BMPwidth;
    int32_t BMPheight;
    uint16_t planes;
    uint16_t bitsPerPixel;
    uint32_t compression;
    uint32_t imageSize;
    int32_t xPixelsPerMeter;
    int32_t yPixelsPerMeter;
    uint32_t colorsUsed;
    uint32_t importantColors;
} BMPInfoHeader;
#pragma pack(pop)

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

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <input_bmp_file>\n", argv[0]);
        return 1;
    }

    const char *inputFilePath = argv[1];
    const char *outputFilePath = "output.bmp";

    // Open input BMP file
    FILE *inputFile = fopen(inputFilePath, "rb");
    if (!inputFile)
    {
        perror("Failed to open input BMP file");
        return 1;
    }

    BMPHeader header;
    fread(&header, sizeof(BMPHeader), 1, inputFile);
    if (header.bfType != 0x4D42)
    {
        fprintf(stderr, "Not a BMP file\n");
        fclose(inputFile);
        return 1;
    }

    BMPInfoHeader infoHeader;
    fread(&infoHeader, sizeof(BMPInfoHeader), 1, inputFile);

    int width = infoHeader.BMPwidth;
    int height = infoHeader.BMPheight;
    int bitsPerPixel = infoHeader.bitsPerPixel;

    if (bitsPerPixel != 8)
    {
        fprintf(stderr, "This code only supports 8-bit BMP files\n");
        fclose(inputFile);
        return 1;
    }

    printf("Width: %d, Height: %d, BitsPerPixel: %d\n", width, height, bitsPerPixel);

    // Read the color palette (8-bit BMPs use a color table with 256 entries)
    int paletteSize = 256 * 4; // Each palette entry is 4 bytes (BGRA)
    uint8_t *colorPalette = (uint8_t *)malloc(paletteSize);
    fread(colorPalette, sizeof(uint8_t), paletteSize, inputFile);

    // Calculate row size with padding
    int rowSize = (width + 3) & ~3; // Width bytes padded to a 4-byte boundary
    uint8_t *pixelData = (uint8_t *)malloc(rowSize * height);
    fseek(inputFile, header.bfOffBits, SEEK_SET);
    fread(pixelData, sizeof(uint8_t), rowSize * height, inputFile);
    fclose(inputFile);

    // Apply general blur
    int blurRadius = 1; // You can adjust the radius for more/less blur
    generalBlur(pixelData, width, height, rowSize, colorPalette, blurRadius);

    // int blurLength = 15; // The length of the blur effect (number of pixels)
    // float angle = 45.0f; // The direction of the motion blur (in degrees)
    // motionBlur(pixelData, width, height, rowSize, colorPalette, blurLength, angle);

    // Open output BMP file
    FILE *outputFile = fopen(outputFilePath, "wb");
    if (!outputFile)
    {
        perror("Failed to open output BMP file");
        free(pixelData);
        free(colorPalette);
        return 1;
    }

    // Write headers to output
    fwrite(&header, sizeof(BMPHeader), 1, outputFile);
    fwrite(&infoHeader, sizeof(BMPInfoHeader), 1, outputFile);
    fwrite(colorPalette, sizeof(uint8_t), paletteSize, outputFile);

    // Write pixel data row-by-row
    for (int i = 0; i < height; i++)
    {
        fwrite(&pixelData[i * rowSize], sizeof(uint8_t), rowSize, outputFile);
    }

    // Clean up
    fclose(outputFile);
    free(pixelData);
    free(colorPalette);

    printf("Saved identical BMP to %s\n", outputFilePath);
    return 0;
}
