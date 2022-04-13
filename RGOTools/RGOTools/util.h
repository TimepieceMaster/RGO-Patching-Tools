#ifndef UTILS_H
#define UTIL_H

typedef unsigned char u8;
typedef unsigned int u32;

typedef struct
{
	u32 size;
	u8* data;
} Memory;

Memory LoadFile(const char* filePath);

#endif
