/*  RGO Patching Tools Version 1.0.0
 *  image.c
 *  Copyright (C) 2022 TimepieceMaster
 *
 *  This file is part of the RGO Patching Tools.
 *
 *  The RGO Patching Tools is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  The RGO Patching Tools is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with the RGO Patching Tools. If not, see <https://www.gnu.org/licenses/>. */

#include <stdio.h>
#include <string.h>
#include "util.h"
#include "image.h"

/* Determine the number of images in an RGO image archive file. */
u32 GetNumImages(const Memory imageData)
{
	/* The easiest way to determine the number of images is to determine the
	 * number of palettes, as each image gets its own palette. */

	const u32 maxPotentialPalettes = imageData.size / PALETTE_NUM_BYTES;
	u32 nPalettes = 1; /* There's always at least 1 palette at the start of the file */

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

u8* GetPalette(const Memory imageData, const u32 paletteIndex)
{
	return &imageData.data[paletteIndex * PALETTE_NUM_BYTES];
}
