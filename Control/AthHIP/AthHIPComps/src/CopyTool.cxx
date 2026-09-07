//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//

// Local include(s).
#include "CopyTool.h"

namespace AthHIP {

std::shared_ptr<const vecmem::copy> CopyTool::copy(const EventContext&) const {

  return m_copy;
}

}  // namespace AthHIP
