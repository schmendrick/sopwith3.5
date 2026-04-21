#include "../../src/replay_validate.h"

#include <vector>

int main()
{
  std::vector<ReplayRowKind> rows;
  rows.push_back(ROW_FRAME_BEGIN);
  rows.push_back(ROW_FRAME);
  rows.push_back(ROW_FRAME_END);
  return replay_validate_required_rows(rows) ? 1 : 0;
}
