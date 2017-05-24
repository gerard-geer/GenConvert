/******************************************************************************
 * File: rom.cc
 * Author: Gerard Geer
 * Purpose:
 *  Defines datatypes and utility functions for genconvert.
 *****************************************************************************/

 #include "rom.h"

const char *repType(ftype type)
{
	switch(type) {
		case BIN:   return ".bin";
		case MD:    return ".md";
		case SMD:   return ".smd";
		case ERROR: return ".???";
		default:    return ".???";
	}
}

ftype parseType(char * type)
{
	// First up: we need to capitalize the string to do case-insensitive
	// comparison.

	// Darn it there's no platform-independent way to do case
	// independent comparison between strings.

	// Copy the string so we don't modify the original.
	char *t = (char*)malloc(sizeof(char)*strlen(type));
	t = strcpy(t, type);

	// We need to check to see if there's a dot.
	if(strrchr(t,'.')) ++t;

	// We need to first get the length of the string.
	register int len = strlen(t);
	// Whether or not the character is within range to change.
	register uint8_t change;

	// Now we need to go and convert all of them.
	for(int i = 0; i < len; ++i)
	{
		// Make sure it's a letter that can be capitalized, e.g. lowecase.
		change = t[i] > 0x60; // 0x61 = 'a'

		// The change flag is set when the character is within operable
		// bounds. What needs to be changed based on that flag is the 6th
		// bit of the character, and to zero. Therefore we need to and it
		// with a value that keeps all other bits the same, but flattens
		// bit 5. So we shift over the flag 5 bits, and take the opposite
		// of it. 
		t[i] &= ~(change<<5);
		// Wait did you notice the lack of branching? :)
	}
	// Now we need to actually do strcmp.
	if(!strcmp(t, "BIN")) return BIN;
	else if(!strcmp(t, "MD")) return MD;
	else if(!strcmp(t, "SMD")) return SMD;
	else return ERROR;
}

ftype getTypeFromFilename(char *str, ftype dft)
{
	// Get the location of the dot.
	char *aftDot = strrchr(str, '.');
	
	// If that location is null, then there was not type
	// attached to the filename.
	if(!aftDot) return dft;

	// If it wasn't null, then the dot was found.
	return parseType(aftDot+1);
}

bool checkFilename(char ** name, ftype type)
{
	if(getTypeFromFilename(*name,BIN)!=type)
	{
		char *n = (char*) calloc(strlen(*name)+5, sizeof(char));
		n = strcat(n, repType(type));
		*name = n;
		return true;
	}
	return false;
}

Rom * loadROM(const char* filename)
{
	// Open the file.
	FILE *f = fopen(filename, "rb");

	// Error, you say?
	if(!f) return NULL;

	// First let's create the new rom instance.
	Rom *rom = (Rom*) malloc(sizeof(Rom));

	// Get the file size.
	fseek(f, 0, SEEK_END);
	rom->size = ftell(f);
	fseek(f, 0, SEEK_SET);

	// Allocate the memory to store the rom data.
	rom->data = (uint8_t*) malloc(sizeof(uint8_t)*rom->size);

	// Let the OS take care of chunking.
	int32_t read = fread(rom->data, sizeof(uint8_t), rom->size, f);

	// Close the file.
	fclose(f);

	// Return the rom.
	return rom;
}

ioerror saveROM(const char* filename, Rom* rom)
{
	// Try to open the file to write bytes.
	FILE *f = fopen(filename, "wb");

	// Make sure we got the handle.
	if(!f) return COULD_NOT_OPEN_FILE;

	// Write the data out.
	int32_t numWritten = fwrite(rom->data, sizeof(uint8_t), rom->size, f);

	// Close the file.
	fclose(f);

	// Make sure the number of items written is correct.
	return (numWritten == rom->size)?NO_ERROR:SIZE_DISCREPANCY;
}

