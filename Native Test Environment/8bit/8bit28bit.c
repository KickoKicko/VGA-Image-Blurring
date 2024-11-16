#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

// Function to scale 8-bit values to desired ranges
uint8_t scale_color(uint8_t color, int bits)
{
    return (color * ((1 << bits) - 1)) / 255;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: %s <input.bmp> <output.bmp>\n", argv[0]);
        return 1;
    }

    FILE *inputFile = fopen(argv[1], "rb");
    if (!inputFile)
    {
        perror("Error opening input file");
        return 1;
    }

    FILE *outputFile = fopen(argv[2], "wb");
    if (!outputFile)
    {
        perror("Error opening output file");
        fclose(inputFile);
        return 1;
    }

    // BMP header and color palette buffers
    uint8_t header[40];
    uint8_t palette[1024];

    // Read and write the header
    fread(header, sizeof(uint8_t), 40, inputFile);
    fwrite(header, sizeof(uint8_t), 40, outputFile);

    // Read and write the palette
    fread(palette, sizeof(uint8_t), 1024, inputFile);
    fwrite(palette, sizeof(uint8_t), 1024, outputFile);

    // Process the pixel data
    uint8_t pixel;
    while (fread(&pixel, sizeof(uint8_t), 1, inputFile) == 1)
    {
        // Convert the palette index (8-bit) to the desired format
        uint8_t r = scale_color((pixel >> 5) & 0x07, 3); // Top 3 bits for red
        uint8_t g = scale_color((pixel >> 2) & 0x07, 3); // Middle 3 bits for green
        uint8_t b = scale_color(pixel & 0x03, 2);        // Bottom 2 bits for blue

        // Combine into a single byte: RRR GGG BB
        uint8_t converted_pixel = (r << 5) | (g << 2) | b;

        fwrite(&converted_pixel, sizeof(uint8_t), 1, outputFile);
    }

    fclose(inputFile);
    fclose(outputFile);

    printf("Conversion complete. Output saved to %s\n", argv[2]);
    return 0;
}
