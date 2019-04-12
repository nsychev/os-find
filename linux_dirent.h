#ifndef FIND_DIRENT
#define FIND_DIRENT

#include <linux/limits.h>

#define DIRENT_SIZE (PATH_MAX + 96)

struct linux_dirent {
    unsigned long   d_ino;
    off_t           d_off;
    unsigned short  d_reclen;
    char            d_name[];
    // char         d_type;
};

#endif
