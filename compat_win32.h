#ifndef __COMPAT_WIN32_H__
#define __COMPAT_WIN32_H__
#ifdef _WIN32

#include <process.h> // system
#include <direct.h> // _mkdir, _getcwd, _chdir

#define OS_DEL_CMD "del "

#define PATH_SEPARATOR "\\"

#endif
#endif // __COMPAT_WIN32_H__
