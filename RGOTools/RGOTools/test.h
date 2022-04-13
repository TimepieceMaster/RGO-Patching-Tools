#ifndef TEST_H
#define TEST_H

#define TEST_UTIL_LOAD_FILE_INPUT "TestFiles/LoadFileInput.txt"
#define TEST_UTIL_LOAD_FILE_OUTPUT "TestFiles/LoadFileOutput.log"
#define TEST_UTIL_LOAD_FILE_INVALID "INVALID_PATH"

void RunAllTests(void);

void TestUtilLoadFile(
	const char* validPath,
	const char* writePath,
	const char* invalidPath
);

#endif
