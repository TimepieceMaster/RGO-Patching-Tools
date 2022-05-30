/*  RGO Patching Tools Version 1.0.0
 *  test.c
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
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "image.h"
#include "test.h"

void TestUtilLoadFile(const char* inputPath, const char* outputPath)
{
	FILE* outputFile = NULL;
	Memory loadedFile = { 0 };

	loadedFile = LoadFile(inputPath);
	if (!loadedFile.data)
	{
		LOAD_FILE_FAIL_MESSAGE(inputPath);
		return;
	}

	outputFile = fopen(outputPath, "wb");
	if (!outputFile)
	{
		FOPEN_FAIL_MESSAGE(outputPath);
		free(loadedFile.data);
		return;
	}
	fwrite(loadedFile.data, loadedFile.size, 1, outputFile);
	free(loadedFile.data);
	fclose(outputFile);
}

void TestUtilFilePathList(const char* inputPath, const char* outputPath)
{
	FILE* outputFile = NULL;
	Memory loadedFile = { 0 };
	FilePathList filePathList = { 0 };

	loadedFile = LoadFile(inputPath);
	if (!loadedFile.data)
	{
		LOAD_FILE_FAIL_MESSAGE(inputPath);
		return;
	}
	filePathList = InitFilePathList(loadedFile);

	outputFile = fopen(outputPath, "wb");
	if (!outputFile)
	{
		FOPEN_FAIL_MESSAGE(outputPath);
		free(loadedFile.data);
		return;
	}

	while (GetNextFilePath(&filePathList))
	{
		fprintf(outputFile, "%s\n", filePathList.currentPath);
	}
	free(loadedFile.data);
	fclose(outputFile);
}

void TestImageGetNumImages(const char* outputPath)
{
	const char* filePathListFilePaths[2] = { PSP_IMAGES_FILE_LIST, PS2_IMAGES_FILE_LIST };
	FILE* outputFile = NULL;

	Memory filePathListMemory = { 0 };
	FilePathList filePathList = { 0 };
	Memory image = { 0 };
	NumImagesInfo numImagesInfo = { 0 };

	u32 i = 0;

	outputFile = fopen(outputPath, "wb");
	if (!outputFile)
	{
		FOPEN_FAIL_MESSAGE(outputPath);
		return;
	}

	/* Get the number of palettes from each image */
	for (i = 0; i < NUM_ELEMENTS(filePathListFilePaths); ++i)
	{
		filePathListMemory = LoadFile(filePathListFilePaths[i]);
		if (!filePathListMemory.data)
		{
			LOAD_FILE_FAIL_MESSAGE(filePathListFilePaths[i]);
			fclose(outputFile);
			return;
		}
		filePathList = InitFilePathList(filePathListMemory);

		while (GetNextFilePath(&filePathList))
		{
			image = LoadFile(filePathList.currentPath);
			if (!image.data)
			{
				LOAD_FILE_FAIL_MESSAGE(filePathList.currentPath);
				free(filePathListMemory.data);
				fclose(outputFile);
				return;
			}
			numImagesInfo = GetNumImages(image);
			fprintf(outputFile, "# images: %u. File: %s\n", numImagesInfo.nImages, filePathList.currentPath);
			free(image.data);
		}
		free(filePathListMemory.data);
	}
	fclose(outputFile);
}

void TestImageGetImageHeader(const char* outputPath)
{
	const char* filePathListFilePaths[2] = { PSP_IMAGES_FILE_LIST, PS2_IMAGES_FILE_LIST };
	FILE* outputFile = NULL;

	Memory filePathListMemory = { 0 };
	FilePathList filePathList = { 0 };
	Memory image = { 0 };
	NumImagesInfo numImagesInfo = { 0 };
	u8* header = NULL;
	u32 nSubfiles = 0;

	u32 i = 0;
	u32 j = 0;

	outputFile = fopen(outputPath, "wb");
	if (!outputFile)
	{
		FOPEN_FAIL_MESSAGE(outputPath);
		return;
	}

	/* Get all image headers in every image and output the number of subFiles */
	for (i = 0; i < NUM_ELEMENTS(filePathListFilePaths); ++i)
	{
		filePathListMemory = LoadFile(filePathListFilePaths[i]);
		if (!filePathListMemory.data)
		{
			LOAD_FILE_FAIL_MESSAGE(filePathListFilePaths[i]);
			fclose(outputFile);
			return;
		}
		filePathList = InitFilePathList(filePathListMemory);

		while (GetNextFilePath(&filePathList))
		{
			image = LoadFile(filePathList.currentPath);
			if (!image.data)
			{
				LOAD_FILE_FAIL_MESSAGE(filePathList.currentPath);
				free(filePathListMemory.data);
				fclose(outputFile);
				return;
			}
			numImagesInfo = GetNumImages(image);
			for(j = 0; j < numImagesInfo.nImages; ++j)
			{
				header = GetImageHeader(image, numImagesInfo, j);
				nSubfiles = LittleEndianRead32(header);
				fprintf(outputFile, "Image %u/%u. # subfiles: %u. File: %s\n", j + 1, numImagesInfo.nImages, nSubfiles, filePathList.currentPath);
			}
			free(image.data);
		}
		free(filePathListMemory.data);
	}
	fclose(outputFile);
}

void TestImageDecompressSingleImage(const char* inputPath, const char* outputPath)
{
	FILE* outputFile = NULL;
	Memory image = { 0 };
	NumImagesInfo numImagesInfo = { 0 };
	u8* currentHeader = NULL;
	Platform imagePlatform = 0;
	Memory decompressedImage = { 0 };
	u32 i = 0;

	outputFile = fopen(outputPath, "wb");
	if (!outputFile)
	{
		return;
	}
	image = LoadFile(inputPath);
	if (!image.data)
	{
		goto errorLoadFile;
	}

	numImagesInfo = GetNumImages(image);
	currentHeader = GetImageHeader(image, numImagesInfo, 0);
	imagePlatform = GetImagePlatform(currentHeader);
	decompressedImage = DecompressImage(currentHeader, imagePlatform);
	if (!decompressedImage.data)
	{
		goto errorDecompressImage;
	}
	fwrite(decompressedImage.data, decompressedImage.size, 1, outputFile);
	free(decompressedImage.data);

	for (i = 1; i < numImagesInfo.nImages; ++i)
	{
		currentHeader = GetNextImageHeader(currentHeader);
		decompressedImage = DecompressImage(currentHeader, imagePlatform);
		if (!decompressedImage.data)
		{
			goto errorDecompressImage;
		}
		fwrite(decompressedImage.data, decompressedImage.size, 1, outputFile);
		free(decompressedImage.data);
	}

errorDecompressImage:
	free(image.data);
errorLoadFile:
	fclose(outputFile);
}

/* This test is just to make sure DecompressImage doesn't crash or fail to decompress on any image.
 * Correctness of the decompression will be evaluated elsewhere. */
void TestImageDecompressAllImages(const char* outputPath)
{
	const char* filePathListFilePaths[2] = { PSP_IMAGES_FILE_LIST, PS2_IMAGES_FILE_LIST };
	FILE* outputFile = NULL;

	Memory filePathListMemory = { 0 };
	FilePathList filePathList = { 0 };
	Memory image = { 0 };
	NumImagesInfo numImagesInfo = { 0 };
	u8* currentHeader = NULL;
	Platform imagePlatform = 0;
	Memory decompressedImage = { 0 };

	u32 i = 0;
	u32 j = 0;

	outputFile = fopen(outputPath, "wb");
	if (!outputFile)
	{
		FOPEN_FAIL_MESSAGE(outputPath);
		return;
	}

	/* Get all image headers in every image and output the number of subFiles */
	for (i = 0; i < NUM_ELEMENTS(filePathListFilePaths); ++i)
	{
		filePathListMemory = LoadFile(filePathListFilePaths[i]);
		if (!filePathListMemory.data)
		{
			LOAD_FILE_FAIL_MESSAGE(filePathListFilePaths[i]);
			fclose(outputFile);
			return;
		}
		filePathList = InitFilePathList(filePathListMemory);

		while (GetNextFilePath(&filePathList))
		{
			image = LoadFile(filePathList.currentPath);
			if (!image.data)
			{
				LOAD_FILE_FAIL_MESSAGE(filePathList.currentPath);
				free(filePathListMemory.data);
				fclose(outputFile);
				return;
			}
			numImagesInfo = GetNumImages(image);
			currentHeader = GetImageHeader(image, numImagesInfo, 0);
			imagePlatform = GetImagePlatform(currentHeader);
			decompressedImage = DecompressImage(currentHeader, imagePlatform);
			if (!decompressedImage.data)
			{
				fprintf(outputFile, "Image %u/%u. File: %s. FAILED\n", j + 1, numImagesInfo.nImages, filePathList.currentPath);
			}
			fprintf(outputFile, "Image %u/%u. File: %s. SUCCEEDED.\n", j + 1, numImagesInfo.nImages, filePathList.currentPath);
			free(decompressedImage.data);

			numImagesInfo = GetNumImages(image);
			for (j = 1; j < numImagesInfo.nImages; ++j)
			{
				currentHeader = GetImageHeader(image, numImagesInfo, 0);
				decompressedImage = DecompressImage(currentHeader, imagePlatform);
				if (!decompressedImage.data)
				{
					fprintf(outputFile, "Image %u/%u. File: %s. FAILED\n", j + 1, numImagesInfo.nImages, filePathList.currentPath);
				}
				fprintf(outputFile, "Image %u/%u. File: %s. SUCCEEDED.\n", j + 1, numImagesInfo.nImages, filePathList.currentPath);
				free(decompressedImage.data);
			}
			free(image.data);
		}
		free(filePathListMemory.data);
	}
	fclose(outputFile);
	return;
}

void TestExtractAllImages(void)
{
	const char* filePathListFilePaths[2] = { PSP_IMAGES_FILE_LIST, PS2_IMAGES_FILE_LIST };
	char outputPath[1024] = { 0 };
	char filename[1024] = { 0 };
	Memory filePathListMemory = { 0 };
	FilePathList filePathList = { 0 };
	char* fileNameStart = 0;
	u32 fileNameLength = 0;
	char* lastOccurance = NULL;
	u32 i = 0;

	for (i = 0; i < NUM_ELEMENTS(filePathListFilePaths); ++i)
	{
		filePathListMemory = LoadFile(filePathListFilePaths[i]);
		if (!filePathListMemory.data)
		{
			LOAD_FILE_FAIL_MESSAGE(filePathListFilePaths[i]);
			return;
		}
		filePathList = InitFilePathList(filePathListMemory);

		while (GetNextFilePath(&filePathList))
		{
			lastOccurance = strrchr(filePathList.currentPath, '/');
			if (!lastOccurance)
			{
				fileNameStart = filePathList.currentPath;
			}
			else
			{
				fileNameStart = lastOccurance + 1;
			}
			lastOccurance = strrchr(filePathList.currentPath, '.');
			if (!lastOccurance)
			{
				fileNameLength = strlen(fileNameStart);
			}
			else
			{
				fileNameLength = lastOccurance - fileNameStart;
			}
			memcpy(filename, fileNameStart, fileNameLength);
			filename[fileNameLength] = '\0';
			strcat(filename, ".png");

			sprintf(outputPath, "TestFiles/Results/ExtractedImages/");
			strcat(outputPath, filename);

			printf("%s\n", filePathList.currentPath);
			ConvertRGOImageToPNGAll(filePathList.currentPath, outputPath);
		}
	}
}
