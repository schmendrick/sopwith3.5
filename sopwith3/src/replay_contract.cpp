#include "replay_contract.h"

const char* replay_row_kind_to_string(ReplayRowKind kind)
{
  switch (kind) {
    case ROW_SESSION: return "SESSION";
    case ROW_FRAME_BEGIN: return "FRAME_BEGIN";
    case ROW_FRAME: return "FRAME";
    case ROW_GROUND: return "GROUND";
    case ROW_PLAYER: return "PLAYER";
    case ROW_ENEMY: return "ENEMY";
    case ROW_OBJECT: return "OBJECT";
    case ROW_FRAME_END: return "FRAME_END";
    default: return "UNKNOWN";
  }
}

ReplayRowKind replay_row_kind_from_string(const std::string& value)
{
  if (value == "SESSION") return ROW_SESSION;
  if (value == "FRAME_BEGIN") return ROW_FRAME_BEGIN;
  if (value == "FRAME") return ROW_FRAME;
  if (value == "GROUND") return ROW_GROUND;
  if (value == "PLAYER") return ROW_PLAYER;
  if (value == "ENEMY") return ROW_ENEMY;
  if (value == "OBJECT") return ROW_OBJECT;
  if (value == "FRAME_END") return ROW_FRAME_END;
  return ROW_UNKNOWN;
}

bool replay_row_kind_required(ReplayRowKind kind)
{
  return kind == ROW_SESSION ||
         kind == ROW_FRAME_BEGIN ||
         kind == ROW_FRAME ||
         kind == ROW_GROUND ||
         kind == ROW_FRAME_END;
}
