#include <memdisk> // for VFS
#include <fstream>

std::vector<unsigned char> readfile_vfs(const std::string& file_name)
{
  // read bitstream file using memdisk
  auto& disk = fs::memdisk();
  disk.init_fs([] (fs::error_t err, auto&) {
    assert(!err);
  });

  auto file = disk.fs().read_file(file_name);
  std::vector<unsigned char> bs(file.data(), file.data() + file.size());
  return bs;
}


