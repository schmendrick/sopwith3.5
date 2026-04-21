#include "../../src/replay_compare.h"

int main()
{
  ReplayCompareResult result = replay_compare_files("missing_left.txt", "missing_right.txt");
  return result.status == REPLAY_COMPARE_FAILURE ? 0 : 1;
}
