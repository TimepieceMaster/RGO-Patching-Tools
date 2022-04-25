#ifndef UTILS_H
#define UTILS_H

#define PSP_IMAGES_FILE_LIST "TestFiles/PSPImages/filelist.txt"
#define PS2_IMAGES_FILE_LIST "TestFiles/PS2Images/filelist.txt"
#define PSP_IMAGES_DIRECTORY "TestFiles/PSPImages/BIN/"
#define PSP_IMAGES_START_NUM 824
#define PSP_IMAGES_END_NUM 2539
#define PS2_IMAGES_BK_DIRECTORY "TestFiles/PS2Images/BK/"
#define PS2_IMAGES_BU_DIRECTORY "TestFiles/PS2Images/BU/"
#define PS2_IMAGES_FC_DIRECTORY "TestFiles/PS2Images/FC/"

#define FALSE 0
#define TRUE (!FALSE)

#define NUM_ELEMENTS(x) (sizeof(x) / sizeof(x[0]))

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned int bool32;

typedef struct
{
	u32 size;
	u8* data;
} Memory;

typedef struct
{
	char** paths;
	u32 nFiles;
} FileList;

Memory LoadFile(const char* filePath);
FileList InitFileList(Memory file);
u32 LittleEndianRead32(const u8* data);
void* MallocMustSucceed(size_t size);
FILE* FOpenMustSucceed(const char* path, const char* mode);
void GeneratePSPImageFileList(void);
void GeneratePS2ImageFileList(void);

#endif
