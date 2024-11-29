# Define variables for compiler and flags
CC = gcc
CFLAGS = -Wall -O2

# Define the source file, executable name, and input/output files
SRC = convertTo8Bit.c
EXE = convertTo8Bit.exe
OUTPUT = output.bmp
PIXELDATA_H = pixeldata.h
PIXELDATA_NAME = output_bmp
STATICPIXELDATA_NAME = static_output_bmp
STATIC_PIXELDATA_H = staticPixeldata.h
BOARD_DIR = ../Board

# Default target
all: run

# Compile the C program
$(EXE): $(SRC)
	$(CC) $(CFLAGS) -o $(EXE) $(SRC)

# Run the executable with the input file
run: $(EXE)
	./$(EXE) $(INPUT)
	xxd -i -n output_bmp -s 0x48A $(OUTPUT) > $(BOARD_DIR)/$(PIXELDATA_H)
	xxd -i -n static_output_bmp -s 0x48A $(OUTPUT) > $(BOARD_DIR)/$(STATIC_PIXELDATA_H)

# Clean up generated files
clean:
	rm -f $(EXE) $(PIXELDATA_H) $(STATIC_PIXELDATA_H) $(OUTPUT)
