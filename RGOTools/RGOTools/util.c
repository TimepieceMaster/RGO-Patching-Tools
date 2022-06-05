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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

/* Getting a list of files in a directory is platform specific :( */
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#define PSP_IMAGES_DIRECTORY "TestFiles/PSPImages/BIN/"
#define PSP_IMAGES_START_NUM 824
#define PSP_IMAGES_END_NUM 2539
#define PS2_IMAGES_BK_DIRECTORY "TestFiles/PS2Images/BK/"
#define PS2_IMAGES_BU_DIRECTORY "TestFiles/PS2Images/BU/"
#define PS2_IMAGES_FC_DIRECTORY "TestFiles/PS2Images/FC/"
#define PS2_IMAGES_SP_DIRECTORY "TestFiles/PS2Images/SP/"
#define PS2_IMAGES_PT_DIRECTORY "TestFiles/PS2Images/PT/"
#define PS2_IMAGES_RAVISH_BK_DIRECTORY "TestFiles/PS2Images/RAvishRomance/BK/"
#define PS2_IMAGES_RAVISH_BU_DIRECTORY "TestFiles/PS2Images/RAvishRomance/BU/"
#define PS2_IMAGES_RAVISH_FC_DIRECTORY "TestFiles/PS2Images/RAvishRomance/FC/"
#define PS2_IMAGES_RAVISH_SP_DIRECTORY "TestFiles/PS2Images/RAvishRomance/SP/"
#define PS2_IMAGES_RAVISH_PT_DIRECTORY "TestFiles/PS2Images/RAvishRomance/PT/"

/* Loads the entirety of a file into dynamic memory. */
Memory LoadFile(const char* filePath)
{
	FILE* file = NULL;
	u32 fileSize = 0;
	u8* data = NULL;
	Memory ret = { 0 };

	file = fopen(filePath, "rb");
	if (!file)
	{
		FOPEN_FAIL_MESSAGE(filePath);
		ret.data = NULL;
		return ret;
	}

	/* Get file size */
	fseek(file, 0, SEEK_END);
	fileSize = ftell(file);
	fseek(file, 0, SEEK_SET);
	fileSize -= ftell(file);

	/* Read in file */
	data = malloc(fileSize);
	if (!data)
	{
		fclose(file);
		ret.data = NULL;
		return ret;
	}
	fread(data, fileSize, 1, file);
	ret.data = data;
	ret.size = fileSize;

	fclose(file);
	return ret;
}

FilePathList InitFilePathList(Memory fileList)
{
	FilePathList ret = { 0 };
	ret.memoryPos = 0;
	ret.currentPath = NULL;
	ret.memory = fileList;
	return ret;
}

/* Set pathList.currentPath to point to the next path. Returns FALSE if there are no more. */
bool32 GetNextFilePath(FilePathList *pathList)
{
	u32 size = 0;
	u8* data = NULL;

	size = pathList->memory.size;
	data = pathList->memory.data;

	if (!pathList->currentPath) /* First path */
	{
		for (; pathList->memoryPos < size; ++pathList->memoryPos)
		{
			if (data[pathList->memoryPos] == '\n')
			{
				data[pathList->memoryPos] = '\0';
				pathList->currentPath = data;
				return TRUE;
			}
		}
		return FALSE;
	}
	else
	{
		++pathList->memoryPos;
		pathList->currentPath = &data[pathList->memoryPos];

		for (; pathList->memoryPos < size; ++pathList->memoryPos)
		{
			if (data[pathList->memoryPos] == '\n')
			{
				data[pathList->memoryPos] = '\0';
				return TRUE;
			}
		}
		return FALSE;
	}
}

u32 LittleEndianRead32(const u8* data)
{
	return data[0] + (data[1] << 8) + (data[2] << 16) + (data[3] << 24);
}

void GeneratePSPImageFileList(void)
{
	const u32 excludedImages[] =
	{
		2535, /* Is actually a PNG and not the RGO image archive file format */
		2537  /* It has a checksum, but the actual data is just all zeroes. */
	};
	FILE* file = NULL;
	u32 i = 0;
	u32 j = 0;

	file = fopen(PSP_IMAGES_FILE_LIST, "wb");
	if (!file)
	{
		FOPEN_FAIL_MESSAGE(PSP_IMAGES_FILE_LIST);
		return;
	}

	for (i = PSP_IMAGES_START_NUM; i <= PSP_IMAGES_END_NUM; ++i)
	{
		for (j = 0; j < NUM_ELEMENTS(excludedImages); ++j)
		{
			if (i == excludedImages[j])
			{
				goto skipImage;
			}
		}
		fprintf(file, "%s%u\n", PSP_IMAGES_DIRECTORY, i);
	skipImage:/* Do nothing */;
	}

	fclose(file);
}

#ifdef _WIN32
void GeneratePS2ImageFileList(void)
{
	const char* directories[] =
	{
		PS2_IMAGES_BK_DIRECTORY,
		PS2_IMAGES_BU_DIRECTORY,
		PS2_IMAGES_FC_DIRECTORY,
		PS2_IMAGES_SP_DIRECTORY,
		PS2_IMAGES_PT_DIRECTORY,
		PS2_IMAGES_RAVISH_BK_DIRECTORY,
		PS2_IMAGES_RAVISH_BU_DIRECTORY,
		PS2_IMAGES_RAVISH_FC_DIRECTORY,
		PS2_IMAGES_RAVISH_SP_DIRECTORY,
		PS2_IMAGES_RAVISH_PT_DIRECTORY
	};

	FILE* file = NULL;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATAA findData = { 0 };
	char buffer[MAX_PATH] = { 0 };

	u32 i = 0;

	file = fopen(PS2_IMAGES_FILE_LIST, "wb");
	if (!file)
	{
		FOPEN_FAIL_MESSAGE(PS2_IMAGES_FILE_LIST);
		return;
	}

	for (i = 0; i < NUM_ELEMENTS(directories); ++i)
	{
		strcpy(buffer, directories[i]);
		strcat(buffer, "/*");

		hFind = FindFirstFileA(buffer, &findData); /* Start search and discard . */

		if (hFind == INVALID_HANDLE_VALUE)
		{
			printf("ERROR: Could not find directory %s\n", directories[i]);
			fclose(file);
			return;
		}

		FindNextFileA(hFind, &findData); /* discard .. */

		while (FindNextFileA(hFind, &findData))
		{
			fprintf(file, "%s%s\n", directories[i], findData.cFileName);
		}

		if (GetLastError() != ERROR_NO_MORE_FILES)
		{
			printf("ERROR: Failed to get all files in directory %s\n", directories[i]);
			FindClose(hFind);
			fclose(file);
		}
		FindClose(hFind);
	}
	fclose(file);
}
#else
void GeneratePS2ImageFileList(void)
{
	printf("ERROR: GeneratePS2ImageFileList is only supported on Windows.\n"
		"Make sure that %s has not been deleted, moved, or renamed.\n", PS2_IMAGES_FILE_LIST);
}
#endif
