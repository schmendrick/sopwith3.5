#include "../../src/replay_contract.h"

int main()
{
  return replay_row_kind_from_string("SESSION") == ROW_SESSION ? 0 : 1;
}
