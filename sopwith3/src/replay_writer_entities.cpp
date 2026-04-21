#include "replay_writer.h"
#include "replay_writer_entities.h"

#include "bird.h"
#include "bomb.h"
#include "bullet.h"
#include "computer.h"
#include "def.h"
#include "flock.h"
#include "frag.h"
#include "ox.h"
#include "player.h"
#include "remote.h"
#include "smoke.h"
#include "sopwith.h"
#include "target.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
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

static std::string replay_object_kind_name(const Object* obj)
{
  if (obj == 0) {
    return "Unknown";
  }
  if (dynamic_cast<const Frag*>(obj) != 0) {
    return "Frag";
  }
  if (dynamic_cast<const Smoke*>(obj) != 0) {
    return "Smoke";
  }
  if (dynamic_cast<const Bomb*>(obj) != 0) {
    return "Bomb";
  }
  if (dynamic_cast<const Bullet*>(obj) != 0) {
    return "Bullet";
  }
  if (dynamic_cast<const Bird*>(obj) != 0) {
    return "Bird";
  }
  if (dynamic_cast<const Flock*>(obj) != 0) {
    return "Flock";
  }
  if (dynamic_cast<const Ox*>(obj) != 0) {
    return "Ox";
  }
  const Target* tgt = dynamic_cast<const Target*>(obj);
  if (tgt != 0) {
    switch (tgt->type) {
      case Target::FLAG:
        return "Flag";
      case Target::CHIMNEY:
        return "Chimney";
      case Target::FUEL:
        return "Fuel";
      case Target::TANK:
        return "Tank";
      default:
        return "Target";
    }
  }
  return "Unknown";
}

static std::string replay_sort_kind_key(Object* obj)
{
  Plane* pl = dynamic_cast<Plane*>(obj);
  if (pl != 0) {
    if (dynamic_cast<Playerplane*>(pl) != 0) {
      return "Playerplane";
    }
    if (dynamic_cast<Computerplane*>(pl) != 0) {
      return "Computerplane";
    }
    if (dynamic_cast<Remoteplane*>(pl) != 0) {
      return "Remoteplane";
    }
    return "Plane";
  }
  return replay_object_kind_name(obj);
}

static bool replay_object_less(Object* a, Object* b)
{
  if (a->x.integer != b->x.integer) {
    return a->x.integer < b->x.integer;
  }
  if (a->y.integer != b->y.integer) {
    return a->y.integer < b->y.integer;
  }
  if (a->angle != b->angle) {
    return a->angle < b->angle;
  }
  std::string ka = replay_sort_kind_key(a);
  std::string kb = replay_sort_kind_key(b);
  if (ka != kb) {
    return ka < kb;
  }
  return reinterpret_cast<std::uintptr_t>(a) < reinterpret_cast<std::uintptr_t>(b);
}

static void append_field(std::ostringstream& o, const char* key, int v)
{
  o << key << "=" << v << "|";
}

static void append_field_bool(std::ostringstream& o, const char* key, bool v)
{
  o << key << "=" << (v ? 1 : 0) << "|";
}

static void append_field_string(std::ostringstream& o, const char* key, const std::string& v)
{
  o << key << "=" << v << "|";
}

static bool replay_emit_hitcounter_for_object_kind(const std::string& kind)
{
  if (kind == "Smoke" || kind == "Bullet" || kind == "Bomb" || kind == "Bird" || kind == "Flock" || kind == "Ox") {
    return false;
  }
  return true;
}

static std::string replay_payload_for_sidecar_object(Object* obj)
{
  const std::string kind = replay_object_kind_name(obj);
  std::ostringstream o;
  append_field_string(o, "object_kind", kind);

  append_field(o, "angle", obj->angle);
  append_field_bool(o, "firing", obj->firing);
  if (replay_emit_hitcounter_for_object_kind(kind)) {
    append_field(o, "hitcounter", obj->hitcounter);
  }
  append_field_bool(o, "inverted", obj->inverted);
  append_field(o, "life", obj->life);
  append_field(o, "speed", obj->speed);
  append_field(o, "width", obj->width);
  append_field(o, "height", obj->height);
  append_field(o, "colour", obj->colour);
  append_field(o, "x_frac", obj->x.frac);
  append_field(o, "x_int", obj->x.integer);
  append_field(o, "y_frac", obj->y.frac);
  append_field(o, "y_int", obj->y.integer);

  std::string s = o.str();
  if (!s.empty() && s[s.size() - 1] == '|') {
    s.erase(s.size() - 1);
  }
  return s;
}

static std::string replay_payload_for_object(Object* obj)
{
  std::ostringstream o;
  append_field(o, "angle", obj->angle);
  append_field_bool(o, "firing", obj->firing);
  append_field(o, "hitcounter", obj->hitcounter);
  append_field_bool(o, "inverted", obj->inverted);
  append_field(o, "life", obj->life);
  append_field(o, "speed", obj->speed);
  append_field(o, "width", obj->width);
  append_field(o, "height", obj->height);
  append_field(o, "colour", obj->colour);
  append_field(o, "x_frac", obj->x.frac);
  append_field(o, "x_int", obj->x.integer);
  append_field(o, "y_frac", obj->y.frac);
  append_field(o, "y_int", obj->y.integer);

  Plane* pl = dynamic_cast<Plane*>(obj);
  if (pl != 0) {
    append_field(o, "ammo", pl->ammo);
    append_field(o, "bombs", pl->bombs);
    append_field_bool(o, "collided", pl->collided);
    append_field(o, "endcounter", pl->endcounter);
    append_field(o, "endstatus", static_cast<int>(pl->endstatus));
    append_field(o, "lives", pl->lives);
    append_field(o, "maxlives", pl->maxlives);
    append_field(o, "plane_state", static_cast<int>(pl->state));
    append_field(o, "score", static_cast<int>(pl->score));
  }

  std::string s = o.str();
  if (!s.empty() && s[s.size() - 1] == '|') {
    s.erase(s.size() - 1);
  }
  return s;
}

static void append_ground_line(int frame_index)
{
  std::string line;
  line.reserve(static_cast<std::size_t>(MAX_X) * 14u + 48u);
  char head[48];
  std::snprintf(head, sizeof head, "GROUND|frame_index=%d", frame_index);
  line.append(head);
  for (int i = 0; i < MAX_X; ++i) {
    char chunk[32];
    std::snprintf(chunk, sizeof chunk, "|z%04d=%d", i, static_cast<int>(ground[i]));
    line.append(chunk);
  }
  replay_writer_emit_line(line);
}

void replay_write_logical_frame_snapshot(int frame_index)
{
  if (!replay_writer_is_open()) {
    return;
  }

  std::vector<Object*> players;
  std::vector<Object*> enemies;
  std::vector<Object*> objects;

  for (std::list<Object*>::iterator it = objectlist.begin(); it != objectlist.end(); ++it) {
    Object* obj = *it;
    Plane* pl = dynamic_cast<Plane*>(obj);
    if (pl != 0) {
      if (dynamic_cast<Playerplane*>(pl) != 0) {
        players.push_back(obj);
      }
      else {
        enemies.push_back(obj);
      }
    }
    else {
      objects.push_back(obj);
    }
  }

  std::sort(players.begin(), players.end(), replay_object_less);
  std::sort(enemies.begin(), enemies.end(), replay_object_less);
  std::sort(objects.begin(), objects.end(), replay_object_less);

  replay_write_frame_begin(frame_index);
  replay_write_frame_row(frame_index);
  append_ground_line(frame_index);

  std::vector<ReplayEntityRow> player_rows;
  for (std::size_t i = 0; i < players.size(); ++i) {
    ReplayEntityRow row;
    row.entity_id = static_cast<int>(i);
    row.payload = replay_payload_for_object(players[i]);
    player_rows.push_back(row);
  }
  player_rows = replay_sort_by_entity(player_rows);
  for (std::size_t i = 0; i < player_rows.size(); ++i) {
    replay_writer_emit_line(replay_build_entity_payload("PLAYER", player_rows[i].entity_id, player_rows[i].payload));
  }

  std::vector<ReplayEntityRow> enemy_rows;
  for (std::size_t i = 0; i < enemies.size(); ++i) {
    ReplayEntityRow row;
    row.entity_id = static_cast<int>(i);
    row.payload = replay_payload_for_object(enemies[i]);
    enemy_rows.push_back(row);
  }
  enemy_rows = replay_sort_by_entity(enemy_rows);
  for (std::size_t i = 0; i < enemy_rows.size(); ++i) {
    replay_writer_emit_line(replay_build_entity_payload("ENEMY", enemy_rows[i].entity_id, enemy_rows[i].payload));
  }

  std::vector<ReplayEntityRow> object_rows;
  for (std::size_t i = 0; i < objects.size(); ++i) {
    ReplayEntityRow row;
    row.entity_id = static_cast<int>(i);
    row.payload = replay_payload_for_sidecar_object(objects[i]);
    object_rows.push_back(row);
  }
  object_rows = replay_sort_by_entity(object_rows);
  for (std::size_t i = 0; i < object_rows.size(); ++i) {
    replay_writer_emit_line(replay_build_entity_payload("OBJECT", object_rows[i].entity_id, object_rows[i].payload));
  }

  replay_write_frame_end(frame_index);
}
