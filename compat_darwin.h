#pragma once
#include <sys/stat.h>
#include <unistd.h>

#define _MAX_PATH 1023

int _mkdir(const char *path) { return mkdir(path, 0777); }

char *_getcwd(char *buffer, int maxlen) { return getcwd(buffer, maxlen); }

int _chdir(const char *path) { return chdir(path); }
