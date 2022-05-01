/*  RGO Patching Tools Version 1.0.0
 *  test.h
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

#ifndef TEST_H
#define TEST_H

#define TEST_UTIL_LOAD_FILE_INPUT "TestFiles/MiscInput/LoadFileInput.txt"
#define TEST_UTIL_LOAD_FILE_OUTPUT "TestFiles/Results/LoadFileOutput.log"
#define TEST_UTIL_LOAD_FILE_INVALID "INVALID_PATH"
#define TEST_UTIL_INIT_FILE_LIST_INPUT "TestFiles/PSPImages/filelist.txt"
#define TEST_UTIL_INIT_FILE_LIST_OUTPUT "TestFiles/Results/InitFileListOutput.log"
#define TEST_IMAGE_GET_NUM_IMAGES_OUTPUT "TestFiles/Results/GetNumImagesOutput.log"

void RunAllTests(void);

void TestUtilLoadFile(const char* validPath, const char* writePath, const char* invalidPath);
void TestUtilInitFileList(const char* path, const char* writePath);
void TestImageGetNumImages(const char* writePath);

#endif
