//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//

// Local include(s).
#include "HostCopyTool.h"

namespace AthDevice {

std::shared_ptr<const vecmem::copy> HostCopyTool::copy(
    const EventContext&) const {

  return m_copy;
}

std::string_view HostCopyTool::flavor() const {
  return "Host";
}

}  // namespace AthDevice
