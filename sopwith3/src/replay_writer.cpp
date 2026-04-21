#include "replay_writer.h"

#include <fstream>

static std::ofstream replay_writer_stream;

bool replay_open_writer(const std::string& path)
{
  replay_writer_stream.open(path.c_str(), std::ios::out | std::ios::trunc);
  return replay_writer_stream.good();
}

void replay_close_writer()
{
  if (replay_writer_stream.good()) {
    replay_writer_stream.flush();
  }
  replay_writer_stream.close();
}

void replay_write_session_row(int schema_version, int seed, int latency, int playerindex)
{
  if (!replay_writer_stream.good()) {
    return;
  }
  replay_writer_stream
    << "SESSION"
    << "|schema_version=" << schema_version
    << "|initial_seed=" << seed
    << "|latency=" << latency
    << "|playerindex=" << playerindex
    << "\n";
}

void replay_write_frame_begin(int frame_index)
{
  if (!replay_writer_stream.good()) {
    return;
  }
  replay_writer_stream << "FRAME_BEGIN|frame_index=" << frame_index << "\n";
}

void replay_write_frame_end(int frame_index)
{
  if (!replay_writer_stream.good()) {
    return;
  }
  replay_writer_stream << "FRAME_END|frame_index=" << frame_index << "\n";
}
