#include "../../src/replay_validate.h"

int main()
{
  return replay_validate_schema_match("1", "2") ? 1 : 0;
}
