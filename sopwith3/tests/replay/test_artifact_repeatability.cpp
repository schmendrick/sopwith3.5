#include "../../src/def.h"

#include <cstdio>
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

static std::string make_flat_ground_row(int frame_index)
{
  std::string row;
  row.reserve(static_cast<std::size_t>(MAX_X) * 2u + 96u);
  char head[96];
  std::snprintf(head,
                sizeof head,
                "GROUND|frame_index=%d|ground_count=%d|ground_values=",
                frame_index,
                MAX_X);
  row.append(head);
  for (int i = 0; i < MAX_X; ++i) {
    if (i > 0) {
      row.push_back(',');
    }
    row.push_back('0');
  }
  return row;
}

int main(int argc, char** argv)
{
  if (argc != 3) {
    return 1;
  }

  static const char kHead[] =
    "SESSION|schema_version=3|initial_seed=42|latency=1|playerindex=0|gamemode=SINGLE|session_id=demo.rec|"
    "version=sopwith3-replay-v1|rules_version=2\n"
    "FRAME_BEGIN|frame_index=0\n"
    "FRAME|frame_index=0\n";

  static const char kTail[] =
    "PLAYER|entity_id=0|angle=0|x_int=0\n"
    "OBJECT|entity_id=0|object_kind=Smoke|angle=0|firing=0|inverted=0|life=0|speed=0|width=0|height=0|colour=0|"
    "x_frac=0|x_int=0|y_frac=0|y_int=0\n"
    "FRAME_END|frame_index=0\n";

  const std::string body =
      std::string(kHead) + make_flat_ground_row(0) + "\n" + kTail;

  write_file(argv[1], body);
  write_file(argv[2], body);

  return load_file(argv[1]) == load_file(argv[2]) ? 0 : 1;
}
