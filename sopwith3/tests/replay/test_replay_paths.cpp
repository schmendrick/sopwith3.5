#include "../../src/replay_paths.h"

#include <cstdio>
#include <fstream>
#include <string>

static int check(bool cond)
{
  return cond ? 0 : 1;
}

int main()
{
  int failures = 0;

  failures += check(replay_normalize_replay_token_to_tape_path("short") == "short.tape");
  failures += check(replay_normalize_replay_token_to_tape_path("short.rec") == "short.tape");
  failures += check(replay_normalize_replay_token_to_tape_path("short.tape") == "short.tape");
  failures += check(replay_normalize_replay_token_to_tape_path("C:\\runs\\demo.rec") == "C:\\runs\\demo.tape");

  failures += check(replay_tape_stem_from_normalized_path("foo.tape") == "foo");
  failures += check(replay_session_id_from_normalized_path("subdir\\bar.tape") == "bar");

  failures += check(replay_directory_containing_file("x.tape") == ".");
  failures += check(replay_directory_containing_file("d/x.tape") == "d");

  const std::string stem = "rp_path_test_z9";
  const std::string f3 = stem + ".3.sidecar";
  const std::string f10 = stem + ".10.sidecar";
  {
    std::ofstream a(f3.c_str());
    std::ofstream b(f10.c_str());
  }
  failures += check(replay_scan_max_sidecar_index(".", stem) == 10);

  std::remove(f3.c_str());
  std::remove(f10.c_str());

  const std::string stem2 = "rp_list_z8";
  {
    std::ofstream a((stem2 + ".2.sidecar").c_str());
    std::ofstream b((stem2 + ".1.sidecar").c_str());
  }
  const std::vector<std::pair<int, std::string>> lst = replay_list_sidecars_sorted(".", stem2);
  failures += check(lst.size() == 2);
  failures += check(lst[0].first == 1);
  failures += check(lst[1].first == 2);
  std::remove((stem2 + ".1.sidecar").c_str());
  std::remove((stem2 + ".2.sidecar").c_str());

  const std::string norm_only = replay_normalize_replay_token_to_tape_path("rp_alloc_fn");
  failures += check(replay_allocate_next_sidecar_path(norm_only) == "rp_alloc_fn.1.sidecar");

  return failures;
}
