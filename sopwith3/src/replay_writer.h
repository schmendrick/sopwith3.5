#ifndef SOPWITH_REPLAY_WRITER_H
#define SOPWITH_REPLAY_WRITER_H

#include <string>

bool replay_open_writer(const std::string& path);
void replay_close_writer();
void replay_write_session_row(int schema_version, int seed, int latency, int playerindex);
void replay_write_frame_begin(int frame_index);
void replay_write_frame_end(int frame_index);

#endif /* SOPWITH_REPLAY_WRITER_H */
