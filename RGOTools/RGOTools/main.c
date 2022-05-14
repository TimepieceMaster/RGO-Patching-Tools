/*  RGO Patching Tools Version 1.0.0
 *  main.c
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

#include "test.h"

int main(void)
{
	TestImageDecompressAllImages(TEST_IMAGE_DECOMPRESS_ALL_IMAGES_OUTPUT);
	TestImageDecompressSingleImage(TEST_IMAGE_DECOMPRESS_IMAGE_PS2_INPUT, TEST_IMAGE_DECOMPRESS_IMAGE_PS2_OUTPUT);
	TestImageDecompressSingleImage(TEST_IMAGE_DECOMPRESS_IMAGE_PSP_INPUT, TEST_IMAGE_DECOMPRESS_IMAGE_PSP_OUTPUT);
	return 0;
}
