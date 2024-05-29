/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Zhengcheng Tao

#include <AsgAnalysisAlgorithms/IOStatsAlg.h>

#include "xAODCore/tools/IOStats.h"
#include "xAODCore/tools/ReadStats.h"

namespace CP {

  StatusCode IOStatsAlg::finalize() {

    xAOD::IOStats::instance().merged().Print(m_printOption.value().c_str());

    return StatusCode::SUCCESS;
  }

} // namespace
