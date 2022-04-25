/* Filename: test.c
 * Author: TimepieceMaster
 * Description: Tests to verify correctness of the code. */

#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include "image.h"
#include "test.h"

/* Runs all of the tests */
void RunAllTests(void)
{
	TestUtilLoadFile(TEST_UTIL_LOAD_FILE_INPUT, TEST_UTIL_LOAD_FILE_OUTPUT, TEST_UTIL_LOAD_FILE_INVALID);
}

/* Tests the correctness of LoadFile in util.c */
void TestUtilLoadFile(const char* validPath, const char* writePath, const char* invalidPath)
{
	FILE* pWriteFile = NULL;
	Memory file = { 0 };

	printf("\nRunning the unit test for LoadFile from util.c\n");

	printf("Copying the contents of %s to %s\n", validPath, writePath);
	file = LoadFile(validPath);

	pWriteFile = FOpenMustSucceed(writePath, "wb");
	fwrite(file.data, file.size, 1, pWriteFile);
	free(file.data);
}

/* Tests the correctness of InitFileList in util.c */
void TestUtilInitFileList(const char* path, const char* writePath)
{
	FILE* pWriteFile = NULL;
	Memory file = { 0 };
	FileList fileList = { 0 };
	u32 i = 0;

	printf("\nRunning the unit test for InitFileList from util.c\n");

	pWriteFile = FOpenMustSucceed(writePath, "wb");

	file = LoadFile(path);
	fileList = InitFileList(file);

	for (i = 0; i < fileList.nFiles; ++i)
	{
		fprintf(pWriteFile, "%s\n", fileList.paths[i]);
	}
	fclose(pWriteFile);
	free(fileList.paths);
	free(file.data);
}

void TestImageGetNumPalettes(const char* writePath)
{
	const char* fileListPaths[] = { PSP_IMAGES_FILE_LIST, PS2_IMAGES_FILE_LIST };

	FILE* pWriteFile = NULL;

	Memory fileListMemory = { 0 };
	FileList fileList = { 0 };
	Memory image = { 0 };
	u32 nPalettes = 0;

	u32 i = 0;
	u32 j = 0;

	printf("\nRunning the unit test for GetNumPalettes from image.c\n");

	pWriteFile = FOpenMustSucceed(writePath, "wb");

	/* Get the number of palettes from each image */
	for (i = 0; i < NUM_ELEMENTS(fileListPaths); ++i)
	{
		fileListMemory = LoadFile(fileListPaths[i]);
		fileList = InitFileList(fileListMemory);

		for (j = 0; j < fileList.nFiles; ++j)
		{
			image = LoadFile(fileList.paths[j]);
			nPalettes = GetNumPalettes(image);
			fprintf(pWriteFile, "# palettes: %u. File: %s\n", nPalettes, fileList.paths[j]);
			free(image.data);
		}
		free(fileListMemory.data);
		free(fileList.paths);
	}
}
