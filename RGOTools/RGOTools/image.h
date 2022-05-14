/*  RGO Patching Tools Version 1.0.0
 *  image.h
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

#ifndef IMAGE_H
#define IMAGE_H

#include "util.h"

typedef struct
{
	u32 nImages;
	u32 lastPaletteSize;
	bool32 hasDefaultHeaderOffset;
} NumImagesInfo;

NumImagesInfo GetNumImages(Memory imageData);
u8* GetPalette(u8* imageData, u32 index);
u8* GetImageHeader(Memory imageData, NumImagesInfo numImagesInfo, u32 index);
u8* GetNextImageHeader(u8* currentHeader);
Platform GetImagePlatform(const u8* header);
Memory DecompressImage(u8* header, Platform platform);

#endif
