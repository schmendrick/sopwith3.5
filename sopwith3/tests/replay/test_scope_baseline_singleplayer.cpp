#include "../../src/replay_verify.h"

int main()
{
  if (!replay_verify_baseline_scope("SINGLE")) {
    return 1;
  }
  if (replay_verify_baseline_scope("MULTIPLE")) {
    return 1;
  }
  return 0;
}
