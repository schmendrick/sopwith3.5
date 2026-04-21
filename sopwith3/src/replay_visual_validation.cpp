#include "replay_visual_validation.h"

#include <fstream>

void replay_visual_log_event(const std::string& event_name, bool ok)
{
  std::ofstream out("replay-visual.log", std::ios::app);
  if (!out.good()) {
    return;
  }
  out << event_name << "=" << (ok ? "ok" : "fail") << "\n";
}

void replay_visual_log_note(const std::string& message)
{
  std::ofstream out("replay-visual.log", std::ios::app);
  if (!out.good()) {
    return;
  }
  out << message << "\n";
}
