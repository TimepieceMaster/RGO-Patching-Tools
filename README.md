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

## Images

The image files for the PSP version of Lucky Star: Ryouou Gakuen Outousai are contained within the following
cpk file:

PSP_GAME/USRDIR/DATA/union.cpk <br />

and are the files 824 to 2539 in the archive.

For the PS2 version, the image files are contained within the following .afs files:

DATA/BK.AFS
DATA/BU.AFS
DATA/FC.AFS
