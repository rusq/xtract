# Xtract for build engine GRP files (macOS port)

This is a port of the Xtract program to macOS.

The [original Xtract][original] Copyright (C) 2000  Timothy Hale and Mathieu
Olivier, licensed under the GPL, hosted on ReBUILD Project.

It goes well with the great
[Blender Add-on for importing BUILD engine maps][iibm].

[![](https://github.com/jensnt/io_import_build_map/blob/main/images/e1l1.png)][iibm]

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

## Notes
Map extraction was reverted to the native with small changes (it now extracts
to maps/ instead of the current directory).  

The original xtract used `dukeconv`, this code has been isolated under DUKECONV
define.  If you want to use `dukeconv`, build it with `make DUKECONV=1`.

You can get the `dukeconv.exe` from the original xtract archive by downloading
it from the [original page][original].

## Original readme_x.txt
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

## Other useful Duke3d related links
- [Build Engine Source Code Page](http://www.advsys.net/ken/buildsrc/default.htm)
- http://dukertcm.com/knowledge-base/downloads-rtcm/duke3d-tools-editart/
- [GRP Plugin for Windows Commander](https://github.com/creaktive/grp.wcx)
- [Grpar tool to extract GRP files](https://github.com/martymac/grpar)
- [grpdecompiler](https://github.com/PopovEvgeniy/grpdecompiler)
- [duke4.net forums](https://forums.duke4.net/)
