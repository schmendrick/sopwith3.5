#include "replay_contract.h"

#include <vector>

bool replay_validate_frame_order(const std::vector<ReplayRowKind>& rows)
{
  if (rows.size() < 4) {
    return false;
  }
  if (rows.front() != ROW_FRAME_BEGIN || rows.back() != ROW_FRAME_END) {
    return false;
  }
  if (rows[1] != ROW_FRAME || rows[2] != ROW_GROUND) {
    return false;
  }

  for (std::size_t i = 3; i + 1 < rows.size(); ++i) {
    if (rows[i] == ROW_FRAME_BEGIN || rows[i] == ROW_FRAME || rows[i] == ROW_GROUND || rows[i] == ROW_FRAME_END) {
      return false;
    }
  }
  return true;
}
