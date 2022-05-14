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
#define TEST_UTIL_FILE_PATH_LIST_INPUT "TestFiles/PSPImages/filelist.txt"
#define TEST_UTIL_FILE_PATH_LIST_OUTPUT "TestFiles/Results/FilePathListOutput.log"
#define TEST_IMAGE_GET_NUM_IMAGES_OUTPUT "TestFiles/Results/GetNumImagesOutput.log"
#define TEST_IMAGE_GET_IMAGE_HEADER_OUTPUT "TestFiles/Results/GetImageHeaderOutput.log"
#define TEST_IMAGE_DECOMPRESS_IMAGE_PSP_INPUT "TestFiles/PSPImages/BIN/824"
#define TEST_IMAGE_DECOMPRESS_IMAGE_PSP_OUTPUT "TestFiles/Results/DecompressImagePSPOutput.bin"
#define TEST_IMAGE_DECOMPRESS_IMAGE_PS2_INPUT "TestFiles/PS2Images/BK/BG_000_A0.obj"
#define TEST_IMAGE_DECOMPRESS_IMAGE_PS2_OUTPUT "TestFiles/Results/DecompressImagePS2Output.bin"
#define TEST_IMAGE_DECOMPRESS_ALL_IMAGES_OUTPUT "TestFiles/Results/DecompressAllImagesOutput.log"

void TestUtilLoadFile(const char* inputPath, const char* outputPath);
void TestUtilFilePathList(const char* inputPath, const char* outputPath);
void TestImageGetNumImages(const char* outputPath);
void TestImageGetImageHeader(const char* outputPath);
void TestImageDecompressSingleImage(const char* inputPath, const char* outputPath);
void TestImageDecompressAllImages(const char* outputPath);

#endif
