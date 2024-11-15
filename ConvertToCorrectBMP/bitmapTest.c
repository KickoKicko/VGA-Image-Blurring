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
    uint16_t bV5BitCount;//Think this chooses the amount of bits for each pixel
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

typedef struct {
    uint8_t rgbBlue;
    uint8_t rgbGreen;
    uint8_t rgbRed;
    uint8_t rgbReserved;
} RGBQUAD;
#pragma pack(pop)

#define M_PI 3.14159265358979323846

extern void sharpen(uint8_t *pixelData, int width, int height, float strength);
extern void convertToGrayscale(uint8_t *pixelData, int width, int height);
extern void gaussianBlur(uint8_t *pixelData, int width, int height, int kernelSize, double sigma);
extern void motionBlur(uint8_t *pixelData, int width, int height, int blurLength);
extern void boxBlur(uint8_t *pixelData, int width, int height, int blurSize);


void downScale(uint8_t *pixelData, int width, int height, BMPInfoHeader infoHeader)
{
    if(((double)width/height) >(4.0/3)){//This means the width is to large for the height
        //printf("111");
        printf("width:%d  height:%d  \n",width,height);
        height = height - height%3;// Make sure the height is dividible by 3
        width = (4*(height/3));// Make sure the new ratio is 4:3
        printf("width:%d  height:%d",width,height);

        for (int i = 0; i < height*width; i++)
        {
            pixelData[i] = pixelData[i];
        }
        infoHeader.bV5Height = height;
        infoHeader.bV5Width = width;

    }
    else if(((double)width/height) <(4.0/3)){//This means the height is to large for the width
        printf("222");
    }
    else{
        //printf("333   %f\n", ((double)width/height));
        printf("333  ");
    }
    printf("\n");
}

void createGrayscalePalette(RGBQUAD *palette) {
    for (int i = 0; i < 256; i++) {
        palette[i].rgbRed = i;      // R, G, B are the same for grayscale
        palette[i].rgbGreen = i;
        palette[i].rgbBlue = i;
        palette[i].rgbReserved = 0;
    }
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
    fseek(inputFile, header.bfOffBits, SEEK_SET);
    fread(pixelData, sizeof(uint8_t), rowSize * height, inputFile);
    fclose(inputFile);

    if (strcmp(kernel, "motionBlur") == 0)
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
        downScale(pixelData, width, height, infoHeader);
    }

    // Write the modified image to a new BMP file
    FILE *outputFile = fopen(outputFilePath, "wb");
    if (!outputFile)
    {
        perror("Failed to open output BMP file");
        free(pixelData);
        return 1;
    }
    if(strcmp(kernel, "test") != 0){
        printf("The number is: %d\n", sizeof(BMPHeader));
        printf("The number is: %d\n", sizeof(BMPInfoHeader));
        printf("The number is: %d\n", infoHeader.bV5BitCount);

        // Write the BMP header
        fwrite(&header, sizeof(BMPHeader), 1, outputFile);
        // Write the BMP info header
        fwrite(&infoHeader, sizeof(BMPInfoHeader), 1, outputFile);
        // Write the pixel data
        fwrite(pixelData, sizeof(uint8_t), rowSize * height, outputFile);

        // Clean up
        fclose(outputFile);
        free(pixelData);

        printf("Converted and saved the grayscale image to %s\n", outputFilePath);
    }
    else if(strcmp(kernel, "test") == 0){
        infoHeader.bV5Reserved = 2882382797;
        infoHeader.bV5Width = 568;
        infoHeader.bV5ClrUsed = 256;
        infoHeader.bV5ClrImportant = 256;
        rowSize = (width + 3)& ~3;
        infoHeader.bV5BitCount = 8;
        header.bfOffBits = header.bfOffBits+1024;
        //infoHeader.bV5ClrUsed = 256;
        //infoHeader.bV5ClrImportant = 256;
        header.bfSize = sizeof(header)+ sizeof(infoHeader)+rowSize*height*sizeof(uint8_t)+10000;
        RGBQUAD palette[256];
        createGrayscalePalette(palette);

        fwrite(&header, sizeof(BMPHeader), 1, outputFile);
        fwrite(&infoHeader, sizeof(BMPInfoHeader), 1, outputFile);
        fwrite(palette, sizeof(RGBQUAD), 256, outputFile);
        // Write the pixel data
        fwrite(pixelData, sizeof(uint8_t), rowSize * height, outputFile);
        fclose(outputFile);
        free(pixelData);
        printf("Converted and saved the resized image to %s\n", outputFilePath);
    }

    
    return 0;
}
