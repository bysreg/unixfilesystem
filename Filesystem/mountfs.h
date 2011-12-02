#ifndef MOUNTFS_H
#define	MOUNTFS_H

#ifndef FILESYSTEM_H
#include "Filesystem.h"
#endif

class mountfs {
public:
    mountfs();
    mountfs(const mountfs& orig);
    virtual ~mountfs();

    char* getFSname(string path);
    static void mount(string path);
private:

};

#endif	/* MOUNTFS_H */

