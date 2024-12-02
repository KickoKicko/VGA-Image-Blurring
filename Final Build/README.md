# Instructions on How to Use Our Program

## Overview

This project was developed using a **DE10-Lite board** equipped with the **DTEK-V toolboard**. Ensure you are in an environment where the **DTEK-V toolchain** is available.

## Running the Code with Our Selected and Prepared Image

> This is the most straightforward way to run our project—simply use the prepared image by following these steps.

1. **Navigate to the Appropriate Directory**

   - Ensure you are in the `../Final Build/Board` directory.

2. **Compile the Program**

   - Run the `make` command.

3. **Execute the Program**
   - Run `dtekv-run main.bin`.

### Using the Switches and Button

- The **switches** are used to select the kernel type and kernel size:

  - The **two leftmost switches** determine the kernel type. These switches represent a binary number in **little-endian order**, where each switch corresponds to a single bit.

  - The **three rightmost switches** determine the kernel size.

- **Applying Your Selection**
  - After setting the desired switch states, press the **button** to apply the chosen kernel type and size.

## Downloading and Using Another Image

> **Note**: This step is optional. You only need to follow this section if you want to try using an image other than the one we prepared.

1. **Prepare the Environment**

   - This process does not run on the board. It converts the input image to the required 24-bit BMP format and embeds it into the binary file. Ensure you are in an environment capable of running `gcc`.

     > **Note**: The `xxd` command is needed for the conversion code to work. Make sure it is available in your environment.

2. **Navigate to the Appropriate Directory**

   - Move to the `../Final Build/Convert` directory.

3. **Run the Conversion Command**
   - Use the command: `make INPUT=<inputBMP.bmp>`
     > **Note**: The exact `make` command may vary depending on your terminal or other factors, such as the version or configuration of `gcc` being used.
   - After this, the program can be compiled and run on the board as before, and the new image will be used.

- **Image Requirements**

  - The input image must be a 24-bit BMP file. The simplest way to create one is to open the image in **Microsoft Paint** and save it as a `.BMP` file.
  - Some 24-bit BMP files have been provided in the `Convert` directory for testing purposes.
