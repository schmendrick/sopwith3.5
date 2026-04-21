#ifndef SOPWITH_REPLAY_ORDERING_H
#define SOPWITH_REPLAY_ORDERING_H

#include <vector>
#include "replay_contract.h"

bool replay_validate_frame_order(const std::vector<ReplayRowKind>& rows);

#endif /* SOPWITH_REPLAY_ORDERING_H */
