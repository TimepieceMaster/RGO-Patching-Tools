#ifndef TEST_H
#define TEST_H

#define TEST_UTIL_LOAD_FILE_INPUT "TestFiles/MiscInput/LoadFileInput.txt"
#define TEST_UTIL_LOAD_FILE_OUTPUT "TestFiles/Results/LoadFileOutput.log"
#define TEST_UTIL_LOAD_FILE_INVALID "INVALID_PATH"
#define TEST_UTIL_INIT_FILE_LIST_INPUT "TestFiles/PSPImages/filelist.txt"
#define TEST_UTIL_INIT_FILE_LIST_OUTPUT "TestFiles/Results/InitFileListOutput.log"
#define TEST_IMAGE_GET_NUM_PALETTES_OUTPUT "TestFiles/Results/GetNumPalettesOutput.log"

void RunAllTests(void);

void TestUtilLoadFile(const char* validPath, const char* writePath, const char* invalidPath);
void TestUtilInitFileList(const char* path, const char* writePath);
void TestImageGetNumPalettes(const char* writePath);

#endif
