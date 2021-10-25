#ifndef __DISKIO_H_
#define __DISKIO_H_

#include <os>
#include <memdisk> // for VFS
#include <fstream>

std::vector<unsigned char> readfile_vfs(const std::string& file_name);

#endif // __DISKIO_H_
