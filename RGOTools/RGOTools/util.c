/* Filename: util.c
 * Author: TimepieceMaster
 * Description: Some useful utility functions and types
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
	pFile = fopen(filePath, "rb");
	if (!pFile)
	{
		printf("ERROR: Could not open file at %s\n", filePath);
		return ret;
	}

	/* Get file size */
	fseek(pFile, 0, SEEK_END);
	fileSize = ftell(pFile);
	fseek(pFile, 0, SEEK_SET);
	fileSize -= ftell(pFile);

	/* Read in file */
	pData = malloc(fileSize);
	if (!pData)
	{
		printf("ERROR: Allocation of %u bytes failed\n", fileSize);
		fclose(pFile);
		return ret;
	}

	fread(pData, fileSize, 1, pFile);
	ret.data = pData;
	ret.size = fileSize;

	fclose(pFile);
	return ret;
}

/* Reads a 32-bit little endian value.
 * Parameter: data - the data from which to read.
 * Return: the read 32-bit little endian value. */
u32 LittleEndianRead32(const u8* data)
{
	return data[0] + (data[1] << 8) + (data[2] << 16) + (data[3] << 24);
}

/* Generates the list of file names for the images in RGO PSP.
 * Parameter: path - the path where the file names should be written. */
bool32 GeneratePSPImageFileList(void)
{
	const u32 startNum = 824;
	const u32 endNum = 2539;
	FILE* pFile = NULL;
	u32 i = 0;

	pFile = fopen(PSP_IMAGES_FILE_LIST, "wb");
	if (!pFile)
	{
		printf("ERROR: Could not open file at %s\n", PSP_IMAGES_FILE_LIST);
		return FALSE;
	}

	for (i = startNum; i <= endNum; ++i)
	{
		fprintf(pFile, "%s%u\n", PSP_IMAGES_DIRECTORY, i);
	}

	fclose(pFile);
	return TRUE;
}

/* Generates the list of file names for the images in RGO PS2.
 * Parameter: path - the path where the file names should be written.
 * Return: whether or not the file list was successfully generated. */
#ifdef _WIN32
bool32 GeneratePS2ImageFileList(void)
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

	pFile = fopen(PS2_IMAGES_FILE_LIST, "wb");

	if (!pFile)
	{
		printf("ERROR: Could not open file at %s\n", PSP_IMAGES_FILE_LIST);
		return FALSE;
	}

	for (i = 0; i < sizeof(directories) / sizeof(const char*); ++i)
	{
		strcpy(buffer, directories[i]);
		strcat(buffer, "/*");

		hFind = FindFirstFileA(buffer, &findData); /* Start search and discard . */

		if (hFind == INVALID_HANDLE_VALUE)
		{
			printf("ERROR: Could not find directory %s\n", directories[i]);
			fclose(pFile);
			return FALSE;
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
			return FALSE;
		}
		FindClose(hFind);
	}

	fclose(pFile);
	return TRUE;
}
#else
bool32 GeneratePS2ImageFileList(void)
{
	printf("ERROR: GeneratePS2ImageFileList is only supported on Windows.\n"
		"Make sure that %s has not been deleted, moved, or renamed.\n", PS2_IMAGES_FILE_LIST);
	return FALSE;
}
#endif
