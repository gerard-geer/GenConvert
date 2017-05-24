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
#include "rom.h"

// It's a tiny application. Let's go ahead and make our command line args
// global.
char* infile  = NULL;
char* outfile = NULL;
char* from    = NULL;
char* to      = NULL;
bool  v = false;
bool  q = false; 

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
		f = getTypeFromFilename(infile, MD);
		if(v) printf("No starting format given. Inferring \"%s\" from \"%s\"\n",
			  repType(f), infile);
	}
	else
	{
		f = parseType(from);
	}
	if(!to)
	{
		t = getTypeFromFilename(outfile, BIN);
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
	
	// Now that we have an output type, we can create the default output
	// filename. (If necessary).
	if(strlen(outfile)<1)
	{
		strcpy(outfile, "out");
		strcat(outfile, repType(t));
	}

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
	if(checkFilename(&outfile, t)&&!q)
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

