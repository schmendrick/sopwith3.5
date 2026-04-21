#include "replay_compare_result.h"
#include "replay_ordering.h"
#include "replay_validate.h"

#include <vector>

ReplayCompareResult replay_compare_validate_structure(const std::vector<ReplayRowKind>& lhs_rows,
                                                      const std::vector<ReplayRowKind>& rhs_rows)
{
  if (!replay_validate_required_rows(lhs_rows) || !replay_validate_required_rows(rhs_rows)) {
    return replay_compare_failure("missing required row kind");
  }
  if (!replay_validate_frame_order(lhs_rows) || !replay_validate_frame_order(rhs_rows)) {
    return replay_compare_failure("invalid row ordering");
  }
  return replay_compare_success();
}
