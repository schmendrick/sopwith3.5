#include "replay_compare.h"

#include <iostream>

int main(int argc, char** argv)
{
  if (argc != 3) {
    std::cerr << "Usage: replay-compare <left.state.txt> <right.state.txt>\n";
    return 2;
  }
  ReplayCompareResult result = replay_compare_files(argv[1], argv[2]);
  replay_compare_print_result(result);
  if (result.status == REPLAY_COMPARE_SUCCESS) {
    return 0;
  }
  return 1;
}
