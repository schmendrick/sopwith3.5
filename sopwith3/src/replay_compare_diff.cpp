#include "replay_compare_result.h"

ReplayCompareResult replay_compare_make_divergence(int frame_index,
                                                   const char* row_kind,
                                                   int entity_id,
                                                   const char* field_name,
                                                   const char* lhs_value,
                                                   const char* rhs_value)
{
  ReplayCompareResult result = replay_compare_failure("first divergence detected");
  result.divergence.frame_index = frame_index;
  result.divergence.row_kind = row_kind;
  result.divergence.entity_id = entity_id;
  result.divergence.field_name = field_name;
  result.divergence.lhs_value = lhs_value;
  result.divergence.rhs_value = rhs_value;
  return result;
}
