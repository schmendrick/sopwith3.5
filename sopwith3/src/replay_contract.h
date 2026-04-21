#ifndef SOPWITH_REPLAY_CONTRACT_H
#define SOPWITH_REPLAY_CONTRACT_H

#include <string>

enum ReplayRowKind {
  ROW_UNKNOWN = 0,
  ROW_SESSION,
  ROW_FRAME_BEGIN,
  ROW_FRAME,
  ROW_GROUND,
  ROW_PLAYER,
  ROW_ENEMY,
  ROW_OBJECT,
  ROW_FRAME_END
};

const char* replay_row_kind_to_string(ReplayRowKind kind);
ReplayRowKind replay_row_kind_from_string(const std::string& value);
ReplayRowKind replay_parse_row_kind(const std::string& row);
bool replay_row_kind_required(ReplayRowKind kind);

#endif /* SOPWITH_REPLAY_CONTRACT_H */
