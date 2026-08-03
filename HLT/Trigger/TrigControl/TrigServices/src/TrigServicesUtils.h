/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGSERVICES_TRIGSERVICESUTILS_H
#define TRIGSERVICES_TRIGSERVICESUTILS_H

#include <algorithm>
#include <atomic>
#include <chrono>
#include <thread>

/// Helper to mark unsupported interfaces 
#define NOSUPPORT(lvl, what) \
  do {                                                                         \
    ATH_MSG_LVL(MSG::lvl, what << " is not supported by this implementation"); \
    return {};                                                                 \
  } while (0)

#define NOSUPPORT_VOID(lvl, what) \
  do {                                                                         \
    ATH_MSG_LVL(MSG::lvl, what << " is not supported by this implementation"); \
    return;                                                                    \
  } while (0)

/// Helpers shared by monitoring services 
namespace TrigServices {

  /// Sleep for duration or until stopFlag is set, whichever comes first
  inline void conditionedSleep(std::chrono::milliseconds duration,
                               const std::atomic<bool>& stopFlag)
  {
    const auto start = std::chrono::steady_clock::now();
    while (true) {
      if (stopFlag.load()) {
        return;
      }
      const auto elapsed = std::chrono::steady_clock::now() - start;
      if (elapsed >= duration) {
        break;
      }
      const auto remaining = duration - std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);
      std::this_thread::sleep_for(std::min(std::chrono::milliseconds(500), remaining));
    }
  }

}

#endif // TRIGSERVICES_TRIGSERVICESUTILS_H
