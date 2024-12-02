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

  - The **three leftmost switches** determine the kernel type. These switches represent a binary number in **little-endian order**, where each switch corresponds to a single bit. The kernel types available are as follows:

    - **0**: Box blur
    - **1**: Gaussian blur
    - **2**: Sharpen
    - **3**: Motion blur
    - **4**: Edge detection

  - The kernel size is determined by the three rightmost switches, with the following mapping:

    - **1**: 3x3 matrix
    - **2**: 5x5 matrix
    - **3**: 7x7 matrix
    - and so on (for the ones that have them implemented).

  - **Important Notes**:

    - **Box blur** and **motion blur** have dynamically scaled matrices.
    - **Gaussian blur** has 3x3, 5x5, and 7x7 matrices implemented.
    - **Edge detection** has 3x3 and 5x5 matrices.
    - **Sharpen** does not change depending on the kernel size.

- **Clarification on Kernel Selection**:

  - **Only the three leftmost switches** are used for kernel selection and can only reach a maximum value of 4, as there are only 5 kernels available (0 to 4).

- **Applying Your Selection**

  - After setting the desired switch states, press the **button** to apply the chosen kernel type and size.

- 7-Segment-Display Menu:
  - The display will show the chosen kernel and kernel size in the format:
    > kernelType ---- kernelSize.
  - The LEDs will indicate the progress of applying a kernel, acting as a loading bar.

### Kernel Size Details

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
