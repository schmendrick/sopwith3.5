#include "replay_contract.h"

#include <map>
#include <string>

typedef std::map<std::string, std::string> ReplayFieldMap;

static ReplayFieldMap parse_fields(const std::string& row)
{
  ReplayFieldMap fields;
  std::size_t start = 0;
  while (start < row.size()) {
    std::size_t end = row.find('|', start);
    std::string token = row.substr(start, end == std::string::npos ? std::string::npos : end - start);
    std::size_t eq = token.find('=');
    if (eq != std::string::npos) {
      fields[token.substr(0, eq)] = token.substr(eq + 1);
    }
    if (end == std::string::npos) {
      break;
    }
    start = end + 1;
  }
  return fields;
}

ReplayRowKind replay_parse_row_kind(const std::string& row)
{
  std::size_t split = row.find('|');
  if (split == std::string::npos) {
    return replay_row_kind_from_string(row);
  }
  return replay_row_kind_from_string(row.substr(0, split));
}
