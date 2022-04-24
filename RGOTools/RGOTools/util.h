#ifndef UTILS_H
#define UTILS_H

#define PSP_IMAGES_FILE_LIST "TestFiles/PSPImages/filelist.txt"
#define PS2_IMAGES_FILE_LIST "TestFiles/PS2Images/filelist.txt"
#define PSP_IMAGES_DIRECTORY "TestFiles/PSPImages/BIN/"
#define PS2_IMAGES_BK_DIRECTORY "TestFiles/PS2Images/BK/"
#define PS2_IMAGES_BU_DIRECTORY "TestFiles/PS2Images/BU/"
#define PS2_IMAGES_FC_DIRECTORY "TestFiles/PS2Images/FC/"

#define FALSE 0
#define TRUE (!FALSE)

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned int bool32;

typedef struct
{
	u32 size;
	u8* data;
} Memory;

Memory LoadFile(const char* filePath);
u32 LittleEndianRead32(const u8* data);
bool32 GeneratePSPImageFileList(void);
bool32 GeneratePS2ImageFileList(void);

#endif
