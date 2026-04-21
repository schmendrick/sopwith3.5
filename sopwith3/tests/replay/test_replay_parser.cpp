#include "../../src/replay_contract.h"
#include "../../src/replay_ordering.h"

#include <vector>

static int check(bool cond)
{
  return cond ? 0 : 1;
}

int main()
{
  int failures = 0;
  failures += check(replay_row_kind_from_string("FRAME") == ROW_FRAME);

  std::vector<ReplayRowKind> valid;
  valid.push_back(ROW_FRAME_BEGIN);
  valid.push_back(ROW_FRAME);
  valid.push_back(ROW_GROUND);
  valid.push_back(ROW_PLAYER);
  valid.push_back(ROW_FRAME_END);
  failures += check(replay_validate_frame_order(valid));

  return failures;
}
