/* Filename: test.c
 * Author: TimepieceMaster
 * Description: Tests to verify correctness of the code. */

#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include "test.h"

/* Runs all of the tests */
void RunAllTests(void)
{
	TestUtilLoadFile(
		TEST_UTIL_LOAD_FILE_INPUT,
		TEST_UTIL_LOAD_FILE_OUTPUT,
		TEST_UTIL_LOAD_FILE_INVALID
	);
}

/* Tests the correctness of LoadFile in util.c */
void TestUtilLoadFile(
	const char* validPath, 
	const char* writePath, 
	const char* invalidPath) 
{
	Memory file = { 0 };
	FILE* pWriteFile = NULL;

	printf("\nRunning the unit test for LoadFile from util.c\n");

	printf("Copying the contents of %s to %s\n", validPath, writePath);
	file = LoadFile(validPath);
	pWriteFile = fopen(writePath, "wb");
	fwrite(file.data, file.size, 1, pWriteFile);
	free(file.data);

	printf("Attempting to a load file at an invalid path\n");
	file = LoadFile(invalidPath);
	free(file.data);
}
