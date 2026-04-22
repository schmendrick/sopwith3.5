#ifndef SOPWITH_REPLAY_PATHS_H
#define SOPWITH_REPLAY_PATHS_H

#include <string>
#include <utility>
#include <vector>

/** Normalize `-h`/`-v` replay token to canonical `.../basename.tape` per spec (strip `.tape`/`.rec` loops, append `.tape`). */
std::string replay_normalize_replay_token_to_tape_path(const std::string& replay_token);

/** Directory containing the file `filepath` (`"."` if no separator). */
std::string replay_directory_containing_file(const std::string& filepath);

/** Basename of `filepath` without directory prefix. */
std::string replay_filename_component(const std::string& filepath);

/** Stem used for sidecars: normalized tape basename without trailing `.tape` (case-insensitive strip). */
std::string replay_tape_stem_from_normalized_path(const std::string& normalized_tape_path);

/** Session/session_id style label: same as tape stem from normalized path. */
std::string replay_session_id_from_normalized_path(const std::string& normalized_tape_path);

/** Maximum positive integer `n` among existing `stem.n.sidecar` in `directory`, or 0 if none. */
int replay_scan_max_sidecar_index(const std::string& directory, const std::string& stem);

/** Full path for next sidecar: `stem.(max+1).sidecar` beside `normalized_tape_path`. */
std::string replay_allocate_next_sidecar_path(const std::string& normalized_tape_path);

/** Entries sorted by numeric `n` ascending; paths are relative or as stored (join dir + name). */
std::vector<std::pair<int, std::string>> replay_list_sidecars_sorted(const std::string& directory,
                                                                     const std::string& stem);

#endif
