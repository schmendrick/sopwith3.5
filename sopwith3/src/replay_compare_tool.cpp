#include "replay_compare.h"
#include "replay_paths.h"

#include <iostream>
#include <string>
#include <vector>

static void print_loaded_sidecars_stdout_line1(const std::vector<std::pair<int, std::string>>& list)
{
  for (size_t i = 0; i < list.size(); ++i) {
    if (i > 0) {
      std::cout << " ";
    }
    std::cout << list[i].second;
  }
  std::cout << "\n";
}

int main(int argc, char** argv)
{
  if (argc != 2 && argc != 3) {
    std::cerr << "Usage: replay-compare <left.sidecar> <right.sidecar>\n";
    std::cerr << "       replay-compare <basename>\n";
    return 2;
  }

  if (argc == 3) {
    ReplayCompareResult result = replay_compare_files(argv[1], argv[2]);
    replay_compare_print_result(result);
    if (result.status == REPLAY_COMPARE_SUCCESS) {
      return 0;
    }
    return 1;
  }

  const std::string basename = argv[1];
  if (basename.empty() || basename.find_first_of("/\\") != std::string::npos) {
    std::cerr << "replay-compare: basename argument must not be empty or contain path separators.\n";
    return 2;
  }

  const std::vector<std::pair<int, std::string>> list = replay_list_sidecars_sorted(".", basename);

  print_loaded_sidecars_stdout_line1(list);

  if (list.size() == 2) {
    ReplayCompareResult result = replay_compare_files(list[0].second, list[1].second);
    replay_compare_print_result(result);
    if (result.status == REPLAY_COMPARE_SUCCESS) {
      return 0;
    }
    return 1;
  }

  if (list.size() < 2) {
    std::cerr << "replay-compare: found " << list.size()
              << " sidecar(s); need exactly two matching files " << basename << ".<n>.sidecar for automatic comparison "
              << "(or pass two explicit artifact paths).\n";
    return 1;
  }

  std::cerr << "replay-compare: found " << list.size()
            << " sidecars; exactly two matches required for automatic compare. "
               "Pick two paths and invoke: replay-compare <left.sidecar> <right.sidecar>\n";
  return 1;
}
