#include "replay_compare.h"
#include "replay_validate.h"

#include <fstream>
#include <string>

ReplayCompareResult replay_compare_make_divergence(int frame_index,
                                                   const char* row_kind,
                                                   int entity_id,
                                                   const char* field_name,
                                                   const char* lhs_value,
                                                   const char* rhs_value);

ReplayCompareResult replay_compare_files(const std::string& lhs_path, const std::string& rhs_path)
{
  std::ifstream lhs(lhs_path.c_str());
  std::ifstream rhs(rhs_path.c_str());

  if (!lhs.good() || !rhs.good()) {
    return replay_compare_failure("could not open artifact file");
  }

  std::string lhs_line;
  std::string rhs_line;
  int line_no = 0;
  while (std::getline(lhs, lhs_line) && std::getline(rhs, rhs_line)) {
    ++line_no;
    if (lhs_line != rhs_line) {
      return replay_compare_make_divergence(line_no, "UNKNOWN", -1, "line", lhs_line.c_str(), rhs_line.c_str());
    }
  }

  if (std::getline(lhs, lhs_line) || std::getline(rhs, rhs_line)) {
    return replay_compare_warning("artifact lengths differ; possible truncation");
  }

  return replay_compare_success();
}
