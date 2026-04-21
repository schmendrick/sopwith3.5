#include "replay_writer.h"

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

struct ReplayEntityRow {
  int entity_id;
  std::string payload;
};

static std::vector<ReplayEntityRow> replay_sort_by_entity(std::vector<ReplayEntityRow> rows)
{
  std::sort(rows.begin(), rows.end(), [](const ReplayEntityRow& lhs, const ReplayEntityRow& rhs) {
    return lhs.entity_id < rhs.entity_id;
  });
  return rows;
}

std::string replay_build_entity_payload(const std::string& row_kind, int entity_id, const std::string& payload)
{
  std::ostringstream out;
  out << row_kind << "|entity_id=" << entity_id << "|" << payload;
  return out.str();
}
