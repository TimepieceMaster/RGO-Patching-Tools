# RGO-Patching-Tools
Development tools for the Lucky Star: Ryouou Gakuen Outousai English Patch

## NOTICE

As of now, the RGO Patching Tools are in early development, and do not currently
have any useful functionality. Check back later for updates.

## Setup
Files from Lucky Star: Ryouou Gakuen Outousai cannot be provided and you
will have to supply them yourself. Once you have obtained the .iso for the
PSP and/or PS2 versions of the game, you can use a program like 7-zip to
extract the iso contents.

Many of the files that the RGO Patching Tools modify are contained within .afs files
on the PS2 version, and .cpk files on the PSP version. These are special archive file
formats and you will need special outside programs to extract the files contained within,
as RGO Patching Tools does not support the extraction and repackaging of .afs or .cpk files,
nor are there currently any plans to add such support.

To extract .afs files, use AFSPacker:   https://github.com/MaikelChan/AFSPacker <br />
To extract .cpk files, use CriPakTools: https://github.com/esperknight/CriPakTools <br />

The RGO Patching Tools also have the following dependencies:

zlib (https://zlib.net/). The source code for zlib should be placed in /RGOTools/RGOTools/OutsideCode/zlib/ <br />
libpng (http://www.libpng.org/pub/png/libpng.html). The source code for libpng should be placed in /RGOTools/RGOTools/OutsideCode/libpng/ <br />

Additionally, you will have to copy zlib.h and zconf.h from the zlib folder into the libpng folder, as well as copy pnglibconf.h.prebuilt from libpng/scripts into the main libpng
folder and rename it to pnglibconf.h <br />

## Images

The image files for the PSP version of Lucky Star: Ryouou Gakuen Outousai are contained within the following
.cpk file:

PSP_GAME/USRDIR/DATA/union.cpk <br />

and are the files 824 to 2539 in the archive, except for files 2535 and 2537.

For the PS2 version, the image files are contained within the following .afs files:

DATA/BK.AFS <br />
DATA/BU.AFS <br />
DATA/FC.AFS <br />
DATA/SP.AFS <br />
DATA/PT.AFS <br />

The PS2 demo, RAvish Romance, is also supported. The image files are located in the same locations as in the final PS2 version.
