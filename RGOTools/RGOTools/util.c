/*  RGO Patching Tools Version 1.0.0
 *  util.c
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

/* Description: Some useful utility functions and types
 * that do not correspond to any outward facing functionality. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

/* Getting a list of files in a directory is platform specific :( */
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

/* Loads the entirety of a file into dynamic memory.
 * Parameter: filePath - the file to load in.
 * Return: Memory struct containing the loaded in file. */
Memory LoadFile(const char* filePath)
{
	FILE* pFile = NULL;
	u32 fileSize = 0;
	u8* pData = NULL;
	Memory ret = { 0 };

	/* Open file */
	pFile = FOpenMustSucceed(filePath, "rb");

	/* Get file size */
	fseek(pFile, 0, SEEK_END);
	fileSize = ftell(pFile);
	fseek(pFile, 0, SEEK_SET);
	fileSize -= ftell(pFile);

	/* Read in file */
	pData = MallocMustSucceed(fileSize);

	fread(pData, fileSize, 1, pFile);
	ret.data = pData;
	ret.size = fileSize;

	fclose(pFile);
	return ret;
}

/* Using a loaded in list of files, create a usuable list of file path strings.
 * Parameter: file - the loaded in list of files.
              Note that this function modifies the underlying data
			  so it is suitable to be referenced by a FileList. The Memory still maintains responsibility
			  over the actual data though, so it still must free its data.
 * Return: a FileList containing a list of pointers to file where each file path string starts. */
FileList InitFileList(Memory file)
{
	FileList ret = { 0 };
	u32 i = 0;
	u32 pathsSet = 1; /* First path is always at the start of file.data */

	/* Find how many file paths there are */
	for (i = 0; i < file.size; ++i)
	{
		if (file.data[i] == '\n')
		{
			/* Turn newline to null terminator so it marks the end of the file path string */
			file.data[i] = '\0';
			++ret.nFiles;
		}
	}
	ret.paths = MallocMustSucceed(sizeof(const char*) * ret.nFiles);

	/* Set up pointers to each file path */
	ret.paths[0] = (char*)file.data;

	for (i = 0; i < file.size; ++i)
	{
		if (file.data[i] == '\0')
		{
			ret.paths[pathsSet] = (char*)&file.data[i + 1];
			++pathsSet;
			if (pathsSet >= ret.nFiles)
			{
				break;
			}
		}
	}

	return ret;
}

/* Reads a 32-bit little endian value.
 * Parameter: data - the data from which to read.
 * Return: the read 32-bit little endian value. */
u32 LittleEndianRead32(const u8* data)
{
	return data[0] + (data[1] << 8) + (data[2] << 16) + (data[3] << 24);
}

/* A wrapper for malloc that terminates the program on failure.
 * Parameter: size - the number of bytes to allocate.
 * Return: a pointer to the allocated memory */
void* MallocMustSucceed(size_t size)
{
	void* result = malloc(size);
	if (!result && size)
	{
		printf("ERROR: Allocation of %llu bytes failed\n", size);
		exit(-1);
	}
	return result;
}

/* A wrapper for fopen that terminates the program on failure.
 * Parameter: path - the path to the file to open
 * Parameter: mode - the mode to open the file in */
FILE* FOpenMustSucceed(const char* path, const char* mode)
{
	FILE* file = fopen(path, mode);
	if (!file)
	{
		printf("ERROR: Could not open file at %s\n", path);
		exit(-1);
	}
	return file;
}

/* Generates the list of file names for the images in RGO PSP.
 * Parameter: path - the path where the file names should be written. */
void GeneratePSPImageFileList(void)
{
	FILE* pFile = NULL;
	u32 i = 0;

	pFile = FOpenMustSucceed(PSP_IMAGES_FILE_LIST, "wb");

	for (i = PSP_IMAGES_START_NUM; i <= PSP_IMAGES_END_NUM; ++i)
	{
		fprintf(pFile, "%s%u\n", PSP_IMAGES_DIRECTORY, i);
	}

	fclose(pFile);
}

/* Generates the list of file names for the images in RGO PS2.
 * Parameter: path - the path where the file names should be written.
 * Return: whether or not the file list was successfully generated. */
#ifdef _WIN32
void GeneratePS2ImageFileList(void)
{
	const char* directories[] =
	{
		PS2_IMAGES_BK_DIRECTORY,
		PS2_IMAGES_BU_DIRECTORY,
		PS2_IMAGES_FC_DIRECTORY
	};

	FILE* pFile = NULL;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATAA findData = { 0 };
	char buffer[MAX_PATH] = { 0 };

	u32 i = 0;

	pFile = FOpenMustSucceed(PS2_IMAGES_FILE_LIST, "wb");

	for (i = 0; i < NUM_ELEMENTS(directories); ++i)
	{
		strcpy(buffer, directories[i]);
		strcat(buffer, "/*");

		hFind = FindFirstFileA(buffer, &findData); /* Start search and discard . */

		if (hFind == INVALID_HANDLE_VALUE)
		{
			printf("ERROR: Could not find directory %s\n", directories[i]);
			fclose(pFile);
			return;
		}

		FindNextFileA(hFind, &findData); /* discard .. */

		while (FindNextFileA(hFind, &findData))
		{
			fprintf(pFile, "%s%s\n", directories[i], findData.cFileName);
		}

		if (GetLastError() != ERROR_NO_MORE_FILES)
		{
			printf("ERROR: Failed to get all files in directory %s\n", directories[i]);
			FindClose(hFind);
			fclose(pFile);
		}
		FindClose(hFind);
	}

	fclose(pFile);
}
#else
void GeneratePS2ImageFileList(void)
{
	printf("ERROR: GeneratePS2ImageFileList is only supported on Windows.\n"
		"Make sure that %s has not been deleted, moved, or renamed.\n", PS2_IMAGES_FILE_LIST);
}
#endif
