/* Filename: image.c
 * Author: TimepieceMaster
 * Description: Used to convert images from RGO's image format to .png files,
 * as well as convert .png files to images that can be used in RGO. */

#include <stdio.h>
#include <string.h>
#include "util.h"
#include "image.h"

/* Determine the number of palettes in an RGO image 
 * Parameter: imageData - an RGO image.
 * Return: the number of palettes in the image */
u32 GetNumPalettes(const Memory imageData)
{
	const u32 maxPotentialPalettes = imageData.size / PALETTE_NUM_BYTES;
	u32 nPalettes = 1; /* There's always at least 1 palette at the start of the file */

	/* Look for palettes following the first one */
	for (; nPalettes < maxPotentialPalettes; ++nPalettes)
	{
		/* Get what would be the first color in the palette */
		const u8* pPalette = &imageData.data[nPalettes * PALETTE_NUM_BYTES];
		u32 color = LittleEndianRead32(pPalette);

		/* No palette color will be zero in the alpha byte but non-zero in other bytes.
		 * However, the signature of MAP data and the first 4 bytes of the image header
		 * always has this property. */
		if (!(color & 0xFF000000) && (color & 0x00FFFFFF))
		{
			/* Non-palette data. There are no more palettes. */
			return nPalettes;
		}
		/* All zeroes or has both alpha and color. If it's all zeroes
		 * it could be padding, but transparent is often the first color
		 * in the palette. Check the second color. */
		else if (color == 0)
		{
			memcpy(&color, &pPalette[4], 4);
			if (color == 0)
			{
				/* Still zero. Definitely padding. There are no more palettes. */
				return nPalettes;
			}
			/* It is a palette, just the first color was transparent. */
		}
		/* It is a palette. */
	}

	return nPalettes;
}
