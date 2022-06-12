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

void TestImageGetImageInfo(const char* outputPath)
{
	const char* filePathListFilePaths[2] = { PSP_IMAGES_FILE_LIST, PS2_IMAGES_FILE_LIST };
	FILE* outputFile = NULL;

	Memory filePathListMemory = { 0 };
	FilePathList filePathList = { 0 };
	Memory image = { 0 };
	ImageInfo imageInfo = { 0 };

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
			imageInfo = GetImageInfo(image);
			fprintf(outputFile, "# images: %u. Has MAP Data: %u. File: %s\n", imageInfo.nImages, imageInfo.hasMAPData, filePathList.currentPath);
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
	ImageInfo imageInfo = { 0 };
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
			imageInfo = GetImageInfo(image);
			for(j = 0; j < imageInfo.nImages; ++j)
			{
				header = GetImageHeader(image, imageInfo, j);
				nSubfiles = LittleEndianRead32(header);
				fprintf(outputFile, "Image %u/%u. # subfiles: %u. File: %s\n", j + 1, imageInfo.nImages, nSubfiles, filePathList.currentPath);
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
	ImageInfo imageInfo = { 0 };
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
		fclose(outputFile);
		return;
	}

	imageInfo = GetImageInfo(image);
	currentHeader = GetImageHeader(image, imageInfo, 0);
	imagePlatform = GetImagePlatform(currentHeader);
	decompressedImage = DecompressImage(currentHeader, imagePlatform);
	if (!decompressedImage.data)
	{
		free(image.data);
		fclose(outputFile);
		return;

	}
	fwrite(decompressedImage.data, decompressedImage.size, 1, outputFile);
	free(decompressedImage.data);

	for (i = 1; i < imageInfo.nImages; ++i)
	{
		currentHeader = GetNextImageHeader(currentHeader);
		decompressedImage = DecompressImage(currentHeader, imagePlatform);
		if (!decompressedImage.data)
		{
			free(image.data);
			fclose(outputFile);
			return;
		}
		fwrite(decompressedImage.data, decompressedImage.size, 1, outputFile);
		free(decompressedImage.data);
	}
}

void TestExtractAllImages(void)
{
	char outputPath[1024] = { 0 };
	Memory filePathListMemory = { 0 };
	FilePathList filePathList = { 0 };
	u32 nImages = 0;
	u32* imageWidths = NULL;
	u32 i = 0;

	filePathListMemory = LoadFile(TEST_IMAGE_EXTRACT_ALL_IMAGES_STANDARD_WIDTH_FILE_LIST);
	if (!filePathListMemory.data)
	{
		LOAD_FILE_FAIL_MESSAGE(TEST_IMAGE_EXTRACT_ALL_IMAGES_STANDARD_WIDTH_FILE_LIST);
		return;
	}
	filePathList = InitFilePathList(filePathListMemory);

	while (GetNextFilePath(&filePathList))
	{
		printf("%s\n", filePathList.currentPath);
		GenerateExtractAllImagesOutputPath(filePathList.currentPath, outputPath);
		ConvertRGOImageToPNGAll(filePathList.currentPath, outputPath, 0);
	}
	free(filePathListMemory.data);

	filePathListMemory = LoadFile(TEST_IMAGE_EXTRACT_ALL_IMAGES_NONSTANDARD_WIDTH_FILE_LIST);
	if (!filePathListMemory.data)
	{
		LOAD_FILE_FAIL_MESSAGE(TEST_IMAGE_EXTRACT_ALL_IMAGES_NONSTANDARD_WIDTH_FILE_LIST);
		return;
	}
	filePathList = InitFilePathList(filePathListMemory);
	filePathList.currentPath = filePathListMemory.data;
	while (filePathList.memoryPos < filePathListMemory.size)
	{
		for (; filePathListMemory.data[filePathList.memoryPos] != ' '; ++filePathList.memoryPos);
		filePathListMemory.data[filePathList.memoryPos] = '\0';
		++filePathList.memoryPos;

		sscanf(&filePathListMemory.data[filePathList.memoryPos], "%u", &nImages);
		imageWidths = malloc(sizeof(u32) * nImages);
		if (!imageWidths)
		{
			free(filePathListMemory.data);
			return;
		}
		for (; filePathListMemory.data[filePathList.memoryPos] != ' '; ++filePathList.memoryPos);
		++filePathList.memoryPos;

		for (i = 0; i < nImages; ++i)
		{
			sscanf(&filePathListMemory.data[filePathList.memoryPos], "%u", &imageWidths[i]);
			if (i < nImages - 1)
			{
				for (; filePathListMemory.data[filePathList.memoryPos] != ' '; ++filePathList.memoryPos);
				++filePathList.memoryPos;
			}
		}

		printf("%s\n", filePathList.currentPath);
		GenerateExtractAllImagesOutputPath(filePathList.currentPath, outputPath);
		ConvertRGOImageToPNGAll(filePathList.currentPath, outputPath, imageWidths);
		free(imageWidths);

		for (; filePathListMemory.data[filePathList.memoryPos] != '\n'; ++filePathList.memoryPos);
		++filePathList.memoryPos;
		filePathList.currentPath = &filePathListMemory.data[filePathList.memoryPos];
	}
	free(filePathListMemory.data);
}

void GenerateExtractAllImagesOutputPath(const char* inputPath, char* outputPath)
{
	const char* fileNameStart = 0;
	size_t fileNameLength = 0;
	char* foundChr = NULL;
	char filename[1024] = { 0 };

	foundChr = strchr(inputPath, '/');
	if (!foundChr)
	{
		fileNameStart = inputPath;
	}
	else
	{
		fileNameStart = foundChr + 1;
	}
	foundChr = strrchr(inputPath, '.');
	if (!foundChr)
	{
		fileNameLength = strlen(fileNameStart);
	}
	else
	{
		fileNameLength = foundChr - fileNameStart;
	}
	memcpy(filename, fileNameStart, fileNameLength);
	filename[fileNameLength] = '\0';
	strcat(filename, ".png");

	sprintf(outputPath, TEST_IMAGE_EXTRACTED_IMAGES_FOLDER);
	strcat(outputPath, filename);
}
