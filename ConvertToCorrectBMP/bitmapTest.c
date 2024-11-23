#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "referenceKernels.h"

#pragma pack(push, 1)
typedef struct
{
    uint16_t bfType;      // File type
    uint32_t bfSize;      // File size in bytes
    uint16_t bfReserved1; // Reserved; must be zero
    uint16_t bfReserved2; // Reserved; must be zero
    uint32_t bfOffBits;   // Offset to start of pixel data
} BMPHeader;

typedef struct
{
    uint32_t bV5Size;
    int32_t bV5Width;
    int32_t bV5Height;
    uint16_t bV5Planes;
    uint16_t bV5BitCount; // Think this chooses the amount of bits for each pixel
    uint32_t bV5Compression;
    uint32_t bV5SizeImage;
    int32_t bV5XPelsPerMeter;
    int32_t bV5YPelsPerMeter;
    uint32_t bV5ClrUsed;
    uint32_t bV5ClrImportant;

    // BITMAPV4HEADER fields
    uint32_t bV5RedMask;
    uint32_t bV5GreenMask;
    uint32_t bV5BlueMask;
    uint32_t bV5AlphaMask;
    uint32_t bV5CSType;
    uint32_t bV5Endpoints[9]; // Three CIEXYZTRIPLE structs, each with three uint32_t values
    uint32_t bV5GammaRed;
    uint32_t bV5GammaGreen;
    uint32_t bV5GammaBlue;

    // BITMAPV5HEADER-specific fields
    uint32_t bV5Intent;
    uint32_t bV5ProfileData;
    uint32_t bV5ProfileSize;
    uint32_t bV5Reserved;
} BMPInfoHeader;

typedef struct
{
    uint8_t rgbBlue;
    uint8_t rgbGreen;
    uint8_t rgbRed;
    uint8_t rgbReserved;
} RGBQUAD;

typedef struct
{
    uint8_t blue;
    uint8_t green;
    uint8_t red;
} PIXEL24;
#pragma pack(pop)

#define M_PI 3.14159265358979323846
const int outputHeight = 240;
const int outputWidth = 320;

extern void sharpen(uint8_t *pixelData, int width, int height, float strength);
extern void convertToGrayscale(uint8_t *pixelData, int width, int height);
extern void gaussianBlur(uint8_t *pixelData, int width, int height, int kernelSize, double sigma);
extern void motionBlur(uint8_t *pixelData, int width, int height, int blurLength);
extern void boxBlur(uint8_t *pixelData, int width, int height, int blurSize);

double colorDistance(PIXEL24 a, RGBQUAD b)
{
    return sqrt(pow(a.red - b.rgbRed, 2) +
                pow(a.green - b.rgbGreen, 2) +
                pow(a.blue - b.rgbBlue, 2));
}

uint8_t findClosestColor(PIXEL24 pixel, RGBQUAD *palette, int paletteSize)
{
    double minDistance = 1e9;
    uint8_t closestIndex = 0;
    for (int i = 0; i < paletteSize; i++)
    {
        double distance = colorDistance(pixel, palette[i]);
        if (distance < minDistance)
        {
            minDistance = distance;
            closestIndex = i;
        }
    }
    return closestIndex;
}

void downScale(uint8_t *pixelData, int width, int height, BMPInfoHeader *infoHeader, RGBQUAD *palette, uint8_t *newPixelData)
{
    double rowSize = (width + 3) & ~3;
    double heightRatio = (double)height / outputHeight;
    double widthRatio = (double)width / outputWidth;
    (*infoHeader).bV5Height = outputHeight;
    (*infoHeader).bV5Width = outputWidth;
    printf("height:%f  width:%f  ", heightRatio, widthRatio);
    int count = 0;
    for (double i = 0; i < outputHeight; i++)
    {
        for (double j = 0; j < outputWidth; j++)
        {
            int pixelPlace = (int)(((int)(i * heightRatio) * rowSize * 3.0) + (j * 3.0 * widthRatio)) - (int)(((int)(i * heightRatio) * rowSize * 3.0) + (j * 3.0 * widthRatio)) % 3;
            PIXEL24 pixel;
            pixel.blue = pixelData[pixelPlace];
            pixel.green = pixelData[pixelPlace + 1];
            pixel.red = pixelData[pixelPlace + 2];
            newPixelData[count] = findClosestColor(pixel, palette, 256);
            count++;
        }
    }
}

void generate332Palette(RGBQUAD *palette)
{
    for (int i = 0; i < 256; i++)
    {
        // Extract 3 bits for red (bits 7-5)
        uint8_t red = (i >> 5) & 0x07; // 3 bits (0-7)
        // Extract 3 bits for green (bits 4-2)
        uint8_t green = (i >> 2) & 0x07; // 3 bits (0-7)
        // Extract 2 bits for blue (bits 1-0)
        uint8_t blue = i & 0x03; // 2 bits (0-3)

        // Scale values to 8-bit range
        palette[i].rgbRed = red * 36;     // 0, 36, 73, ..., 255
        palette[i].rgbGreen = green * 36; // 0, 36, 73, ..., 255
        palette[i].rgbBlue = blue * 85;   // 0, 85, 170, 255
        palette[i].rgbReserved = 0;       // Reserved, typically 0
    }
}

void boxBlur2(uint8_t *pixelData)
{
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "Usage: %s <input_bmp_file> <kernel>\n", argv[0]);
        return 1;
    }

    const char *inputFilePath = argv[1];
    const char *outputFilePath = "output.bmp";
    const char *kernel = argv[2];

    RGBQUAD palette[256];
    // generateVGA256Palette(palette);
    generate332Palette(palette);

    // Open the BMP file
    FILE *inputFile = fopen(inputFilePath, "rb");
    if (!inputFile)
    {
        perror("Failed to open input BMP file");
        return 1;
    }

    BMPHeader header;
    fread(&header, sizeof(BMPHeader), 1, inputFile);
    if (header.bfType != 0x4D42)
    { // Check for 'BM'
        fprintf(stderr, "Not a BMP file\n");
        fclose(inputFile);
        return 1;
    }

    BMPInfoHeader infoHeader;
    fread(&infoHeader, sizeof(BMPInfoHeader), 1, inputFile);

    int width = infoHeader.bV5Width;
    int height = infoHeader.bV5Height;

    // Allocate memory for pixel data
    int rowSize = (width * 3 + 3); // Rows are padded to 4-byte boundaries
    uint8_t *pixelData = (uint8_t *)malloc(rowSize * height);

    printf(" width:%d   height:%d", width, height);
    int memorySize = outputWidth * outputHeight;
    printf("  memorySize:%d   ", memorySize);
    uint8_t *newPixelData = (uint8_t *)malloc(memorySize);
    fseek(inputFile, header.bfOffBits, SEEK_SET);
    fread(pixelData, sizeof(uint8_t), rowSize * height, inputFile);
    fclose(inputFile);

    /*if (strcmp(kernel, "motionBlur") == 0)
    {
        motionBlur(pixelData, width, height, 150);
    }
    else if (strcmp(kernel, "grayScale") == 0)
    {
        convertToGrayscale(pixelData, width, height);
    }
    else if (strcmp(kernel, "gaussianBlur") == 0)
    {
        gaussianBlur(pixelData, width, height, 15, 2.5);
    }
    else if (strcmp(kernel, "boxBlur") == 0)
    {
        boxBlur(pixelData, width, height, 7);
    }
    else if (strcmp(kernel, "sharpen") == 0)
    {
        sharpen(pixelData, width, height, 1.3);
    }
    else if (strcmp(kernel, "test") == 0)
    {
        downScale(pixelData, width, height, &infoHeader, palette, newPixelData);
    }*/

    downScale(pixelData, width, height, &infoHeader, palette, newPixelData);
    if (strcmp(kernel, "motionBlur") == 0)
    {
        boxBlur2(newPixelData);
    }

    // Write the modified image to a new BMP file
    FILE *outputFile = fopen(outputFilePath, "wb");
    if (!outputFile)
    {
        perror("Failed to open output BMP file");
        free(pixelData);
        return 1;
    }
    
    infoHeader.bV5Reserved = 2882382797; // To make it easier to see where the infoheader ends
    infoHeader.bV5ClrUsed = 256;
    infoHeader.bV5ClrImportant = 256;
    infoHeader.bV5BitCount = 8;
    header.bfOffBits = header.bfOffBits + 1024; // Allocating memory for palette data
    header.bfSize = sizeof(header) + sizeof(infoHeader) + outputWidth * outputHeight + 1024;

    free(pixelData);
    fwrite(&header, sizeof(BMPHeader), 1, outputFile);
    fwrite(&infoHeader, sizeof(BMPInfoHeader), 1, outputFile);
    fwrite(palette, sizeof(RGBQUAD), 256, outputFile);
    // Write the pixel data
    fwrite(newPixelData, sizeof(uint8_t), outputWidth * outputHeight, outputFile);
    fclose(outputFile);
    free(newPixelData);
    printf("Converted and saved the resized image to %s\n", outputFilePath);

    return 0;
}