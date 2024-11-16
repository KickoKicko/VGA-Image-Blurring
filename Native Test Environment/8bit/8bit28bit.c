#include <stdio.h>
#include <stdlib.h>

// Define BMP header sizes
#define HEADER_SIZE 54
#define PALETTE_SIZE 1024

// Function to convert 8-bit to 3-bit (or 2-bit for blue)
unsigned char convert_channel(unsigned char value, int bits)
{
    return value >> (8 - bits);
}

// Function to calculate row padding
int calculate_padding(int width, int bytes_per_pixel)
{
    return (4 - (width * bytes_per_pixel) % 4) % 4;
}

// Function to process BMP and save the output
void process_bmp(const char *input_file, const char *output_file)
{
    FILE *input = fopen(input_file, "rb");
    if (!input)
    {
        perror("Error opening input file");
        return;
    }

    FILE *output = fopen(output_file, "wb");
    if (!output)
    {
        perror("Error opening output file");
        fclose(input);
        return;
    }

    // Read and write BMP header
    unsigned char header[HEADER_SIZE];
    fread(header, sizeof(unsigned char), HEADER_SIZE, input);
    fwrite(header, sizeof(unsigned char), HEADER_SIZE, output);

    // Extract width and height from the BMP header
    int width = *(int *)&header[18];
    int height = *(int *)&header[22];
    int bytes_per_pixel = 1; // For 8-bit BMP files
    int row_padding = calculate_padding(width, bytes_per_pixel);

    // Read and skip the palette
    unsigned char palette[PALETTE_SIZE];
    fread(palette, sizeof(unsigned char), PALETTE_SIZE, input);

    // Extract pixel data
    unsigned char *row = (unsigned char *)malloc(width * sizeof(unsigned char));
    unsigned char *new_row = (unsigned char *)malloc(width * sizeof(unsigned char));
    if (!row || !new_row)
    {
        perror("Memory allocation failed");
        free(row);
        free(new_row);
        fclose(input);
        fclose(output);
        return;
    }

    for (int y = 0; y < height; y++)
    {
        // Read a row of pixel data (excluding padding)
        fread(row, sizeof(unsigned char), width, input);
        fseek(input, row_padding, SEEK_CUR); // Skip padding

        // Convert each pixel in the row
        for (int x = 0; x < width; x++)
        {
            unsigned char index = row[x];
            unsigned char blue = palette[index * 4];
            unsigned char green = palette[index * 4 + 1];
            unsigned char red = palette[index * 4 + 2];

            // Convert to 3-bit red, 3-bit green, 2-bit blue
            unsigned char red_3bit = convert_channel(red, 3);
            unsigned char green_3bit = convert_channel(green, 3);
            unsigned char blue_2bit = convert_channel(blue, 2);

            // Combine into rrrgggbb format
            new_row[x] = (red_3bit << 5) | (green_3bit << 2) | blue_2bit;
        }

        // Write the new row to the output file
        fwrite(new_row, sizeof(unsigned char), width, output);

        // Add padding to the output file
        for (int p = 0; p < row_padding; p++)
        {
            fputc(0, output);
        }
    }

    // Clean up
    free(row);
    free(new_row);
    fclose(input);
    fclose(output);

    printf("Conversion complete! Output saved to %s\n", output_file);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: %s <input bmp> <output bmp>\n", argv[0]);
        return 1;
    }

    process_bmp(argv[1], argv[2]);

    return 0;
}
