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
	const char* filePathListFilePaths[2] = {PSP_IMAGES_FILE_LIST, PS2_IMAGES_FILE_LIST};
	FILE* outputFile = NULL;

	Memory filePathListMemory = { 0 };
	FilePathList filePathList = { 0 };
	Memory image = { 0 };
	u32 nImages = 0;

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
			nImages = GetNumImages(image);
			fprintf(outputFile, "# images: %u. File: %s\n", nImages, filePathList.currentPath);
			free(image.data);
		}
		free(filePathListMemory.data);
	}
	fclose(outputFile);
}
