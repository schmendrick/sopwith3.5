#include "../../src/replay_contract.h"
#include "../../src/replay_ordering.h"

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

static std::vector<std::string> split_lines(const char* text)
{
  std::vector<std::string> lines;
  const char* p = text;
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

int main()
{
  static const char kSampleArtifact[] =
    "SESSION|schema_version=1|initial_seed=42|latency=1|playerindex=0|gamemode=SINGLE|session_id=demo.rec|"
    "version=sopwith3-replay-v1|rules_version=2\n"
    "FRAME_BEGIN|frame_index=0\n"
    "FRAME|frame_index=0\n"
    "GROUND|frame_index=0|z0000=0\n"
    "PLAYER|entity_id=0|angle=0|x_int=0\n"
    "FRAME_END|frame_index=0\n";

  int failures = 0;
  std::vector<std::string> lines = split_lines(kSampleArtifact);
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

  std::vector<ReplayRowKind> kinds;
  for (std::size_t i = 1; i < lines.size(); ++i) {
    kinds.push_back(replay_parse_row_kind(lines[i]));
  }
  failures += check(replay_validate_frame_order(kinds));

  return failures;
}
