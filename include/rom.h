/******************************************************************************
 * File: rom.h
 * Author: Gerard Geer
 * Purpose:
 *  Defines datatypes and utility functions for genconvert.
 *****************************************************************************/

#ifndef ROM_H
#define ROM_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/**
 * This enum enumerates the filetypes we can work with.
 */
typedef enum filetype_enum
{
	BIN,MD,SMD,ERROR
} ftype;

/**
 * This enum just talks about IO errors.
 */
typedef enum ioerror_enum
{
	NO_ERROR=0, COULD_NOT_OPEN_FILE, SIZE_DISCREPANCY
} ioerror;

/**
 * This struct represents a rom in terms of size and contents.
 */
typedef struct rom_struct
{
	// The size of the rom.
	uint32_t size;
	// The data of the rom.
	uint8_t *data;
} Rom;

/**
 * Returns a string representation of a filetype.
 * Parameters:
 *  type: The ftype to represent.
 * Returns:
 *  A string representation of the filetype.
 */
const char *repType(ftype type);

/**
 * Parses a filetype command line argument into an ftype.
 * Parameters:
 *  arg: The argument to parse.
 * Returns:
 *  The appropriate ftype enum value.
 */
ftype parseType(char * type);

/**
 * Gets the filetype from a string.
 * Parameters:
 *  str: The string we need to look at.
 *  dft: The ftype to default to if none is found in the string.
 * Returns:
 *  The filetype.
 */
ftype getTypeFromFilename(char *str, ftype dft);

/**
 * Checks the outfile filename.
 * Parameters:
 *  outtype: The type we're converting to.
 * Returns:
 *  True if changes were necessary. False otherwise.
 */
bool checkFilename(char ** name, ftype type);

/**
 * Opens the rom file and loads the data into a rom instance.
 * Parameters:
 *  filename: The filename of the Rom to open.
 * Returns:
 *  A pointer to a new Rom instance, NULL if unsuccessful.
 */
Rom * loadROM(const char* filename);

/**
 * Writes a rom to file.
 * Parameters:
 *  filename: The filename of the file to write the ROM instance to.
 *  rom: The rom to save.
 * Returns:
 *  0 if successful, 1 otherwise.
 */
ioerror saveROM(const char* filename, Rom* rom);

#endif // ROM_H
