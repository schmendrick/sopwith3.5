#ifndef SOPWITH_REPLAY_COMPARE_RESULT_H
#define SOPWITH_REPLAY_COMPARE_RESULT_H

#include <string>

enum ReplayCompareStatus {
  REPLAY_COMPARE_SUCCESS = 0,
  REPLAY_COMPARE_WARNING,
  REPLAY_COMPARE_FAILURE
};

struct ReplayDivergence {
  int frame_index;
  std::string row_kind;
  int entity_id;
  std::string field_name;
  std::string lhs_value;
  std::string rhs_value;
};

struct ReplayCompareResult {
  ReplayCompareStatus status;
  std::string message;
  ReplayDivergence divergence;
};

ReplayCompareResult replay_compare_success();
ReplayCompareResult replay_compare_warning(const std::string& message);
ReplayCompareResult replay_compare_failure(const std::string& message);

#endif /* SOPWITH_REPLAY_COMPARE_RESULT_H */
