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

// Function to convert RGB to RRRGGGBB format (3 bits for red, 3 bits for green, 2 bits for blue)
uint8_t convertToRRRGGGBB(uint8_t r, uint8_t g, uint8_t b)
{
    uint8_t r3 = (r >> 5) & 0x07; // 3 bits for red
    uint8_t g3 = (g >> 5) & 0x07; // 3 bits for green
    uint8_t b2 = (b >> 6) & 0x03; // 2 bits for blue

    return (r3 << 5) | (g3 << 2) | b2; // Combine into a single byte
}

// Function to convert the palette
void convertPaletteToRRRGGGBB(uint8_t *colorPalette, int paletteSize)
{
    for (int i = 0; i < paletteSize; i += 4) // Each palette entry has 4 bytes (BGRA)
    {
        uint8_t b = colorPalette[i];     // Blue
        uint8_t g = colorPalette[i + 1]; // Green
        uint8_t r = colorPalette[i + 2]; // Red

        // Convert and update the palette entry to RRRGGGBB
        uint8_t newColor = convertToRRRGGGBB(r, g, b);
        colorPalette[i] = newColor; // Store the new color (the alpha channel is ignored)
    }
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

    convertPaletteToRRRGGGBB(colorPalette, paletteSize);

    // Calculate row size with padding
    int rowSize = (width + 3) & ~3; // Width bytes padded to a 4-byte boundary
    uint8_t *pixelData = (uint8_t *)malloc(rowSize * height);
    fseek(inputFile, header.bfOffBits, SEEK_SET);
    fread(pixelData, sizeof(uint8_t), rowSize * height, inputFile);
    fclose(inputFile);

    // Apply general blur
    // int blurRadius = 1; // You can adjust the radius for more/less blur
    // generalBlur(pixelData, width, height, rowSize, colorPalette, blurRadius);

    // int blurLength = 15; // The length of the blur effect (number of pixels)
    // float angle = 90.0f; // The direction of the motion blur (in degrees)
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
