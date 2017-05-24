/*****************************************************************************
 * File: conv.h
 * Author: Gerard Geer github.com/gerard-geer
 * License: GPL v3
 * Purpose:
 *   Defines conversion subroutines.
 ****************************************************************************/

#ifndef CONV_H
#define CONV_H

#include <stdlib.h>
#include <stdio.h>
#include "rom.h"

/**
 * Converts a assumed to be .MD format to .BIN format.
 * Parameters:
 * 	md: The MD formatted Rom.
 * Returns:
 *  A new in BIN format.
 */
void mdToBin(Rom *md, Rom *bin);

/**
 * Converts a assumed to be .SMD format to .BIN format.
 * Parameters:
 * 	md: The MD formatted Rom.
 * Returns:
 *  A new in BIN format.
 */
void smdToBin(Rom *smd, Rom *bin);

/**
 * Converts a assumed to be .BIN to the .MD format.
 * Parameters:
 *  bin: The rom to convert.
 * Returns:
 *  The Rom, converted to MD format.
 */
void binToMd(Rom *bin, Rom *md);

/**
 * Converts a assumed to be .BIN to the .SMD format.
 * Parameters:
 *  bin: The rom to convert.
 * Returns:
 *  The Rom, converted to MD format.
 */
void binToSmd(Rom *bin, Rom *smd);

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
Rom * convert(Rom *rom, ftype from, ftype to);

#endif // CONV_H
