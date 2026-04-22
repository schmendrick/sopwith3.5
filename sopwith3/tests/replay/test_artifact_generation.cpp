#include "../../src/replay_contract.h"
#include "../../src/def.h"
#include "../../src/replay_ordering.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

static int check(bool cond)
{
  return cond ? 0 : 1;
}

static bool has_token(const std::string& line, const char* token)
{
  return line.find(token) != std::string::npos;
}

static std::vector<std::string> split_lines(const std::string& text)
{
  std::vector<std::string> lines;
  const char* p = text.c_str();
  while (*p) {
    const char* eol = std::strchr(p, '\n');
    if (eol == 0) {
      lines.push_back(std::string(p));
      break;
    }
    lines.push_back(std::string(p, eol));
    p = eol + 1;
  }
  return lines;
}

/** Synthetic GROUND row: flat terrain (all zeros), matches data-model GroundRow shape. */
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

int main()
{
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

  const std::string sample =
      std::string(kHead) + make_flat_ground_row(0) + "\n" + kTail;

  int failures = 0;
  std::vector<std::string> lines = split_lines(sample);
  failures += check(!lines.empty());
  const std::string& session = lines[0];
  failures += check(has_token(session, "schema_version="));
  failures += check(has_token(session, "initial_seed="));
  failures += check(has_token(session, "latency="));
  failures += check(has_token(session, "playerindex="));
  failures += check(has_token(session, "gamemode="));
  failures += check(has_token(session, "session_id="));
  failures += check(has_token(session, "version="));
  failures += check(has_token(session, "rules_version="));

  failures += check(has_token(lines[5], "object_kind="));

  std::vector<ReplayRowKind> kinds;
  for (std::size_t i = 1; i < lines.size(); ++i) {
    kinds.push_back(replay_parse_row_kind(lines[i]));
  }
  failures += check(replay_validate_frame_order(kinds));

  return failures;
}
