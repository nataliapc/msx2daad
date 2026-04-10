#include <string.h>
#include "dos.h"
#include "heap.h"


char* getProgramPath(char *path)
{
    switch (dosver()) {
        case VER_MSXDOS1x:
            strcpy(path, "A:\\");
            path[0] = get_current_drive() + 'A';
            break;
        case VER_MSXDOS2x:
        case VER_NextorDOS:
            if (get_env("PROGRAM", path, MAX_PATH_SIZE)) {
                return NULL;
            }
            break;
        default:
            return NULL;
    }
    return path;
}
