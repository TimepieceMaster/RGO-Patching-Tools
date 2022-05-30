/*  RGO Patching Tools Version 1.0.0
 *  util.h
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

#ifndef UTILS_H
#define UTILS_H

#define PSP_IMAGES_FILE_LIST "TestFiles/PSPImages/filelist.txt"
#define PS2_IMAGES_FILE_LIST "TestFiles/PS2Images/filelist.txt"

#define FALSE 0
#define TRUE (!FALSE)

#define NUM_ELEMENTS(x) (sizeof(x) / sizeof(x[0]))
#define FOPEN_FAIL_MESSAGE(path) (printf("Could not open file %s", path))
#define LOAD_FILE_FAIL_MESSAGE(path) (printf("Failed to load file %s", path))

#define CHECKSUM_LENGTH 16

typedef enum
{
	PLATFORM_PS2,
	PLATFORM_PSP
} Platform;

typedef int i32;
typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned int bool32;

typedef struct
{
	u32 size;
	u8* data;
} Memory;

/* Used to process newline-separated lists of file paths */
typedef struct
{
	u32 memoryPos;
	u8* currentPath;
	Memory memory;
} FilePathList;

Memory LoadFile(const char* filePath);
FilePathList InitFilePathList(Memory fileList);
bool32 GetNextFilePath(FilePathList* pathList);
u32 LittleEndianRead32(const u8* data);
void GeneratePSPImageFileList(void);
void GeneratePS2ImageFileList(void);

#endif
