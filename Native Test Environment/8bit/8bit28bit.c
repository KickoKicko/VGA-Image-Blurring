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

    // Read and skip the palette
    unsigned char palette[PALETTE_SIZE];
    fread(palette, sizeof(unsigned char), PALETTE_SIZE, input);

    // Extract pixel data
    fseek(input, 0, SEEK_END);
    long file_size = ftell(input);
    long pixel_data_size = file_size - HEADER_SIZE - PALETTE_SIZE;
    fseek(input, HEADER_SIZE + PALETTE_SIZE, SEEK_SET);

    unsigned char *pixel_data = (unsigned char *)malloc(pixel_data_size);
    if (!pixel_data)
    {
        perror("Memory allocation failed");
        fclose(input);
        fclose(output);
        return;
    }
    fread(pixel_data, sizeof(unsigned char), pixel_data_size, input);

    // Convert pixels
    unsigned char red, green, blue, new_value;
    unsigned char *new_pixel_data = (unsigned char *)malloc(pixel_data_size);
    if (!new_pixel_data)
    {
        perror("Memory allocation failed");
        free(pixel_data);
        fclose(input);
        fclose(output);
        return;
    }

    for (long i = 0; i < pixel_data_size; i++)
    {
        unsigned char index = pixel_data[i];
        blue = palette[index * 4];
        green = palette[index * 4 + 1];
        red = palette[index * 4 + 2];

        // Convert to 3-bit red, 3-bit green, 2-bit blue
        unsigned char red_3bit = convert_channel(red, 3);
        unsigned char green_3bit = convert_channel(green, 3);
        unsigned char blue_2bit = convert_channel(blue, 2);

        // Combine into rrrgggbb format
        new_value = (red_3bit << 5) | (green_3bit << 2) | blue_2bit;
        new_pixel_data[i] = new_value;
    }

    // Write new pixel data to output file
    fwrite(new_pixel_data, sizeof(unsigned char), pixel_data_size, output);

    // Clean up
    free(pixel_data);
    free(new_pixel_data);
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
