#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

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
#define M_EULER 2.718281828459045235360287471352 
const int outputHeight = 240;
const int outputWidth = 320;

extern void sharpen(uint8_t *pixelData, int width, int height, float strength);
extern void convertToGrayscale(uint8_t *pixelData, int width, int height);
extern void gaussianBlur(uint8_t *pixelData, int width, int height, int kernelSize, double sigma);
extern void motionBlur(uint8_t *pixelData, int width, int height, int blurLength);
extern void boxBlur(uint8_t *pixelData, int width, int height, int blurSize);

double colorDistance24(PIXEL24 a, RGBQUAD b)
{
    return sqrt(pow(a.red - b.rgbRed, 2) +
                pow(a.green - b.rgbGreen, 2) +
                pow(a.blue - b.rgbBlue, 2));
}

uint8_t findClosestColor24(PIXEL24 pixel, RGBQUAD *palette, int paletteSize)
{
    double minDistance = 1e9;
    uint8_t closestIndex = 0;
    for (int i = 0; i < paletteSize; i++)
    {
        double distance = colorDistance24(pixel, palette[i]);
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
    int memorySize = outputWidth * outputHeight;
    double rowSize = (width + 3) & ~3;
    double heightRatio = (double)height / outputHeight;
    double widthRatio = (double)width / outputWidth;
    (*infoHeader).bV5Height = outputHeight;
    (*infoHeader).bV5Width = outputWidth;
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
            newPixelData[count] = findClosestColor24(pixel, palette, 256);
            count++;
        }
    }
}

void generate332Palette(RGBQUAD *palette)
{
    for (int i = 0; i < 256; i++)
    {
        uint8_t red = (i >> 5) & 0x07;
        uint8_t green = (i >> 2) & 0x07;
        uint8_t blue = i & 0x03;

        // Scale values to 8-bit range
        palette[i].rgbRed = red * 36;     // 0, 36, 73, ..., 255
        palette[i].rgbGreen = green * 36; // 0, 36, 73, ..., 255
        palette[i].rgbBlue = blue * 85;   // 0, 85, 170, 255
        palette[i].rgbReserved = 0;       // Reserved, typically 0
    }
}

int generateGaussianKernel(int kernelRadie, int *filterMatrix)
{
    double sigma = 0.84089642;
    int sum = 0;
    int count = 0;
    // Generate the kernel values
    double multiplier = 1/(pow(M_EULER,(-((kernelRadie * kernelRadie)*2) / (2 * sigma * sigma)))/(2 * M_PI * sigma * sigma));
    for (int y = -kernelRadie; y <= kernelRadie; y++)
    {
        for (int x = -kernelRadie; x <= kernelRadie; x++)
        {
            filterMatrix[count] = roundf(multiplier*pow(M_EULER,(-((x * x) + (y * y)) / (2 * sigma * sigma)))/(2 * M_PI * sigma * sigma));
            sum += filterMatrix[count];
            count++;
        }
    }
    return sum;
}

void generateSharpenKernel(int kernelRadie, int *filterMatrix)
{
    int count = 0;
    // Generate the kernel values
    for (int y = -kernelRadie; y <= kernelRadie; y++)
    {
        for (int x = -kernelRadie; x <= kernelRadie; x++)
        {
            if(x==0 && y == 0){
                filterMatrix[count] = 5;
            }
            else if((x==1 && y == 0) || (x==-1 && y == 0) || (x==0 && y == 1) || (x==0 && y == -1)){
                filterMatrix[count] = -1;
            }
            else{
            filterMatrix[count] = 0;
            }
            count++;
        }
    }
}

uint8_t blurringKernel(uint8_t arr[], int filterMatrix[], int total, int arrSize)
{
    int blueTotal = 0;
    int greenTotal = 0;
    int redTotal = 0;
    for (int i = 0; i < arrSize; i++)
    {
        blueTotal += filterMatrix[i] * ((arr[i] >> 0) & 3);
        greenTotal += filterMatrix[i] * ((arr[i] >> 2) & 7);
        redTotal += filterMatrix[i] * ((arr[i] >> 5) & 7);
    }
    blueTotal = (blueTotal + (total / 2)) / total;
    greenTotal = (greenTotal + (total / 2)) / total;
    redTotal = (redTotal + (total / 2)) / total;

    blueTotal = fmax(0, fmin(3, blueTotal));
    greenTotal = fmax(0, fmin(7, greenTotal));
    redTotal = fmax(0, fmin(7, redTotal));
    return blueTotal + (greenTotal << 2) + (redTotal << 5);
}

int *matrixGenerator(int blurType, int kernelSize, int kernelRadie, int *sumPointer)
{
    int *filterMatrix = (int *)malloc(kernelSize * sizeof(int));
    if (blurType == 0)// BoxBlur
    {
        for (size_t i = 0; i < kernelSize; i++)
        {
            filterMatrix[i] = 1;
        }
    }
    else if (blurType == 1) // GaussianBlur
    {
        *sumPointer = generateGaussianKernel(kernelRadie, filterMatrix);
    }
    else if (blurType == 2) // Sharpen
    {
        generateSharpenKernel(kernelRadie, filterMatrix);
    }
    else if (blurType == 3) // MotionBlur
    {
        for (int i = 0; i < kernelRadie * 2 + 1; i++)
        {
            for (int j = 0; j < kernelRadie * 2 + 1; j++)
            {
                if (i == kernelRadie)
                {
                    filterMatrix[i * (kernelRadie * 2 + 1) + j] = 1;
                }
                else
                {
                    filterMatrix[i * (kernelRadie * 2 + 1) + j] = 0;
                }
            }
        }
    }

    return filterMatrix;
}

void paddingKernel(int kernelRadie, uint8_t *pixels, int x2, int y2, uint8_t * oldPixelData){
    int count = 0;
    for (int y = kernelRadie; y >= -kernelRadie; y--)
    {
        for (int x = -kernelRadie; x <= kernelRadie; x++)
        {
            if (x + x2 < outputWidth && y + y2 < outputHeight && x + x2 >= 0 && y + y2 >= 0){
                pixels[count] = oldPixelData[x + x2 + ((y + y2) * outputWidth)];
            }
            else if(y + y2 < outputHeight && y + y2 >= 0){
                pixels[count] = oldPixelData[(-1*x) + x2 + ((y + y2) * outputWidth)];
            }
            else if(x + x2 < outputWidth && x + x2 >= 0){
                pixels[count] = oldPixelData[x + x2 + (((-1*y) + y2) * outputWidth)];
            }
            else {
                pixels[count] = oldPixelData[(-1*x) + x2 + (((-1*y) + y2) * outputWidth)];
            }
            count++;
        }
    }
    
}

void blurring(uint8_t *pixelData, int blurType, int kernelRadie)
{
    uint8_t *tempPixelData = (uint8_t *)malloc(outputHeight * outputWidth);
    for (int y = 0; y < outputHeight; y++)
    {
        for (int x = 0; x < outputWidth; x++)
        {
            int position = x + (y * outputWidth);
            int kernelSize = (kernelRadie * 2 + 1) * (kernelRadie * 2 + 1);
            uint8_t *temp = (uint8_t *)malloc(kernelSize);
            if (x <= kernelRadie - 1 || y <= kernelRadie - 1 || x >= outputWidth - kernelRadie || y >= outputHeight - kernelRadie)
            {
                paddingKernel(kernelRadie, temp, x, y, pixelData);// Blurring the outer lines through mirrored padding
            }
            else
            {
                int count = 0;
                for (int i = -kernelRadie; i <= kernelRadie; i++)
                {
                    for (int j = -kernelRadie; j <= kernelRadie; j++)
                    {
                        temp[count] = pixelData[position + j + (i * outputWidth)];
                        count++;
                    }
                }
            }
            int sum = 0;
            int *filterMatrix = matrixGenerator(blurType, kernelSize, kernelRadie, &sum);
            if (blurType == 0)
            {// Box
                tempPixelData[position] = blurringKernel(temp, filterMatrix, kernelSize, kernelSize);
            }
            else if (blurType == 1)
            { // Gaussian
                tempPixelData[position] = blurringKernel(temp, filterMatrix, sum, kernelSize);
            }
            else if (blurType == 2)
            { // Sharpen
                tempPixelData[position] = blurringKernel(temp, filterMatrix, 1, kernelSize);
            }
            else if (blurType == 3)
            { // Motion
                tempPixelData[position] = blurringKernel(temp, filterMatrix, kernelRadie * 2 + 1, kernelSize);
            }
        }
    }

    for (int i = 0; i < 76800; i++)
    {
        pixelData[i] = tempPixelData[i];
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
    const int kernelSizeInput = atoi(argv[3]);

    RGBQUAD palette[256];
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

    int memorySize = outputWidth * outputHeight;
    uint8_t *newPixelData = (uint8_t *)malloc(memorySize);
    fseek(inputFile, header.bfOffBits, SEEK_SET);
    fread(pixelData, sizeof(uint8_t), rowSize * height, inputFile);
    fclose(inputFile);

    downScale(pixelData, width, height, &infoHeader, palette, newPixelData);
    if (strcmp(kernel, "boxBlur") == 0)
    {
        blurring(newPixelData, 0, kernelSizeInput);
    }
    else if (strcmp(kernel, "gaussianBlur") == 0)
    {
        blurring(newPixelData, 1, kernelSizeInput);
    }
    else if (strcmp(kernel, "sharpen") == 0)
    {
        blurring(newPixelData, 2, kernelSizeInput);
    }
    else if (strcmp(kernel, "motionBlur") == 0)
    {
        blurring(newPixelData, 3, kernelSizeInput);
    }
    else if (strcmp(kernel, "test") == 0)
    {
        blurring(newPixelData, 3, kernelSizeInput);
    }

    // Write the modified image to a new BMP file
    FILE *outputFile = fopen(outputFilePath, "wb");
    if (!outputFile)
    {
        perror("Failed to open output BMP file");
        free(pixelData);
        return 1;
    }

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