#include <stdio.h>
#include "test.h"
#include "util.h"

int main(void)
{
	RunAllTests();
	GeneratePSPImageFileList();
	GeneratePS2ImageFileList();
	return 0;
}
