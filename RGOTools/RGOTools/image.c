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
#include <stdlib.h>
#include "OutsideCode/zlib/zlib.h"
#include "util.h"
#include "image.h"
#include "PS2Decompress.h"

#define DEFAULT_PALETTE_NUM_BYTES 1024
#define MAP_DATA_SIGNATURE_LITTLE_ENDIAN 0x0050414D /* In big endian, it would be "MAP" but this form is more convenient */
#define DEFAULT_HEADER_OFFSET 0x1800

static u32 GetNumBytesToNextHeader(const u8* currentHeader, u32 nSubfiles);
static bool32 DecompressPSPSubimage(u8* src, u32 srcSize, u8* dst, u32 dstSize);

/* Determine the number of images in an RGO image archive file. */
NumImagesInfo GetNumImages(const Memory imageData)
{
	/* The easiest way to determine the number of images is to determine the
	 * number of palettes, as each image gets its own palette. */

	NumImagesInfo ret =
	{
		.nImages = 1, /* There's always at least 1 palette at the start of the file */
		.lastPaletteSize = DEFAULT_PALETTE_NUM_BYTES,
		.hasDefaultHeaderOffset = TRUE
	};
	u32 color = 0;

	/* Most images only have one palette. In that case, either the first byte after
	 * the palette is MAP data, or it is padding. */
	color = LittleEndianRead32(&imageData.data[DEFAULT_PALETTE_NUM_BYTES]);
	if (color == 0)
	{
		/* Could be padding, but often the first color of a palette is transparent.
		 * Check the second color. */
		memcpy(&color, &imageData.data[DEFAULT_PALETTE_NUM_BYTES + 4], 4);
		if (color == 0)
		{
			/* Definitely padding. There's only 1 palette */
			return ret;
		}
		/* Not padding. First color was just transparent. */
	}
	else if (color == MAP_DATA_SIGNATURE_LITTLE_ENDIAN)
	{
		/* MAP data. There's only 1 palette */
		return ret;
	}
	else if (color > 0 && color < 256)
	{
		/* Image header. In practice only one file has the image header
		 * immediately follow when there's only one palette (PSP image 2536). */
		ret.hasDefaultHeaderOffset = FALSE;
		return ret;
	}

	/* More than one palette. The palette data will end when the first image header
	 * data appears (images containing MAP data only ever have 1 palette), which could
	 * happen on any 16 byte boundary
	 * (In practice any 1024 byte boundary in all files except PSP image 2533 and PS2 image pt_omake).
	 * This means the final palette will have a multiple of 4 colors, but could have less than 256 colors. */
	ret.hasDefaultHeaderOffset = FALSE;
	ret.lastPaletteSize = 0;
	while (1)
	{
		/* Get the current color */
		color = LittleEndianRead32(&imageData.data[ret.nImages * DEFAULT_PALETTE_NUM_BYTES + ret.lastPaletteSize]);

		/* No palette color will be non-zero in the red byte but zero in all other bytes.
		 * However, the first 4 bytes of the image header always has this property.
		 * (With one exception, see GetNumBytesToNextHeader).
		 * However, that exception does not occur in the first image header, so we're fine. */
		if (color > 0 && color < 256)
		{
			/* Image header. There are no more palettes. */
			break;
		}
		/* Still in palette data. */
		ret.lastPaletteSize += 16;
		if (ret.lastPaletteSize == DEFAULT_PALETTE_NUM_BYTES)
		{
			ret.lastPaletteSize = 0;
			++ret.nImages;
		}
	}
	if (ret.lastPaletteSize != 0)
	{
		++ret.nImages;
	}
	else
	{
		ret.lastPaletteSize = DEFAULT_PALETTE_NUM_BYTES;
	}
	return ret;
}

u8* GetPalette(u8* imageData, u32 index)
{
	return &imageData[index * DEFAULT_PALETTE_NUM_BYTES];
}

u8* GetImageHeader(Memory imageData, NumImagesInfo numImagesInfo, u32 index)
{
	u8* ret = NULL;
	u32 headerCheck = 0;

	u32 headersFoundAfterFirst = 0;
	u32 nSubfiles = 0;

	/* Get first header */
	if (numImagesInfo.hasDefaultHeaderOffset)
	{
		ret = &imageData.data[DEFAULT_HEADER_OFFSET];
	}
	else
	{
		ret = &imageData.data[DEFAULT_PALETTE_NUM_BYTES * (numImagesInfo.nImages - 1) + numImagesInfo.lastPaletteSize];
	}

	/* Advance to the header we need */
	for (; headersFoundAfterFirst < index; ++headersFoundAfterFirst)
	{
		ret = GetNextImageHeader(ret);
	}
	return ret;
}

u8* GetNextImageHeader(u8* currentHeader)
{
	u32 nSubfiles = 0;
	nSubfiles = LittleEndianRead32(currentHeader);
	return &currentHeader[GetNumBytesToNextHeader(currentHeader, nSubfiles)];
}

static u32 GetNumBytesToNextHeader(const u8* currentHeader, u32 nSubfiles)
{
	u32 imageDataSize = 0;
	u32 checkPadding = 0;
	u32 checkForChecksum[4] = { 0 };
	u32 i = 0;

	/* Calculate how many bytes are in the image.
	 * The last 4 bytes in the header give the size of the image data, but there
	 * is also a 16-byte checksum which must be 16-byte-aligned that isn't counted in that value. */
	imageDataSize = LittleEndianRead32(&currentHeader[(nSubfiles + 1) * 4]);
	imageDataSize += (imageDataSize % 16) + CHECKSUM_LENGTH;

	/* Round up the image size to the nearest kilobyte */
	if (imageDataSize % 1024 != 0)
	{
		imageDataSize = (imageDataSize / 1024 + 1) * 1024;
	}

	/* There may still be some number of kilobytes of additional padding before the next header. */
	memcpy(&checkPadding, &currentHeader[imageDataSize], 4);
	while (checkPadding == 0)
	{
		/* Even if it is zero, it is possible for there to be an image
		 * with zero subimages and thus no actual data.
		 * So it's not padding, but it's actually the header, conceptually speaking.
		 * If this is the case, the 16-bytes preceding it will be the checksum of the
		 * previous image (some non-zero value). In practice, this only happens
		 * with one image (PSP image 2530) for which the image was removed but not its palette. */
		memcpy(checkForChecksum, &currentHeader[imageDataSize - CHECKSUM_LENGTH], CHECKSUM_LENGTH);
		for (i = 0; i < NUM_ELEMENTS(checkForChecksum); ++i)
		{
			if (checkForChecksum[i] != 0)
			{
				/* Checksum found. It's an empty image header. */
				return imageDataSize;
			}
		}
		/* Actually padding. Move to the next kilobyte. */
		imageDataSize += 1024;
		memcpy(&checkPadding, &currentHeader[imageDataSize], 4);
	}
	return imageDataSize;
}

Platform GetImagePlatform(const u8* header)
{
	/* The PSP version puts the compressed image data at a 16-byte alignment and so has 12 bytes of
	/* padding after the start of the subfile since the first 4 bytes of a subfile contain the uncompressed
	 * size of the subfile and subfiles are also 16-byte aligned. The PS2 version does not have this padding. */
	u32 firstSubfileOffset = 0;
	u32 checkPadding = 0;
	firstSubfileOffset = LittleEndianRead32(&header[4]);
	memcpy(&checkPadding, &header[firstSubfileOffset + 4], 4);
	if (checkPadding != 0)
	{
		return PLATFORM_PS2;
	}
	else
	{
		return PLATFORM_PSP;
	}
}

Memory DecompressImage(u8* header, Platform platform)
{
	Memory ret = { 0 };
	u32 nSubfiles = 0;
	u32 decompressedSize = 0;
	u32 compressedSize = 0;
	u32 decompressedBytesRemaining = 0;
	u32 currentHeaderSubfileOffset = 0;
	u32 nextHeaderSubfileOffset = 0;
	u8* compressedDataInPtr = NULL;
	u8* decompressedDataOutPtr = NULL;

	u32 i = 0;

	nSubfiles = LittleEndianRead32(header);

	/* Handle exception case where image has zero subfiles (PSP image 2530) */
	if (nSubfiles == 0)
	{
		return ret;
	}

	/* Allocate memory to hold the uncompressed image */
	for (i = 0; i < nSubfiles; ++i)
	{
		currentHeaderSubfileOffset = LittleEndianRead32(&header[(i + 1) * 4]);
		decompressedSize += LittleEndianRead32(&header[currentHeaderSubfileOffset]);
	}
	ret.data = malloc(decompressedSize);
	if (!ret.data)
	{
		return ret;
	}
	ret.size = decompressedSize;

	/* Decompress the subfiles and put them contiguously in the allocated memory */
	decompressedBytesRemaining = ret.size;
	currentHeaderSubfileOffset = LittleEndianRead32(&header[4]);
	for (i = 0; i < nSubfiles; ++i)
	{
		nextHeaderSubfileOffset = LittleEndianRead32(&header[(i + 2) * 4]);
		decompressedSize = LittleEndianRead32(&header[currentHeaderSubfileOffset]);
		compressedSize = nextHeaderSubfileOffset - currentHeaderSubfileOffset;
		decompressedDataOutPtr = &ret.data[ret.size - decompressedBytesRemaining];

		/* The compressed data is offset differently from the
		 * start of a subfile between the PS2 and PSP, and the PSP uses gzip for compression, while
		 * the PS2 version uses a custom algorithm */
		if (platform == PLATFORM_PS2)
		{
			compressedDataInPtr = &header[currentHeaderSubfileOffset];
			DecompressPS2Subimage(compressedDataInPtr, compressedSize, decompressedDataOutPtr, decompressedSize);
		}
		else
		{
			compressedDataInPtr = &header[currentHeaderSubfileOffset + 16];
			if (!DecompressPSPSubimage(compressedDataInPtr, compressedSize, decompressedDataOutPtr, decompressedSize))
			{
				free(ret.data);
				ret.data = NULL;
				return ret;
			}
		}
		decompressedBytesRemaining -= decompressedSize;
		currentHeaderSubfileOffset = nextHeaderSubfileOffset;
	}

	return ret;
}

static bool32 DecompressPSPSubimage(u8* src, u32 srcSize, u8* dst, u32 dstSize)
{
	z_stream zStream = { 0 };

	zStream.next_in = src;
	zStream.avail_in = srcSize;
	zStream.next_out = dst;
	zStream.avail_out = dstSize;

	if (inflateInit2(&zStream, 16 + MAX_WBITS) != Z_OK)
	{
		return FALSE;
	}
	if (inflate(&zStream, Z_FINISH) != Z_STREAM_END)
	{
		return FALSE;
	}
	if (inflateEnd(&zStream) != Z_OK)
	{
		return FALSE;
	}

	return TRUE;
}
