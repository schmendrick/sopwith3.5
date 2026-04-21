#include "replay_verify.h"

bool replay_verify_baseline_scope(const std::string& gamemode)
{
  return gamemode == "SINGLE";
}
