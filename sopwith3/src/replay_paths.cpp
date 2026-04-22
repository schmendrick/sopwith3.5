#include "replay_paths.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <dirent.h>

namespace {

bool ends_with_icase(const std::string& s, const std::string& suf)
{
  if (s.size() < suf.size()) {
    return false;
  }
  for (size_t i = 0; i < suf.size(); ++i) {
    const char a = static_cast<char>(std::tolower(static_cast<unsigned char>(s[s.size() - suf.size() + i])));
    const char b = static_cast<char>(std::tolower(static_cast<unsigned char>(suf[i])));
    if (a != b) {
      return false;
    }
  }
  return true;
}

std::string join_dir_file(const std::string& dir, const std::string& name)
{
  if (dir.empty() || dir == ".") {
    return name;
  }
  if (!dir.empty() && (dir.back() == '/' || dir.back() == '\\')) {
    return dir + name;
  }
  return dir + "/" + name;
}

bool parse_sidecar_index(const std::string& stem, const std::string& filename, int* out_index)
{
  const std::string pfx = stem + ".";
  const std::string sfx = ".sidecar";
  if (filename.size() <= pfx.size() + sfx.size()) {
    return false;
  }
  if (filename.compare(0, pfx.size(), pfx) != 0) {
    return false;
  }
  if (!ends_with_icase(filename, sfx)) {
    return false;
  }
  const std::string mid = filename.substr(pfx.size(), filename.size() - pfx.size() - sfx.size());
  if (mid.empty()) {
    return false;
  }
  for (size_t i = 0; i < mid.size(); ++i) {
    if (mid[i] < '0' || mid[i] > '9') {
      return false;
    }
  }
  const int v = std::atoi(mid.c_str());
  if (v <= 0) {
    return false;
  }
  *out_index = v;
  return true;
}

} // namespace

std::string replay_normalize_replay_token_to_tape_path(const std::string& replay_token)
{
  std::string prefix;
  std::string basename = replay_token;
  const std::string::size_type slash = basename.find_last_of("/\\");
  if (slash != std::string::npos) {
    prefix = basename.substr(0, slash + 1);
    basename = basename.substr(slash + 1);
  }
  for (;;) {
    if (ends_with_icase(basename, ".tape")) {
      basename.resize(basename.size() - 5);
      continue;
    }
    if (ends_with_icase(basename, ".rec")) {
      basename.resize(basename.size() - 4);
      continue;
    }
    break;
  }
  return prefix + basename + ".tape";
}

std::string replay_directory_containing_file(const std::string& filepath)
{
  const std::string::size_type slash = filepath.find_last_of("/\\");
  if (slash == std::string::npos) {
    return ".";
  }
  if (slash == 0) {
    return filepath.substr(0, 1);
  }
  return filepath.substr(0, slash);
}

std::string replay_filename_component(const std::string& filepath)
{
  const std::string::size_type slash = filepath.find_last_of("/\\");
  if (slash == std::string::npos) {
    return filepath;
  }
  return filepath.substr(slash + 1);
}

std::string replay_tape_stem_from_normalized_path(const std::string& normalized_tape_path)
{
  std::string base = replay_filename_component(normalized_tape_path);
  if (ends_with_icase(base, ".tape")) {
    base.resize(base.size() - 5);
  }
  return base;
}

std::string replay_session_id_from_normalized_path(const std::string& normalized_tape_path)
{
  return replay_tape_stem_from_normalized_path(normalized_tape_path);
}

int replay_scan_max_sidecar_index(const std::string& directory, const std::string& stem)
{
  int max_n = 0;
  DIR* dir = opendir(directory.empty() ? "." : directory.c_str());
  if (!dir) {
    return 0;
  }
  struct dirent* entry;
  while ((entry = readdir(dir)) != nullptr) {
    const std::string name = entry->d_name;
    int n = 0;
    if (parse_sidecar_index(stem, name, &n)) {
      if (n > max_n) {
        max_n = n;
      }
    }
  }
  closedir(dir);
  return max_n;
}

std::string replay_allocate_next_sidecar_path(const std::string& normalized_tape_path)
{
  const std::string dir = replay_directory_containing_file(normalized_tape_path);
  const std::string stem = replay_tape_stem_from_normalized_path(normalized_tape_path);
  const int next_n = replay_scan_max_sidecar_index(dir, stem) + 1;
  const std::string name = stem + "." + std::to_string(next_n) + ".sidecar";
  return join_dir_file(dir, name);
}

std::vector<std::pair<int, std::string>> replay_list_sidecars_sorted(const std::string& directory,
                                                                     const std::string& stem)
{
  std::vector<std::pair<int, std::string>> out;
  const std::string scan_dir = directory.empty() ? "." : directory;
  DIR* dir = opendir(scan_dir.c_str());
  if (!dir) {
    return out;
  }
  struct dirent* entry;
  while ((entry = readdir(dir)) != nullptr) {
    const std::string name = entry->d_name;
    int n = 0;
    if (parse_sidecar_index(stem, name, &n)) {
      out.push_back(std::make_pair(n, join_dir_file(scan_dir, name)));
    }
  }
  closedir(dir);
  std::sort(out.begin(), out.end(),
            [](const std::pair<int, std::string>& a, const std::pair<int, std::string>& b) {
              return a.first < b.first;
            });
  return out;
}
