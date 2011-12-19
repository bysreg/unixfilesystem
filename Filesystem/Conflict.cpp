#include <sys/stat.h>
#include "Conflict.h"

int Conflict::LINUXmkdir(const char *path, int mode) {
    return mkdir(path,mode);
}