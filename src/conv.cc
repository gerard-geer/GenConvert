/*****************************************************************************
 * File: conv.cc
 * Author: Gerard Geer github.com/gerard-geer
 * License: GPL v3
 * Purpose:
 *   Defines conversion subroutines.
 ****************************************************************************/

#include "conv.h"

void mdToBin(Rom *md, Rom *bin)
{
	// Initialize the target instance.
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
}

void binToMd(Rom *bin, Rom *md)
{
	// Initialize the target instance.
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
}

Rom * convert(Rom *rom, ftype from, ftype to)
{
	if(from == SMD || to == SMD)
	{
		printf("SMD is not supported yet.\n");
		return NULL;
	}

	if(from == to) return rom;

	Rom *converted = (Rom*) malloc(sizeof(Rom));

	if(from == MD && to == BIN ) mdToBin(rom, converted);
	else if(from == BIN && to == MD ) binToMd(rom, converted);
	else free(converted);

	return converted;
}

