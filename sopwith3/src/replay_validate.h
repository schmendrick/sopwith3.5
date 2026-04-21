#ifndef SOPWITH_REPLAY_VALIDATE_H
#define SOPWITH_REPLAY_VALIDATE_H

#include <string>
#include <vector>
#include "replay_contract.h"

bool replay_validate_required_rows(const std::vector<ReplayRowKind>& rows);
bool replay_validate_schema_match(const std::string& lhs_schema, const std::string& rhs_schema);

#endif /* SOPWITH_REPLAY_VALIDATE_H */
