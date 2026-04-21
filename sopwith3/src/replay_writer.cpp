#include "replay_writer.h"

#include "def.h"

#include <cctype>
#include <fstream>
#include <sstream>

static std::ofstream replay_writer_stream;
static bool replay_writer_open_flag = false;

static void replay_sanitize_field(std::ostream& out, const std::string& s)
{
  for (std::size_t i = 0; i < s.size(); ++i) {
    unsigned char c = static_cast<unsigned char>(s[i]);
    if (c == '|' || c == '\n' || c == '\r') {
      out.put('_');
    }
    else {
      out.put(static_cast<char>(c));
    }
  }
}

static const char* replay_gamemode_label(int g)
{
  switch (g) {
    case NO_GAMEMODE:
      return "NO_GAMEMODE";
    case SINGLE:
      return "SINGLE";
    case COMPUTER:
      return "COMPUTER";
    case NOVICE:
      return "NOVICE";
    case MULTIPLE:
      return "MULTIPLE";
    default:
      return "UNKNOWN";
  }
}

bool replay_open_writer(const std::string& path)
{
  replay_writer_stream.open(path.c_str(), std::ios::out | std::ios::trunc);
  replay_writer_open_flag = replay_writer_stream.good();
  return replay_writer_open_flag;
}

void replay_close_writer()
{
  if (replay_writer_stream.good()) {
    replay_writer_stream.flush();
  }
  replay_writer_stream.close();
  replay_writer_open_flag = false;
}

bool replay_writer_is_open()
{
  return replay_writer_open_flag && replay_writer_stream.good();
}

void replay_writer_emit_line(const std::string& line)
{
  if (!replay_writer_is_open()) {
    return;
  }
  replay_writer_stream << line << "\n";
}

void replay_write_session_row(const ReplaySessionInfo& info)
{
  if (!replay_writer_is_open()) {
    return;
  }
  replay_writer_stream << "SESSION"
                        << "|schema_version=" << info.schema_version
                        << "|initial_seed=" << info.initial_seed
                        << "|latency=" << info.latency
                        << "|playerindex=" << info.playerindex
                        << "|gamemode=" << replay_gamemode_label(info.gamemode)
                        << "|session_id=";
  replay_sanitize_field(replay_writer_stream, info.session_id);
  replay_writer_stream << "|version=";
  replay_sanitize_field(replay_writer_stream, info.engine_version);
  replay_writer_stream << "|rules_version=" << info.rules_version << "\n";
}

void replay_write_frame_begin(int frame_index)
{
  if (!replay_writer_is_open()) {
    return;
  }
  replay_writer_stream << "FRAME_BEGIN|frame_index=" << frame_index << "\n";
}

void replay_write_frame_end(int frame_index)
{
  if (!replay_writer_is_open()) {
    return;
  }
  replay_writer_stream << "FRAME_END|frame_index=" << frame_index << "\n";
}

void replay_write_frame_row(int frame_index, unsigned speedtick)
{
  if (!replay_writer_is_open()) {
    return;
  }
  replay_writer_stream << "FRAME|frame_index=" << frame_index << "|speedtick=" << speedtick << "\n";
}
