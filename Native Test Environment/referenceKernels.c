#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "referenceKernels.h";

#define M_PI 3.14159265358979323846

void convertToGrayscale(uint8_t *pixelData, int width, int height)
{
    int rowSize = (width * 3 + 3); // Rows are padded to 4-byte boundaries
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int index = y * rowSize + x * 3;
            uint8_t b = pixelData[index];     // Blue
            uint8_t g = pixelData[index + 1]; // Green
            uint8_t r = pixelData[index + 2]; // Red
            // Simple luminosity method for grayscale
            uint8_t gray = (uint8_t)(0.299 * r + 0.587 * g + 0.114 * b);
            pixelData[index] = gray;     // Blue
            pixelData[index + 1] = gray; // Green
            pixelData[index + 2] = gray; // Red
        }
    }
}

void gaussianBlur(uint8_t *pixelData, int width, int height, int kernelSize, double sigma)
{
    int rowSize = (width * 3 + 3) & ~3;                         // Rows are padded to 4-byte boundaries
    uint8_t *blurredData = (uint8_t *)malloc(height * rowSize); // Allocate memory for the blurred image

    // Create Gaussian kernel
    double *kernel = (double *)malloc(kernelSize * kernelSize * sizeof(double));
    double sum = 0.0;
    int halfSize = kernelSize / 2;

    // Calculate Gaussian kernel values
    for (int y = -halfSize; y <= halfSize; y++)
    {
        for (int x = -halfSize; x <= halfSize; x++)
        {
            double exponent = -(x * x + y * y) / (2 * sigma * sigma);
            double value = (1.0 / (2 * M_PI * sigma * sigma)) * exp(exponent);
            kernel[(y + halfSize) * kernelSize + (x + halfSize)] = value;
            sum += value; // Normalize sum
        }
    }

    // Normalize the kernel
    for (int i = 0; i < kernelSize * kernelSize; i++)
    {
        kernel[i] /= sum;
    }

    // Apply Gaussian blur
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            double rSum = 0, gSum = 0, bSum = 0;

            // Apply the kernel
            for (int ky = -halfSize; ky <= halfSize; ky++)
            {
                for (int kx = -halfSize; kx <= halfSize; kx++)
                {
                    int ny = y + ky;
                    int nx = x + kx;

                    // Ensure we stay within image bounds
                    if (ny >= 0 && ny < height && nx >= 0 && nx < width)
                    {
                        int index = ny * rowSize + nx * 3;
                        double weight = kernel[(ky + halfSize) * kernelSize + (kx + halfSize)];
                        bSum += pixelData[index] * weight;     // Blue
                        gSum += pixelData[index + 1] * weight; // Green
                        rSum += pixelData[index + 2] * weight; // Red
                    }
                }
            }

            // Set blurred pixel
            int index = y * rowSize + x * 3;
            blurredData[index] = (uint8_t)(bSum);     // Blue
            blurredData[index + 1] = (uint8_t)(gSum); // Green
            blurredData[index + 2] = (uint8_t)(rSum); // Red
        }
    }

    // Copy blurred data back to original pixelData
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int index = y * rowSize + x * 3;
            pixelData[index] = blurredData[index];         // Blue
            pixelData[index + 1] = blurredData[index + 1]; // Green
            pixelData[index + 2] = blurredData[index + 2]; // Red
        }
    }

    // Clean up
    free(blurredData);
    free(kernel);
}

void sharpen(uint8_t *pixelData, int width, int height, float strength)
{
    int rowSize = (width * 3 + 3) & ~3;                           // Rows are padded to 4-byte boundaries
    uint8_t *sharpenedData = (uint8_t *)malloc(height * rowSize); // Allocate memory for the sharpened image
    memcpy(sharpenedData, pixelData, height * rowSize);           // Copy original data for reference

    // Sharpen kernel (Laplacian kernel)
    int kernel[3][3] = {
        {0, -1, 0},
        {-1, 5, -1},
        {0, -1, 0}};

    // Apply sharpening filter
    for (int y = 1; y < height - 1; y++)
    {
        for (int x = 1; x < width - 1; x++)
        {
            int rSum = 0, gSum = 0, bSum = 0;

            // Convolve with the kernel
            for (int ky = -1; ky <= 1; ky++)
            {
                for (int kx = -1; kx <= 1; kx++)
                {
                    int ny = y + ky;
                    int nx = x + kx;
                    int index = ny * rowSize + nx * 3;

                    int kernelValue = kernel[ky + 1][kx + 1];
                    bSum += pixelData[index] * kernelValue;     // Blue
                    gSum += pixelData[index + 1] * kernelValue; // Green
                    rSum += pixelData[index + 2] * kernelValue; // Red
                }
            }

            // Normalize and apply strength
            int index = y * rowSize + x * 3;
            sharpenedData[index] = (uint8_t)fmin(fmax(bSum * strength, 0), 255);     // Blue
            sharpenedData[index + 1] = (uint8_t)fmin(fmax(gSum * strength, 0), 255); // Green
            sharpenedData[index + 2] = (uint8_t)fmin(fmax(rSum * strength, 0), 255); // Red
        }
    }

    // Copy sharpened data back to original pixelData
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int index = y * rowSize + x * 3;
            pixelData[index] = sharpenedData[index];         // Blue
            pixelData[index + 1] = sharpenedData[index + 1]; // Green
            pixelData[index + 2] = sharpenedData[index + 2]; // Red
        }
    }

    free(sharpenedData); // Free the allocated memory
}

void motionBlur(uint8_t *pixelData, int width, int height, int blurLength)
{
    int rowSize = (width * 3 + 3) & ~3;                         // Rows are padded to 4-byte boundaries
    uint8_t *blurredData = (uint8_t *)malloc(height * rowSize); // Allocate memory for the blurred image
    memcpy(blurredData, pixelData, height * rowSize);           // Copy original data to preserve unblurred areas

    // Apply horizontal motion blur
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int rSum = 0, gSum = 0, bSum = 0;
            int count = 0;

            // Accumulate pixel values within the blur length
            for (int k = 0; k < blurLength; k++)
            {
                int nx = x + k;
                if (nx < width)
                {
                    int index = y * rowSize + nx * 3;
                    bSum += pixelData[index];     // Blue
                    gSum += pixelData[index + 1]; // Green
                    rSum += pixelData[index + 2]; // Red
                    count++;
                }
            }

            // Set the blurred pixel to the average of the accumulated values
            int index = y * rowSize + x * 3;
            blurredData[index] = (uint8_t)(bSum / count);     // Blue
            blurredData[index + 1] = (uint8_t)(gSum / count); // Green
            blurredData[index + 2] = (uint8_t)(rSum / count); // Red
        }
    }

    // Copy blurred data back to original pixelData
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int index = y * rowSize + x * 3;
            pixelData[index] = blurredData[index];         // Blue
            pixelData[index + 1] = blurredData[index + 1]; // Green
            pixelData[index + 2] = blurredData[index + 2]; // Red
        }
    }

    free(blurredData); // Free the allocated memory
}

#include <stdint.h>

void motionBlur8(uint8_t *pixelData, int width, int height, int blurLength)
{
    // Temporary array to store blurred pixels
    uint8_t *tempData = (uint8_t *)malloc(width * height * sizeof(uint8_t));

    // Check for allocation failure
    if (tempData == NULL)
    {
        return; // Exit if memory allocation fails
    }

    // Process each row individually to apply a horizontal motion blur effect
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            // Copy the current pixel index
            uint8_t pixelValue = pixelData[y * width + x];

            // Apply blur by copying this pixel index to the right up to blurLength
            for (int k = 0; k < blurLength; k++)
            {
                int targetX = x + k;

                // Ensure we do not go out of bounds
                if (targetX < width)
                {
                    tempData[y * width + targetX] = pixelValue;
                }
            }
        }
    }

    // Copy the blurred data back into the original pixel data
    for (int i = 0; i < width * height; i++)
    {
        pixelData[i] = tempData[i];
    }

    // Free the temporary data array
    free(tempData);
}

void boxBlur(uint8_t *pixelData, int width, int height, int blurSize)
{
    int rowSize = (width * 3 + 3) & ~3;                         // Rows are padded to 4-byte boundaries
    uint8_t *blurredData = (uint8_t *)malloc(height * rowSize); // Allocate memory for the blurred image

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int rSum = 0, gSum = 0, bSum = 0;
            int count = 0;

            // Apply box blur kernel
            for (int dy = -blurSize; dy <= blurSize; dy++)
            {
                for (int dx = -blurSize; dx <= blurSize; dx++)
                {
                    int ny = y + dy;
                    int nx = x + dx;

                    // Ensure we stay within image bounds
                    if (ny >= 0 && ny < height && nx >= 0 && nx < width)
                    {
                        int index = ny * rowSize + nx * 3;
                        bSum += pixelData[index];     // Blue
                        gSum += pixelData[index + 1]; // Green
                        rSum += pixelData[index + 2]; // Red
                        count++;
                    }
                }
            }

            // Calculate the average color values
            int index = y * rowSize + x * 3;
            blurredData[index] = (uint8_t)(bSum / count);     // Blue
            blurredData[index + 1] = (uint8_t)(gSum / count); // Green
            blurredData[index + 2] = (uint8_t)(rSum / count); // Red
        }
    }

    // Copy blurred data back to original pixelData
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int index = y * rowSize + x * 3;
            pixelData[index] = blurredData[index];         // Blue
            pixelData[index + 1] = blurredData[index + 1]; // Green
            pixelData[index + 2] = blurredData[index + 2]; // Red
        }
    }

    free(blurredData); // Free the allocated memory
}