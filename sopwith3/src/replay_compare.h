#ifndef SOPWITH_REPLAY_COMPARE_H
#define SOPWITH_REPLAY_COMPARE_H

#include <string>
#include "replay_compare_result.h"

ReplayCompareResult replay_compare_files(const std::string& lhs_path, const std::string& rhs_path);
void replay_compare_print_result(const ReplayCompareResult& result);

#endif /* SOPWITH_REPLAY_COMPARE_H */
