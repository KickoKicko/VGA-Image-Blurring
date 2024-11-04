#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

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
    uint32_t biSize;         // Size of this header (40 bytes)
    int32_t biWidth;         // Width of the bitmap in pixels
    int32_t biHeight;        // Height of the bitmap in pixels
    uint16_t biPlanes;       // Number of color planes (must be 1)
    uint16_t biBitCount;     // Number of bits per pixel (should be 24 for RGB)
    uint32_t biCompression;  // Compression type (0 = none)
    uint32_t biSizeImage;    // Size of the image data (can be 0 for uncompressed)
    int32_t biXPelsPerMeter; // Horizontal resolution in pixels per meter
    int32_t biYPelsPerMeter; // Vertical resolution in pixels per meter
    uint32_t biClrUsed;      // Number of colors in the palette (0 = use all)
    uint32_t biClrImportant; // Number of important colors (0 = all)
} BMPInfoHeader;
#pragma pack(pop)

void createWhiteBMP(const char *filename, int width, int height)
{
    BMPHeader header;
    BMPInfoHeader infoHeader;

    // Set up BMP header
    header.bfType = 0x4D42;                                                                                                    // 'BM'
    header.bfSize = sizeof(BMPHeader) + sizeof(BMPInfoHeader) + (width * height * 3) + (height * ((4 - (width * 3) % 4) % 4)); // Add padding
    header.bfReserved1 = 0;
    header.bfReserved2 = 0;
    header.bfOffBits = sizeof(BMPHeader) + sizeof(BMPInfoHeader);

    // Set up BMP info header
    infoHeader.biSize = sizeof(BMPInfoHeader);
    infoHeader.biWidth = width;
    infoHeader.biHeight = height;
    infoHeader.biPlanes = 1;
    infoHeader.biBitCount = 24;     // 24 bits per pixel
    infoHeader.biCompression = 0;   // No compression
    infoHeader.biSizeImage = 0;     // Can be 0 for uncompressed
    infoHeader.biXPelsPerMeter = 0; // 72 DPI
    infoHeader.biYPelsPerMeter = 0; // 72 DPI
    infoHeader.biClrUsed = 0;       // No palette
    infoHeader.biClrImportant = 0;  // All colors are important

    // Create pixel data
    int rowSize = (width * 3 + 3) & ~3; // Rows are padded to 4-byte boundaries
    uint8_t *pixelData = (uint8_t *)malloc(rowSize * height);

    // Fill the pixel data with white (255, 255, 255) in BGR format
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int index = y * rowSize + x * 3;

            if (x < width / 2)
            {
                pixelData[index] = 255;   // Blue
                pixelData[index + 1] = 0; // Green
                pixelData[index + 2] = 0; // Red
            }
            else
            {
                pixelData[index] = 0;       // Blue
                pixelData[index + 1] = 255; // Green
                pixelData[index + 2] = 0;   // Red
            }
        }
    }

    // Write the BMP file
    FILE *file = fopen(filename, "wb");
    if (!file)
    {
        perror("Failed to open file for writing");
        free(pixelData);
        return;
    }

    fwrite(&header, sizeof(BMPHeader), 1, file);
    fwrite(&infoHeader, sizeof(BMPInfoHeader), 1, file);
    fwrite(pixelData, sizeof(uint8_t), rowSize * height, file);

    fclose(file);
    free(pixelData);
    printf("Created white BMP file: %s\n", filename);
}

int main()
{
    const char *filename = "white_image.bmp";
    int width = 800;  // Width of the image
    int height = 600; // Height of the image

    createWhiteBMP(filename, width, height);
    return 0;
}
