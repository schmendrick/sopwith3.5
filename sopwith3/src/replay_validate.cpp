#include "replay_contract.h"

#include <set>
#include <string>
#include <vector>

bool replay_validate_required_rows(const std::vector<ReplayRowKind>& rows)
{
  std::set<ReplayRowKind> have(rows.begin(), rows.end());
  return have.count(ROW_FRAME_BEGIN) &&
         have.count(ROW_FRAME) &&
         have.count(ROW_GROUND) &&
         have.count(ROW_FRAME_END);
}

bool replay_validate_schema_match(const std::string& lhs_schema, const std::string& rhs_schema)
{
  return lhs_schema == rhs_schema;
}
