#include <fstream>
#include <string>

static std::string load_file(const char* path)
{
  std::ifstream in(path, std::ios::binary);
  if (!in.good()) {
    return "";
  }
  return std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
}

int main(int argc, char** argv)
{
  if (argc != 3) {
    return 1;
  }
  return load_file(argv[1]) == load_file(argv[2]) ? 0 : 1;
}
