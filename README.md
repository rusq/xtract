# Xtract for build engine GRP files (macOS port)

This is a port of the Xtract program to macOS.

The [original Xtract][original] Copyright (C) 2000  Timothy Hale and Mathieu
Olivier, licensed under the GPL.

It goes well with the great
[Blender Add-on for importing BUILD engine maps][iibm].

## Usage

```sh
xtract <grp file>
```

## Building

```sh
make
```

[original]: https://blood.sourceforge.net/rebuild.php
[iibm]: https://github.com/jensnt/io_import_build_map

Original readme_x.txt:
```
--- xtract readme---

This program is designed to extract the palette.dat file, any art files, and any maps in a build engine group (grp) file.

It is packaged with the dukeconv program, which will convert all your build engine map files over to the intermediate map format used by Quake 1/2 and Half-Life. 

xtract will extract all the art from a build group file, and place them in a subdirectory named tiles. It will then Extract all the maps, and the dukeconv program will convert them, and place them in a subdirectory named maps. 

After this you will need to put all the tiles in a wad:
http://prime.telefragged.com/wally/
Links you to wally, a very capable wad maker.

After this, you should be able to make your own maps using the extracted maps as a starting point. 

Enjoy!
```
