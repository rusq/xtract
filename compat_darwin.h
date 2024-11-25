#pragma once
#if defined (__APPLE__) || defined (__FreeBSD__)

#include <sys/stat.h>
#include <unistd.h>

#define _MAX_PATH 1023

#define OS_DEL_CMD "rm "

#define PATH_SEPARATOR "/"

#define _mkdir(path) mkdir(path, 0777)
#define _getcwd(buffer, maxlen) getcwd(buffer, maxlen)
#define _chdir(path) chdir(path)

#endif /* __APPLE__ || __FreeBSD__ */
