#include "replay_compare_result.h"

#include <iostream>

void replay_compare_print_result(const ReplayCompareResult& result)
{
  if (result.status == REPLAY_COMPARE_SUCCESS) {
    std::cout << "Replay compare success\n";
    return;
  }

  if (result.status == REPLAY_COMPARE_WARNING) {
    std::cout << "Replay compare warning: " << result.message << "\n";
    return;
  }

  std::cout
    << "Replay compare failure: " << result.message
    << " frame=" << result.divergence.frame_index
    << " row=" << result.divergence.row_kind
    << " field=" << result.divergence.field_name
    << " lhs=" << result.divergence.lhs_value
    << " rhs=" << result.divergence.rhs_value
    << "\n";
}
