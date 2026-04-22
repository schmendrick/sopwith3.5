#include "replay_paths.h"

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>

/*
  Cross-language RNG core parity harness (C++ side).

  Why this exists:
  - We need a tiny deterministic executable that isolates replay token normalization,
    seed derivation, and randv step behavior from the full game loop.
  - The same algorithm is implemented in C# and both outputs are compared byte-for-byte
    to prove deterministic parity before/while porting logic across languages.
*/

namespace {

int16_t wrap_int16(int64_t v)
{
  const uint16_t lo = static_cast<uint16_t>(v & 0xffff);
  return static_cast<int16_t>(lo);
}

int16_t seed_from_token(const std::string& token)
{
  const std::string normalized = replay_normalize_replay_token_to_tape_path(token);
  uint32_t h = 2166136261u;
  for (size_t i = 0; i < normalized.size(); ++i) {
    h ^= static_cast<unsigned char>(normalized[i]);
    h *= 16777619u;
  }
  int16_t s = static_cast<int16_t>(h & 0x7fffu);
  if (s == 0) {
    s = 1;
  }
  return s;
}

int16_t legacy_step(int16_t current, int32_t x, int32_t y)
{
  const int16_t next = wrap_int16(static_cast<int64_t>(x) * static_cast<int64_t>(y) *
                                  static_cast<int64_t>(current) + 7491LL);
  if (next == 0) {
    return wrap_int16(74917777LL);
  }
  return next;
}

uint32_t v2_type(int16_t randv, int32_t x, int32_t y, int32_t i)
{
  const int32_t randv1 = static_cast<int32_t>(x * y * randv * 7491);
  const uint16_t u16 = static_cast<uint16_t>(static_cast<uint32_t>(randv1) * static_cast<uint32_t>(i));
  return static_cast<uint32_t>((static_cast<uint32_t>(u16) * 8u) >> 16);
}

uint32_t troubled_sound_bit(int16_t randv)
{
  return static_cast<uint32_t>(randv & 1);
}

uint32_t explosion_type(int16_t randv, int32_t x, int32_t y, int32_t i)
{
  return v2_type(randv, x, y, i);
}

bool parse_args(int argc, char** argv, std::string* token, int* steps)
{
  *steps = 0;
  for (int i = 1; i < argc; ++i) {
    const std::string arg(argv[i]);
    if (arg == "--token" && i + 1 < argc) {
      *token = argv[++i];
      continue;
    }
    if (arg == "--steps" && i + 1 < argc) {
      *steps = std::atoi(argv[++i]);
      continue;
    }
    if (arg == "--help" || arg == "-h") {
      return false;
    }
  }
  return !token->empty() && *steps > 0;
}

} // namespace

int main(int argc, char** argv)
{
  std::string token;
  int steps = 0;
  if (!parse_args(argc, argv, &token, &steps)) {
    std::cerr << "Usage: rng-parity-cpp --token <replay-token> --steps <N>\n";
    return 1;
  }

  const std::string normalized = replay_normalize_replay_token_to_tape_path(token);
  int16_t state = seed_from_token(token);

  std::cout << "token=" << token << "\n";
  std::cout << "normalized=" << normalized << "\n";
  std::cout << "seed=" << state << "\n";
  for (int n = 0; n < steps; ++n) {
    const int32_t x = 100 + 13 * n;
    const int32_t y = 200 + 7 * n;
    const int32_t i = 1 + (n % 15);
    const uint32_t type = v2_type(state, x, y, i);
    const uint32_t troubled = troubled_sound_bit(state);
    const uint32_t explosion = explosion_type(state, x, y, i);
    std::cout << "step=" << n
              << " randv=" << state
              << " x=" << x
              << " y=" << y
              << " i=" << i
              << " v2_type=" << type
              << " troubled_sound_bit=" << troubled
              << " explosion_type=" << explosion
              << "\n";
    state = legacy_step(state, x, y);
  }
  return 0;
}
