#ifndef SOPWITH_REPLAY_WRITER_H
#define SOPWITH_REPLAY_WRITER_H

#include <string>

struct ReplaySessionInfo {
  int schema_version;
  int initial_seed;
  int latency;
  int playerindex;
  int gamemode;
  int rules_version;
  std::string session_id;
  std::string engine_version;
};

bool replay_open_writer(const std::string& path);
void replay_close_writer();
bool replay_writer_is_open();
void replay_writer_emit_line(const std::string& line);

void replay_write_session_row(const ReplaySessionInfo& info);
void replay_write_frame_begin(int frame_index);
void replay_write_frame_end(int frame_index);
void replay_write_frame_row(int frame_index);

std::string replay_build_entity_payload(const std::string& row_kind, int entity_id, const std::string& payload);

#endif /* SOPWITH_REPLAY_WRITER_H */
