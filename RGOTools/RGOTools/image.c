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
#include "OutsideCode/libpng/png.h"
#include "util.h"
#include "image.h"

#define DEFAULT_PALETTE_NUM_BYTES 1024
#define MAP_DATA_SIGNATURE_LITTLE_ENDIAN 0x0050414D /* In big endian, it would be "MAP" but this form is more convenient */
#define DEFAULT_HEADER_OFFSET 0x1800
#define TILE_WIDTH 16
#define TILE_HEIGHT 8
#define TILE_SIZE (TILE_WIDTH * TILE_HEIGHT)
#define PSP_IMAGE_WIDTH 512
#define PS2_IMAGE_WIDTH 640
#define TILES_PER_ROW (PSP_IMAGE_WIDTH / TILE_WIDTH)
#define TILE_ROW_SIZE (PSP_IMAGE_WIDTH * TILE_HEIGHT)

static u32 GetNumBytesToNextHeader(const u8* currentHeader, u32 nSubfiles);
static bool32 DecompressPSPSubimage(u8* src, u32 srcSize, u8* dst, u32 dstSize);
static void DecompressPS2Subimage(u8* src, u8* dst, u32 numBytesToDecompress);

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
			compressedDataInPtr = &header[currentHeaderSubfileOffset + 4];
			DecompressPS2Subimage(compressedDataInPtr, decompressedDataOutPtr, decompressedSize);
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

static void DecompressPS2Subimage(u8* src, u8* dst, u32 numBytesToDecompress)
{
	u8 circularBuf[0x1000] = { 0 };
	u32 bufPos = 0xFEE;
	u32 backReferenceLength = 0;
	u32 backReferenceOffset = 0;
	u32 currentBackReferenceByte = 0;
	u32 encodingTypeBitField = 0;
	u32 i = 0;
	while (numBytesToDecompress != 0)
	{
		encodingTypeBitField >>= 1;
		if (!(encodingTypeBitField & 0x100))
		{
			/* Get encoding information on the next 8 sections, one bit per section.
			 * One means the next byte is read in directly.
			 * Zero means the next group of bytes is taken from the circular buffer.
			 * The circular buffer contains the last 4 kilobytes of uncompressed data.
			 * that have been read in. */
			encodingTypeBitField = *src | 0xFF00;
			++src;
		}
		if (encodingTypeBitField & 0x1)
		{
			circularBuf[bufPos] = *src;
			*dst = *src;
			bufPos = (bufPos + 1) & 0xFFF;
			++src;
			++dst;
			--numBytesToDecompress;
		}
		else
		{
			backReferenceLength = (src[1] & 0xF) + 3; /* Back-reference must be at least 3 bytes long */
			backReferenceOffset = *src | ((src[1] & 0xF0) << 4);
			src += 2;
			for (i = 0; i < backReferenceLength; ++i)
			{
				currentBackReferenceByte = circularBuf[(backReferenceOffset + i) & 0xFFF];
				circularBuf[bufPos] = currentBackReferenceByte;
				*dst = currentBackReferenceByte;
				bufPos = (bufPos + 1) & 0xFFF;
				++dst;
			}
			numBytesToDecompress -= backReferenceLength;
		}
	}
}

/* On PSP, the pixels in an image aren't given in linear order, but instead are
 * grouped into 16 x 8 tiles. */
Memory TiledToLinear(Memory tiledImage)
{
	u32 currentRowInTile = 0;
	u32 currentTileInRow = 0;
	u8* src = NULL;
	u8* dst = NULL;
	Memory ret = { 0 };

	u32 i = 0;

	ret.data = malloc(tiledImage.size);
	if (!ret.data)
	{
		return ret;
	}
	ret.size = tiledImage.size;

	src = tiledImage.data;
	dst = ret.data;
	for (i = 0; i < tiledImage.size / TILE_WIDTH; ++i)
	{
		memcpy(dst, src, TILE_WIDTH);
		++currentTileInRow;
		src += TILE_SIZE;
		dst += TILE_WIDTH;
		if (currentTileInRow == TILES_PER_ROW)
		{
			src -= TILE_ROW_SIZE;
			src += TILE_WIDTH;
			currentTileInRow = 0;
			++currentRowInTile;
			if (currentRowInTile == TILE_HEIGHT)
			{
				src -= TILE_SIZE;
				src += TILE_ROW_SIZE;
				currentRowInTile = 0;
			}
		}
	}
	return ret;
}

/* On PS2, the palette is not given in order, and instead
 * are grouped into 32-color groups where colors 8-15 and 16-24
 * are swapped. Additionally alpha ranges from 0x0 to 0x80, so
* it needs to be converted to range from 0x0 + 0xFF */
void CorrectPS2Palette(u32* palette, u32 nColors)
{
	const u32 colorGroupSize = 32;
	const u32 colorGroupNumBytes = 32 * 4;
	u32 i = 0;
	u32 temp[32] = { 0 };
	for (i = 0; i < nColors / colorGroupSize; ++i)
	{
		memcpy(temp, &palette[i * 32 + 16], 32);
		memcpy(&palette[i * 32 + 16], &palette[i * 32 + 8], 32);
		memcpy(&palette[i * 32 + 8], temp, 32);
	}
	if (nColors % (colorGroupSize) >= 24)
	{
		memcpy(temp, &palette[i * 32 + 16], 32);
		memcpy(&palette[i * 32 + 16], &palette[i * 32 + 8], 32);
		memcpy(&palette[i * 32 + 8], temp, 32);
	}
	for (i = 0; i < nColors; ++i)
	{
		if (palette[i] & 0x80000000)
		{
			palette[i] |= 0xFF000000;
		}
		else
		{
			palette[i] += palette[i] & 0xFF000000;
		}
	}
}

bool32 WriteToPNG(Memory decompressedImage, u32* palette, u32 width, u32 height, const char* outputPath)
{
	FILE* outputFile = NULL;
	png_structp pngWritePtr = NULL;
	png_infop pngInfoPtr = NULL;
	u8** rowPointers = NULL;

	u32* finalImageData = NULL;
	u32 i = 0;

	/* setup output file, memory, and libpng */
	outputFile = fopen(outputPath, "wb");
	if (!outputFile)
	{
		return FALSE;
	}
	finalImageData = malloc(decompressedImage.size * 4);
	if (!finalImageData)
	{
		fclose(outputFile);
		return FALSE;
	}
	rowPointers = malloc(sizeof(u8*) * height);
	if (!rowPointers)
	{
		fclose(outputFile);
		free(finalImageData);
		return FALSE;
	}
	pngWritePtr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!pngWritePtr)
	{
		fclose(outputFile);
		free(finalImageData);
		free(rowPointers);
		return FALSE;
	}
	pngInfoPtr = png_create_info_struct(pngWritePtr);
	if (!pngInfoPtr)
	{
		fclose(outputFile);
		free(finalImageData);
		free(rowPointers);
		png_destroy_write_struct(&pngWritePtr, NULL);
		return FALSE;
	}
	if (setjmp(png_jmpbuf(pngWritePtr)))
	{
		fclose(outputFile);
		free(finalImageData);
		free(rowPointers);
		png_destroy_write_struct(&pngWritePtr, &pngInfoPtr);
		return FALSE;
	}
	png_init_io(pngWritePtr, outputFile);
	png_set_IHDR(pngWritePtr, pngInfoPtr, width, height, 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_write_info(pngWritePtr, pngInfoPtr);

	/* Prepare image data for writing as PNG and then write to the PNG */
	for (i = 0; i < decompressedImage.size; ++i)
	{
		finalImageData[i] = palette[decompressedImage.data[i]];
	}
	for (i = 0; i < height; ++i)
	{
		rowPointers[i] = (u8*)(&finalImageData[width * i]);
	}
	png_write_image(pngWritePtr, rowPointers);
	png_write_end(pngWritePtr, NULL);

	/* Cleanup */
	fclose(outputFile);
	free(finalImageData);
	free(rowPointers);
	png_destroy_write_struct(&pngWritePtr, &pngInfoPtr);
	return TRUE;
}

bool32 ConvertRGOImageToPNG(Memory image, NumImagesInfo numImagesInfo, u8* header, u32 imageIndex, const char* imageOutputPath)
{
	u32* palette = NULL;
	Platform platform = 0;
	Memory decompressedImage = { 0 };
	Memory untiledImage = { 0 };
	u32 width = 0;
	u32 height = 0;

	palette = (u32*)GetPalette(image.data, imageIndex);
	platform = GetImagePlatform(header);
	decompressedImage = DecompressImage(header, platform);
	if (!decompressedImage.data)
	{
		return FALSE;
	}
	if (platform == PLATFORM_PS2)
	{
		if ((numImagesInfo.nImages - 1) == imageIndex)
		{
			CorrectPS2Palette(palette, numImagesInfo.lastPaletteSize / 4);
		}
		else
		{
			CorrectPS2Palette(palette, DEFAULT_PALETTE_NUM_BYTES / 4);
		}
		width = PS2_IMAGE_WIDTH;
	}
	else if (platform == PLATFORM_PSP)
	{
		untiledImage = TiledToLinear(decompressedImage);
		if (!untiledImage.data)
		{
			free(decompressedImage.data);
			return FALSE;
		}
		free(decompressedImage.data);
		decompressedImage = untiledImage;

		width = PSP_IMAGE_WIDTH;
	}
	height = decompressedImage.size / width;
	if (!WriteToPNG(decompressedImage, palette, width, height, imageOutputPath))
	{
		free(decompressedImage.data);
		return FALSE;
	}

	free(decompressedImage.data);
	return TRUE;
}

void ConvertRGOImageToPNGAll(const char* inputPath, const char* outputPath)
{
	Memory image = { 0 };
	NumImagesInfo numImagesInfo = { 0 };
	u8* header = NULL;
	u32 i = 0;
	char* outputPathMultipleFiles = NULL;
	u32 appendLocation = 0;
	char* appendPtr = NULL;

	image = LoadFile(inputPath);
	if (!image.data)
	{
		LOAD_FILE_FAIL_MESSAGE(inputPath);
		return;
	}
	numImagesInfo = GetNumImages(image);
	header = GetImageHeader(image, numImagesInfo, 0);
	if (!ConvertRGOImageToPNG(image, numImagesInfo, header, 0, outputPath))
	{
		printf("Failed to extract image 0 in %s\n", inputPath);
	}
	if (numImagesInfo.nImages > 1)
	{
		outputPathMultipleFiles = malloc(strlen(outputPath) + 256); /* Just something reasonably big enough */
		if (!outputPathMultipleFiles)
		{
			free(image.data);
			return;
		}
		appendPtr = strrchr(outputPath, '.');
		if (!appendPtr)
		{
			appendLocation = (u32)strlen(outputPath);
		}
		else
		{
			appendLocation = (u32)(appendPtr - outputPath);
		}
		memcpy(outputPathMultipleFiles, outputPath, appendLocation);
	}
	for (i = 1; i < numImagesInfo.nImages; ++i)
	{
		sprintf(&outputPathMultipleFiles[appendLocation], "_%u", i);
		strcat(outputPathMultipleFiles, &outputPath[appendLocation]);
		header = GetNextImageHeader(header);
		if (!ConvertRGOImageToPNG(image, numImagesInfo, header, i, outputPathMultipleFiles))
		{
			printf("Failed to extract image %u in %s\n", i, inputPath);
		}
	}
	free(image.data);
	free(outputPathMultipleFiles);
}
