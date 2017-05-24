/*****************************************************************************
 * File: genconvert.cc
 * Author: Gerard Geer github.com/gerard-geer
 * License: GPL v3
 * Purpose:
 *   Allows one to convert MD roms from one format to another.
 ****************************************************************************/

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <argp.h>
#include <string.h>

// It's a tiny application. Let's go ahead and make our command line args
// global.
char* infile  = NULL;
char* outfile = NULL;
char* from    = NULL;
char* to      = NULL;
bool  v = false;
bool  q = false; 

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
 * Parses a filetype command line argument into an ftype.
 * Parameters:
 *  arg: The argument to parse.
 * Returns:
 *  The appropriate ftype enum value.
 */
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

/**
 * Returns a string representation of a filetype.
 * Parameters:
 *  type: The ftype to represent.
 * Returns:
 *  A string representation of the filetype.
 */
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

/**
 * Gets the filetype from a string.
 * Parameters:
 *  str: The string we need to look at.
 *  dft: The ftype to default to if none is found in the string.
 * Returns:
 *  The filetype.
 */
ftype getType(char *str, ftype dft)
{
	// Get the location of the dot.
	char *aftDot = strrchr(str, '.');
	
	// If that location is null, then there was not type
	// attached to the filename.
	if(!aftDot) return dft;

	// If it wasn't null, then the dot was found.
	return parseType(aftDot+1);
}

/**
 * Checks the outfile filename.
 * Parameters:
 *  outtype: The type we're converting to.
 * Returns:
 *  True if changes were necessary. False otherwise.
 */
bool checkFilename(ftype outtype)
{
	if(getType(outfile,BIN)!=outtype)
	{
		char *n = (char*) calloc(strlen(outfile)+5, sizeof(char));
		n = strcat(n, repType(outtype));
		outfile = n;
		return true;
	}
	return false;
}


/**
 * Opens the rom file and loads the data into a rom instance.
 * Parameters:
 *  filename: The filename of the Rom to open.
 * Returns:
 *  A pointer to a new Rom instance, NULL if unsuccessful.
 */
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

/**
 * Writes a rom to file.
 * Parameters:
 *  filename: The filename of the file to write the ROM instance to.
 *  rom: The rom to save.
 * Returns:
 *  0 if successful, 1 otherwise.
 */
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

/**
 * Converts a Rom assumed to be .MD format to .BIN format.
 * Parameters:
 * 	md: The MD formatted Rom.
 * Returns:
 *  A new Rom in BIN format.
 */
Rom * mdToBin(Rom *md)
{
	// Create a new Rom instance to store the conversion. 
	Rom * bin = (Rom*) malloc(sizeof(Rom));
	bin->data = (uint8_t*) malloc(sizeof(uint8_t)*md->size);
	bin->size = md->size;

	// Get the midpoint of the rom.
	register uint32_t mid = md->size>>1; // Divide by 2 bitwise.
	// Go through and place all the first-half bytes at odd positions.
	for(int i = 1, j = 0; j < mid; i+=2, ++j)
	{
		bin->data[i] = md->data[j];
	}
	// Go through and place all the second-half bytes at even slots.
	for(int i = 0, j = mid; j < md->size; i+=2, ++j)
	{
		bin->data[i] = md->data[j];
	}
	// And just like that we're done.
	return bin;
}

/**
 * Converts a Rom assumed to be .BIN to the .MD format.
 * Parameters:
 *  bin: The rom to convert.
 * Returns:
 *  The Rom, converted to MD format.
 */
Rom * binToMd(Rom *bin)
{
	// Create a new Rom instance to store the result of the conversion.
	Rom * md = (Rom*) malloc(sizeof(Rom));
	md->data = (uint8_t*) malloc(sizeof(uint8_t)*bin->size);
	md->size = bin->size;

	// Get the midpoint of the rom.
	register uint32_t mid = bin->size>>1; // Divide by 2 bitwise.

	// Go through the original rom and divvy it up.
	for(int i = 1, j = 0; j < mid; i+=2, ++j)
	{
		md->data[j] = bin->data[i];
	}
	for(int i = 0, j = mid; j < md->size; i+=2, ++j)
	{
		md->data[j] = bin->data[i];
	}
	// Man, I feel like we could do better with our cache locality.
	return md;
}

/**
 * Chooses the right conversion operation based on the input and output types
 * and performs it.
 * Parameters:
 *  rom: The rom to convert.
 *  from: The ftype being converted from.
 *  to: The ftype being converted to.
 * Returns:
 *  A new, freshly converted rom instance, if from != to. Otherwise the
 *  same instance is returned. (who wants to waste memory?)
 */
Rom * convert(Rom *rom, ftype from, ftype to)
{
	if(v) printf("Converting from \"%s\" to \"%s\"...\n",
		  repType(from), repType(to));

	if(from == SMD || to == SMD)
	{
		printf("SMD is not supported yet.\n");
		return NULL;
	}

	else if(from == to)
	{
		if(v) printf("No conversion necessary.\n");
		return rom;
	}

	else if(from == MD && to == BIN ) return mdToBin(rom);
	else if(from == BIN && to == MD ) return binToMd(rom);
	else return NULL;
}

/**
 * The argp parsing callback.
 */
static int parse_opt(int key, char *arg, struct argp_state *state)
{
	switch(key)
	{
		case 'o': // Output file.
			free(outfile); // Clear the default value.
			outfile = arg;
			break;
		case 'f': // "FROM" file format.
			from = arg;
			break;
		case 't': // "TO" file format.
			free(to); // Clear the default value.
			to = arg;
			break;
		case 'v': // verbose?
			v = true & !q; // Make sure we didn't sneak it in after a -q.
			break;
		case 'q': // Quiet?
			q = true;
			v = false;
			break;
		case ARGP_KEY_ARG: // Argument specified without an option.
			infile = arg;
			break;
	}
	return 0;
}

/**
 * The entry point into the program.
 */
int main(int argc, char **argv)
{
	// First things first: let's set some darn default values.
	outfile = (char*) malloc(sizeof(char)*9);
	strcpy(outfile, "out.bin");

	// Now we define the arguments we care about for argp.
	struct argp_option options[] = {
	{"outfile", 'o', "FILE", 0, "Output file. Defaults to <out.bin>"},
	{"from",'f',"FROM", 0,
	"Original format. [.bin, .md, .smd]. (Omission tries to infer.)"},
	{"to",'t',"TO",0,
	"The target format. Choose from [.bin, .md, .smd]. (Omission infers.)"},
	{"verbose", 'v', 0, 0, "Print out extra information."},
	{"quiet", 'q', 0, 0, 
	"Make this darn program shut up for once. Overrides v --verbose"},
		NULL // argp uses the ol' run-to-null philosophy I've learned.
	};
	struct argp argp = {options, parse_opt, 0,0};

	// Go ahead and parse the command line.
	argp_parse(&argp, argc, argv, 0,0,0);

	// Make sure we got an input file.
	if(!infile&&!q) { printf("Error: no input files.\n"); return 1; }

	// Okay, it's time to do some inferencing if we didn't get any types.
	ftype f, t;
	if(!from)
	{
		f = getType(infile, MD);
		if(v) printf("No starting format given. Inferring \"%s\" from \"%s\"\n",
			  repType(f), infile);
	}
	else
	{
		f = parseType(from);
	}
	if(!to)
	{
		t = getType(outfile, BIN);
		if(v) printf("No target format given. Inferring \"%s\" from \"%s\"\n",
			  repType(t), outfile);
	}
	else
	{
		t = parseType(to);
	}

	// Make sure that worked.
	if(f==ERROR&&!q) {printf("Error: Invalid source filetype.\n"); return 2;}
	if(t==ERROR&&!q) {printf("Error: Invalid target filetype.\n"); return 3;}
	
	// Now we can load the ROM.
	Rom *in = loadROM(infile);

	if(q)

	if(v) printf("Input file: \"%s\"\nSize: %d bytes\nFormat: \"%s\"",
		  infile, in->size, repType(f));
	

	// Make sure it was loaded.
	if(!in){printf("Error: Unable to open in file \"%s\"\n",infile); return 4;}

	// Go ahead and try to convert.
	Rom *out = convert(in, f, t);
	if(v) printf("output file: \"%s\"\nSize: %d bytes\nFormat: \"%s\"",
		  outfile, out->size, repType(f));

	// Make sure that worked.
	if(!out &&!q)
	{
		printf("Error: Conversion not possible. Check types.\n"); return 5;
	}

	// We need to make sure the output filename is going to be accurate.
	if(checkFilename(t)&&!q)
	{
		printf("Warning: filename didn't match target format. Modifying...\n");
	}

	// Alrighty now we've just got to save.
	ioerror e = saveROM(outfile, out);

	// Don't need the product anymore either.
	free(out->data); free(out);
	
	if(e==COULD_NOT_OPEN_FILE&&!q)
	{
		printf("Error: Could not open \"%s\" for writing.\n", outfile);
		return 6;
	}
	else if(e==SIZE_DISCREPANCY&&!q)
	{
		printf("Error: Size discrepancy when writing. \"%s\" may be incorrect."
		, outfile);
		return 7;
	}
	else if(v) printf("File saved successfully.\n");

	printf("Ran!\n");
	return 0;
}

