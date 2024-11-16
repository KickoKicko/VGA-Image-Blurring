#include <stdio.h>
#include <stdlib.h>

#pragma pack(push, 1)
typedef struct
{
    unsigned char header[2];      // "BM" signature
    unsigned int fileSize;        // Size of the file
    unsigned short reserved1;     // Reserved field 1
    unsigned short reserved2;     // Reserved field 2
    unsigned int dataOffset;      // Offset to the pixel data
    unsigned int headerSize;      // Header size (usually 40 bytes)
    unsigned int width;           // Width of the image
    unsigned int height;          // Height of the image
    unsigned short planes;        // Number of color planes (always 1)
    unsigned short bpp;           // Bits per pixel (should be 8 for indexed)
    unsigned int compression;     // Compression type (0 for none)
    unsigned int imageSize;       // Image data size (may be 0)
    unsigned int xPixelsPerMeter; // Horizontal resolution (not used here)
    unsigned int yPixelsPerMeter; // Vertical resolution (not used here)
    unsigned int colorsUsed;      // Number of colors used
    unsigned int importantColors; // Important colors (not used here)
} BMPHeader;

#pragma pack(pop)

void convertToRGB(FILE *inFile, FILE *outFile, BMPHeader *bmpHeader)
{
    // Move to the start of the pixel data
    fseek(inFile, bmpHeader->dataOffset, SEEK_SET);

    // Read the color palette (assuming it's 256 colors)
    unsigned char palette[256 * 4]; // 256 colors with 4 bytes each (RGBA)
    fread(palette, sizeof(unsigned char), 256 * 4, inFile);

    // Create a new BMP header for output (with 24-bit color depth)
    BMPHeader newBmpHeader = *bmpHeader;
    newBmpHeader.bpp = 24;                                             // Change to 24 bits per pixel (RGB)
    newBmpHeader.imageSize = bmpHeader->width * bmpHeader->height * 3; // New image size

    // Output the new BMP header
    fwrite(&newBmpHeader, sizeof(BMPHeader), 1, outFile);

    // Loop through each pixel, convert the color index to RGB, and write to the new file
    unsigned char colorIndex;
    unsigned char rgb[3]; // Store RGB values
    for (unsigned int i = 0; i < bmpHeader->width * bmpHeader->height; ++i)
    {
        // Read the color index
        fread(&colorIndex, sizeof(unsigned char), 1, inFile);

        // Get the corresponding RGB values from the palette
        unsigned char *color = &palette[colorIndex * 4];
        rgb[0] = color[2]; // R
        rgb[1] = color[1]; // G
        rgb[2] = color[0]; // B

        // Write the RGB values to the new file
        fwrite(rgb, sizeof(unsigned char), 3, outFile);
    }
}

int main()
{
    FILE *inFile = fopen("flowers.bmp", "rb");
    if (inFile == NULL)
    {
        fprintf(stderr, "Error: Could not open input BMP file.\n");
        return 1;
    }

    // Read the BMP header
    BMPHeader bmpHeader;
    fread(&bmpHeader, sizeof(BMPHeader), 1, inFile);

    // Verify it's an 8-bit indexed BMP (check for valid header values)
    if (bmpHeader.header[0] != 'B' || bmpHeader.header[1] != 'M')
    {
        fprintf(stderr, "Error: Not a valid BMP file.\n");
        fclose(inFile);
        return 1;
    }

    if (bmpHeader.bpp != 8)
    {
        fprintf(stderr, "Error: This program only supports 8-bit indexed BMP files.\n");
        fclose(inFile);
        return 1;
    }

    // Open the output file
    FILE *outFile = fopen("output.bmp", "wb");
    if (outFile == NULL)
    {
        fprintf(stderr, "Error: Could not open output BMP file.\n");
        fclose(inFile);
        return 1;
    }

    // Convert the image to RGB and write it to the new file
    convertToRGB(inFile, outFile, &bmpHeader);

    fclose(inFile);
    fclose(outFile);

    printf("Conversion completed successfully!\n");
    return 0;
}
