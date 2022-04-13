/* Filename: util.c
 * Author: TimepieceMaster
 * Description: Some useful utility functions and types
 * that do not correspond to any specific feature. */

#include <stdio.h>
#include <stdlib.h>
#include "util.h"

/* Loads the entirety of a file into dynamic memory.
 * Parameter: filePath - the file to load in.
 * Return: Memory object containing the loaded in file. */
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
