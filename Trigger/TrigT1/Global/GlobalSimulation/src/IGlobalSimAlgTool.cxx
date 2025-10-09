/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "IGlobalSimAlgTool.h"

namespace GlobalSim {
  // default do nothing update of the TIP word. To be used by
  // IGlobalSimAlgTools which do not report to the TIP.
  StatusCode IGlobalSimAlgTool::updateTIP(std::bitset<s_nbits_TIP>&,
					  const EventContext&) const {
    return StatusCode::SUCCESS;
  }
}
