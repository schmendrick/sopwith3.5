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

static void write_file(const char* path, const std::string& body)
{
  std::ofstream out(path, std::ios::binary | std::ios::trunc);
  out << body;
}

int main(int argc, char** argv)
{
  if (argc != 3) {
    return 1;
  }

  static const char kCanonicalArtifact[] =
    "SESSION|schema_version=1|initial_seed=42|latency=1|playerindex=0|gamemode=SINGLE|session_id=demo.rec|"
    "version=sopwith3-replay-v1|rules_version=2\n"
    "FRAME_BEGIN|frame_index=0\n"
    "FRAME|frame_index=0|speedtick=0\n"
    "GROUND|frame_index=0|z0000=0\n"
    "PLAYER|entity_id=0|angle=0|x_int=0\n"
    "FRAME_END|frame_index=0\n";

  const std::string body(kCanonicalArtifact);
  write_file(argv[1], body);
  write_file(argv[2], body);

  return load_file(argv[1]) == load_file(argv[2]) ? 0 : 1;
}
