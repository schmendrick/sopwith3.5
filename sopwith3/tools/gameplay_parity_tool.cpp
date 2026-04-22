#include "replay_paths.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

const char* kToolId = "gameplay-parity-harness 0.1.0";

struct TapeRef {
  std::string logical_name;
  std::string path;
};

struct MatrixCase {
  std::string case_id;
  std::string token;
  int steps = 0;
  std::vector<TapeRef> tape_refs;
};

std::string trim_ws(const std::string& s)
{
  std::size_t a = 0;
  while (a < s.size() && std::isspace(static_cast<unsigned char>(s[a]))) {
    ++a;
  }
  std::size_t b = s.size();
  while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1]))) {
    --b;
  }
  return s.substr(a, b - a);
}

bool read_entire_file(const std::string& path, std::string* out)
{
  std::ifstream in(path.c_str(), std::ios::in | std::ios::binary);
  if (!in.good()) {
    return false;
  }
  std::ostringstream buf;
  buf << in.rdbuf();
  *out = buf.str();
  return true;
}

std::string join_path(const std::string& dir, const std::string& name)
{
  if (dir.empty() || dir == ".") {
    return name;
  }
  if (!dir.empty() && (dir.back() == '/' || dir.back() == '\\')) {
    return dir + name;
  }
  return dir + "/" + name;
}

std::string resolve_tape_path(const std::string& path)
{
  if (path.find('/') != std::string::npos || path.find('\\') != std::string::npos) {
    return path;
  }
  return join_path(".", path);
}

std::string extract_json_string_after_key(const std::string& blob, const std::string& key, std::size_t search_from)
{
  const std::string needle = "\"" + key + "\"";
  std::size_t p = blob.find(needle, search_from);
  if (p == std::string::npos) {
    return std::string();
  }
  p = blob.find(':', p + needle.size());
  if (p == std::string::npos) {
    return std::string();
  }
  p = blob.find('"', p + 1);
  if (p == std::string::npos) {
    return std::string();
  }
  const std::size_t start = p + 1;
  const std::size_t end = blob.find('"', start);
  if (end == std::string::npos) {
    return std::string();
  }
  return blob.substr(start, end - start);
}

int extract_json_int_after_key(const std::string& blob, const std::string& key, std::size_t search_from)
{
  const std::string needle = "\"" + key + "\"";
  std::size_t p = blob.find(needle, search_from);
  if (p == std::string::npos) {
    return -1;
  }
  p = blob.find(':', p + needle.size());
  if (p == std::string::npos) {
    return -1;
  }
  ++p;
  while (p < blob.size() && std::isspace(static_cast<unsigned char>(blob[p]))) {
    ++p;
  }
  std::size_t q = p;
  while (q < blob.size() && (blob[q] == '-' || (blob[q] >= '0' && blob[q] <= '9'))) {
    ++q;
  }
  if (q == p) {
    return -1;
  }
  return std::atoi(blob.substr(p, q - p).c_str());
}

bool parse_matrix(const std::string& blob, std::string* matrix_version, std::vector<MatrixCase>* cases)
{
  *matrix_version = extract_json_string_after_key(blob, "matrix_version", 0);
  if (matrix_version->empty()) {
    return false;
  }

  const std::string needle = "\"case_id\"";
  std::size_t cursor = 0;
  while (true) {
    const std::size_t cpos = blob.find(needle, cursor);
    if (cpos == std::string::npos) {
      break;
    }
    MatrixCase c;
    c.case_id = extract_json_string_after_key(blob, "case_id", cpos);
    c.token = extract_json_string_after_key(blob, "token", cpos);
    c.steps = extract_json_int_after_key(blob, "steps", cpos);
    if (c.case_id.empty() || c.token.empty() || c.steps <= 0) {
      return false;
    }

    std::size_t tr_key = blob.find("\"tape_refs\"", cpos);
    if (tr_key == std::string::npos) {
      return false;
    }
    std::size_t lb = blob.find('[', tr_key);
    std::size_t rb = blob.find(']', lb == std::string::npos ? cpos : lb);
    if (lb == std::string::npos || rb == std::string::npos || rb <= lb) {
      return false;
    }
    const std::string arr = blob.substr(lb + 1, rb - lb - 1);
    if (trim_ws(arr).empty()) {
      // no tapes
    } else {
      std::size_t a = 0;
      while (true) {
        const std::size_t obj0 = arr.find('{', a);
        if (obj0 == std::string::npos) {
          break;
        }
        const std::size_t obj1 = arr.find('}', obj0);
        if (obj1 == std::string::npos) {
          return false;
        }
        const std::string obj = arr.substr(obj0, obj1 - obj0 + 1);
        TapeRef t;
        t.logical_name = extract_json_string_after_key(obj, "logical_name", 0);
        t.path = extract_json_string_after_key(obj, "path", 0);
        if (t.logical_name.empty() || t.path.empty()) {
          return false;
        }
        c.tape_refs.push_back(t);
        a = obj1 + 1;
      }
    }

    std::sort(c.tape_refs.begin(), c.tape_refs.end(), [](const TapeRef& lhs, const TapeRef& rhs) {
      return lhs.logical_name < rhs.logical_name;
    });

    cases->push_back(c);
    cursor = rb + 1;
  }

  return !cases->empty();
}

int16_t seed_from_token(const std::string& token)
{
  const std::string normalized = replay_normalize_replay_token_to_tape_path(token);
  uint32_t h = 2166136261u;
  for (std::size_t i = 0; i < normalized.size(); ++i) {
    h ^= static_cast<unsigned char>(normalized[i]);
    h *= 16777619u;
  }
  int16_t s = static_cast<int16_t>(h & 0x7fffu);
  if (s == 0) {
    s = 1;
  }
  return s;
}

uint32_t prng(uint32_t x)
{
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  return x;
}

struct MockObj {
  int id = 0;
  int x_int = 0;
  int y_int = 0;
  int angle = 0;
  std::string kind;
};

bool mock_less(const MockObj& a, const MockObj& b)
{
  if (a.x_int != b.x_int) {
    return a.x_int < b.x_int;
  }
  if (a.y_int != b.y_int) {
    return a.y_int < b.y_int;
  }
  if (a.angle != b.angle) {
    return a.angle < b.angle;
  }
  if (a.kind != b.kind) {
    return a.kind < b.kind;
  }
  return a.id < b.id;
}

std::string mock_sort_key(const MockObj& o)
{
  std::ostringstream out;
  out << "x=" << o.x_int << "|y=" << o.y_int << "|angle=" << o.angle << "|kind=" << o.kind << "|id=" << o.id;
  return out.str();
}

uint32_t rotr32(uint32_t x, unsigned n)
{
  return (x >> n) | (x << (32u - n));
}

void sha256_of_bytes(const std::string& bytes, std::string* out_hex)
{
  static const uint32_t k[64] = {
      0x428a2f98u, 0x71374491u, 0xb5c0fbcfu, 0xe9b5dba5u, 0x3956c25bu, 0x59f111f1u, 0x923f82a4u, 0xab1c5ed5u, 0xd807aa98u, 0x12835b01u, 0x243185beu, 0x550c7dc3u, 0x72be5d74u, 0x80deb1feu, 0x9bdc06a7u, 0xc19bf174u,
      0xe49b69c1u, 0xefbe4786u, 0x0fc19dc6u, 0x240ca1ccu, 0x2de92c6fu, 0x4a7484aau, 0x5cb0a9dcu, 0x76f988dau, 0x983e5152u, 0xa831c66du, 0xb00327c8u, 0xbf597fc7u, 0xc6e00bf3u, 0xd5a79147u, 0x06ca6351u, 0x14292967u,
      0x27b70a85u, 0x2e1b2138u, 0x4d2c6dfcu, 0x53380d13u, 0x650a7354u, 0x766a0abbu, 0x81c2c92eu, 0x92722c85u, 0xa2bfe8a1u, 0xa81a664bu, 0xc24b8b70u, 0xc76c51a3u, 0xd192e819u, 0xd6990624u, 0xf40e3585u, 0x106aa070u,
      0x19a4c116u, 0x1e376c08u, 0x2748774cu, 0x34b0bcb5u, 0x391c0cb3u, 0x4ed8aa4au, 0x5b9cca4fu, 0x682e6ff3u, 0x748f82eeu, 0x78a5636fu, 0x84c87814u, 0x8cc70208u, 0x90befffau, 0xa4506cebu, 0xbef9a3f7u, 0xc67178f2u};

  uint32_t h0 = 0x6a09e667u;
  uint32_t h1 = 0xbb67ae85u;
  uint32_t h2 = 0x3c6ef372u;
  uint32_t h3 = 0xa54ff53au;
  uint32_t h4 = 0x510e527fu;
  uint32_t h5 = 0x9b05688cu;
  uint32_t h6 = 0x1f83d9abu;
  uint32_t h7 = 0x5be0cd19u;

  const uint64_t bitlen = static_cast<uint64_t>(bytes.size()) * 8ULL;
  std::string buf = bytes;
  buf.push_back(static_cast<char>(0x80));
  while ((buf.size() % 64u) != 56u) {
    buf.push_back('\0');
  }
  for (int i = 7; i >= 0; --i) {
    buf.push_back(static_cast<char>((bitlen >> (static_cast<unsigned>(i) * 8u)) & 255u));
  }

  for (std::size_t offset = 0; offset + 64u <= buf.size(); offset += 64u) {
    uint32_t w[64];
    for (int i = 0; i < 16; ++i) {
      const unsigned char* p = reinterpret_cast<const unsigned char*>(buf.data() + offset + static_cast<std::size_t>(i) * 4u);
      w[i] = (static_cast<uint32_t>(p[0]) << 24) | (static_cast<uint32_t>(p[1]) << 16) | (static_cast<uint32_t>(p[2]) << 8) |
             static_cast<uint32_t>(p[3]);
    }
    for (int i = 16; i < 64; ++i) {
      const uint32_t s0 = rotr32(w[i - 15], 7u) ^ rotr32(w[i - 15], 18u) ^ (w[i - 15] >> 3);
      const uint32_t s1 = rotr32(w[i - 2], 17u) ^ rotr32(w[i - 2], 19u) ^ (w[i - 2] >> 10);
      w[i] = w[i - 16] + s0 + w[i - 7] + s1;
    }

    uint32_t a = h0;
    uint32_t b = h1;
    uint32_t c = h2;
    uint32_t d = h3;
    uint32_t e = h4;
    uint32_t f = h5;
    uint32_t g = h6;
    uint32_t h = h7;

    for (int i = 0; i < 64; ++i) {
      const uint32_t S1 = rotr32(e, 6u) ^ rotr32(e, 11u) ^ rotr32(e, 25u);
      const uint32_t ch = (e & f) ^ ((~e) & g);
      const uint32_t t1 = h + S1 + ch + k[i] + w[i];
      const uint32_t S0 = rotr32(a, 2u) ^ rotr32(a, 13u) ^ rotr32(a, 22u);
      const uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
      const uint32_t t2 = S0 + maj;
      h = g;
      g = f;
      f = e;
      e = d + t1;
      d = c;
      c = b;
      b = a;
      a = t1 + t2;
    }

    h0 += a;
    h1 += b;
    h2 += c;
    h3 += d;
    h4 += e;
    h5 += f;
    h6 += g;
    h7 += h;
  }

  static const char* hexd = "0123456789abcdef";
  out_hex->clear();
  out_hex->reserve(64);
  const uint32_t hv[8] = {h0, h1, h2, h3, h4, h5, h6, h7};
  for (int i = 0; i < 8; ++i) {
    for (int j = 3; j >= 0; --j) {
      const unsigned char byte = static_cast<unsigned char>((hv[i] >> (j * 8)) & 255u);
      out_hex->push_back(hexd[(byte >> 4) & 15u]);
      out_hex->push_back(hexd[byte & 15u]);
    }
  }
}

bool sha256_file_hex(const std::string& path, std::string* out_hex)
{
  std::ifstream in(path.c_str(), std::ios::in | std::ios::binary);
  if (!in.good()) {
    return false;
  }
  std::string bytes;
  {
    std::ostringstream buf;
    buf << in.rdbuf();
    bytes = buf.str();
  }
  sha256_of_bytes(bytes, out_hex);
  return true;
}

void emit_headers(const std::string& matrix_version,
                  const std::string& case_id,
                  const std::string& token,
                  const std::string& normalized,
                  int steps,
                  const std::vector<TapeRef>& tape_refs)
{
  std::cout << "tool=" << kToolId << "\n";
  std::cout << "matrix_version=" << matrix_version << "\n";
  std::cout << "case_id=" << case_id << "\n";
  std::cout << "token=" << token << "\n";
  std::cout << "normalized=" << normalized << "\n";
  std::cout << "steps=" << steps << "\n";
  if (tape_refs.empty()) {
    std::cout << "tape_ref=none\n";
    return;
  }
  for (std::size_t i = 0; i < tape_refs.size(); ++i) {
    const std::string resolved = resolve_tape_path(tape_refs[i].path);
    std::string hex;
    if (!sha256_file_hex(resolved, &hex)) {
      std::cerr << "Failed to read tape file for sha256: " << resolved << "\n";
      std::exit(2);
    }
    std::cout << "tape_ref=" << tape_refs[i].logical_name << "|path=" << resolved << "|sha256=" << hex << "\n";
  }
}

void run_case(const MatrixCase& mc, const std::string& matrix_version_cli)
{
  const std::string normalized = replay_normalize_replay_token_to_tape_path(mc.token);
  const int16_t seed = seed_from_token(mc.token);
  if (matrix_version_cli.empty()) {
    std::cerr << "Missing matrix_version\n";
    std::exit(1);
  }

  emit_headers(matrix_version_cli, mc.case_id, mc.token, normalized, mc.steps, mc.tape_refs);

  std::vector<std::string> decisions;

  const int steps = mc.steps;
  int prev_collided = 0;
  int obj_life = 3;
  int player_score = static_cast<int>(static_cast<uint16_t>(seed)) % 1000;
  int player_lives = 3;
  int enemy_score = static_cast<int>(static_cast<uint16_t>(seed ^ 0x1357)) % 500;
  int enemy_lives = 3;

  for (int s = 0; s < steps; ++s) {
    const uint32_t mix = prng(static_cast<uint32_t>(static_cast<uint16_t>(seed)) ^ static_cast<uint32_t>(s * 1315423911u));

    if (s == 0) {
      decisions.push_back("decision logical_step=0 category=spawn_lifecycle key=entity_created entity_id=100 kind=Mock");
      decisions.push_back("decision logical_step=0 category=spawn_lifecycle key=entity_created entity_id=101 kind=Mock");
    }
    if (steps > 0 && s == steps - 1) {
      std::ostringstream line;
      line << "decision logical_step=" << s << " category=spawn_lifecycle key=entity_destroyed entity_id=100 kind=Mock";
      decisions.push_back(line.str());
    }

    const int cur_collided = static_cast<int>(mix & 1u);
    if (s > 0 && cur_collided != prev_collided) {
      std::ostringstream line;
      line << "decision logical_step=" << s << " category=collision_outcome key=plane_collided plane=player prev=" << prev_collided << " next=" << cur_collided;
      decisions.push_back(line.str());
    }
    prev_collided = cur_collided;

    const int prev_life = obj_life;
    if (s % 3 == 0 && obj_life > 0) {
      obj_life -= 1;
    }
    if (prev_life > 0 && obj_life <= 0) {
      std::ostringstream line;
      line << "decision logical_step=" << s << " category=collision_outcome key=object_life entity_id=200 prev_life=" << prev_life << " new_life=" << obj_life << " crossed=1";
      decisions.push_back(line.str());
    }

    const int prev_ps = player_score;
    const int prev_pl = player_lives;
    const int prev_es = enemy_score;
    const int prev_el = enemy_lives;

    if ((mix % 4u) == 0u) {
      player_score += static_cast<int>((mix % 5u)) - 2;
    }
    if (s > 0 && (s % 11) == 0 && player_lives > 0) {
      player_lives -= 1;
    }
    if ((mix % 6u) == 0u) {
      enemy_score += static_cast<int>((mix >> 3) % 3) - 1;
    }
    if (s > 0 && (s % 13) == 0 && enemy_lives > 0) {
      enemy_lives -= 1;
    }

    const int dps = player_score - prev_ps;
    const int dpl = player_lives - prev_pl;
    if (dps != 0 || dpl != 0) {
      std::ostringstream line;
      line << "decision logical_step=" << s << " category=score_lives_delta key=player score_delta=" << dps << " lives_delta=" << dpl;
      decisions.push_back(line.str());
    }
    const int des = enemy_score - prev_es;
    const int del = enemy_lives - prev_el;
    if (des != 0 || del != 0) {
      std::ostringstream line;
      line << "decision logical_step=" << s << " category=score_lives_delta key=enemy score_delta=" << des << " lives_delta=" << del;
      decisions.push_back(line.str());
    }

    MockObj o1;
    o1.id = 1;
    o1.x_int = 10 + ((static_cast<int>(mix) % 50));
    o1.y_int = 20 + ((static_cast<int>(mix) >> 3) % 40);
    o1.angle = (s * 7) % 360;
    o1.kind = "Bomb";
    MockObj o2;
    o2.id = 2;
    o2.x_int = o1.x_int;
    o2.y_int = o1.y_int + 1 + (static_cast<int>(mix) % 3);
    o2.angle = (s * 5) % 360;
    o2.kind = "Bird";
    std::vector<MockObj> objs;
    objs.push_back(o1);
    objs.push_back(o2);
    std::sort(objs.begin(), objs.end(), mock_less);
    std::ostringstream keys;
    for (std::size_t i = 0; i < objs.size(); ++i) {
      if (i) {
        keys << ";";
      }
      keys << mock_sort_key(objs[i]);
    }
    {
      std::ostringstream line;
      line << "decision logical_step=" << s << " category=replay_entity_order key=object_sort_keys keys=" << keys.str();
      decisions.push_back(line.str());
    }
    {
      std::ostringstream line;
      line << "decision logical_step=" << s << " category=replay_entity_order key=entity_row_order rows=PLAYER|ENEMY|OBJECT|OBJECT";
      decisions.push_back(line.str());
    }
  }

  std::sort(decisions.begin(), decisions.end());
  for (std::size_t i = 0; i < decisions.size(); ++i) {
    std::cout << decisions[i] << "\n";
  }
}

bool parse_cli(int argc,
               char** argv,
               std::string* case_id,
               std::string* matrix_version,
               std::string* token,
               int* steps,
               std::string* matrix_file)
{
  *steps = 0;
  for (int i = 1; i < argc; ++i) {
    const std::string a(argv[i]);
    if (a == "--case" && i + 1 < argc) {
      *case_id = argv[++i];
      continue;
    }
    if (a == "--matrix-version" && i + 1 < argc) {
      *matrix_version = argv[++i];
      continue;
    }
    if (a == "--token" && i + 1 < argc) {
      *token = argv[++i];
      continue;
    }
    if (a == "--steps" && i + 1 < argc) {
      *steps = std::atoi(argv[++i]);
      continue;
    }
    if (a == "--matrix-file" && i + 1 < argc) {
      *matrix_file = argv[++i];
      continue;
    }
    if (a == "--help" || a == "-h") {
      return false;
    }
  }
  return !case_id->empty() && !matrix_version->empty() && !token->empty() && *steps > 0;
}

} // namespace

int main(int argc, char** argv)
{
  std::string case_id;
  std::string matrix_version_cli;
  std::string token_cli;
  int steps_cli = 0;
  std::string matrix_file = "tools/gameplay-parity-matrix.json";
  if (!parse_cli(argc, argv, &case_id, &matrix_version_cli, &token_cli, &steps_cli, &matrix_file)) {
    std::cerr << "Usage: gameplay-parity-cpp --case <id> --matrix-version <ver> --token <replay-token> --steps <N> [--matrix-file <path>]\n";
    return 1;
  }

  std::string blob;
  if (!read_entire_file(matrix_file, &blob)) {
    std::cerr << "Failed to read matrix file: " << matrix_file << "\n";
    return 2;
  }

  std::string matrix_version_file;
  std::vector<MatrixCase> cases;
  if (!parse_matrix(blob, &matrix_version_file, &cases)) {
    std::cerr << "Failed to parse matrix file: " << matrix_file << "\n";
    return 2;
  }
  if (matrix_version_file != matrix_version_cli) {
    std::cerr << "matrix_version mismatch (cli=" << matrix_version_cli << " file=" << matrix_version_file << ")\n";
    return 2;
  }

  MatrixCase chosen;
  bool found = false;
  for (std::size_t i = 0; i < cases.size(); ++i) {
    if (cases[i].case_id == case_id) {
      chosen = cases[i];
      found = true;
      break;
    }
  }
  if (!found) {
    std::cerr << "Unknown case_id: " << case_id << "\n";
    return 2;
  }
  if (chosen.token != token_cli || chosen.steps != steps_cli) {
    std::cerr << "CLI token/steps do not match matrix case row\n";
    return 2;
  }

  run_case(chosen, matrix_version_cli);
  return 0;
}
