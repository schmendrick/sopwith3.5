#include "replay_compare_result.h"

ReplayCompareResult replay_compare_success()
{
  ReplayCompareResult result;
  result.status = REPLAY_COMPARE_SUCCESS;
  result.message = "comparison succeeded";
  result.divergence.frame_index = -1;
  result.divergence.entity_id = -1;
  return result;
}

ReplayCompareResult replay_compare_warning(const std::string& message)
{
  ReplayCompareResult result = replay_compare_success();
  result.status = REPLAY_COMPARE_WARNING;
  result.message = message;
  return result;
}

ReplayCompareResult replay_compare_failure(const std::string& message)
{
  ReplayCompareResult result = replay_compare_success();
  result.status = REPLAY_COMPARE_FAILURE;
  result.message = message;
  return result;
}
