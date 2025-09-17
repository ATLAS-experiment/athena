/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MuonPatternHelpers_MuonSegmentFitHelperFunctions__H
#define MuonPatternHelpers_MuonSegmentFitHelperFunctions__H

#include "MuonPatternEvent/MuonHoughDefs.h"


#include "Acts/Seeding/detail/CompSpacePointAuxiliaries.hpp"

namespace MuonR4::SegmentFit {
    /** @brief Converts the 5 segment parameters into the 4-dimensional Line_t parameters */
    constexpr Line_t::ParamVector spatialLinePars(const Parameters& segmentPars); 
}
#include "MuonPatternHelpers/SegmentFitHelperFunctions.icc"
#endif // MUONR4__MuonSegmentFitHelperFunctions__H
