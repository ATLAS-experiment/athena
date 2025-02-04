// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#include "FPGATrackSimGNNPatternRecoTool.h"

///////////////////////////////////////////////////////////////////////////////
// AthAlgTool

FPGATrackSimGNNPatternRecoTool::FPGATrackSimGNNPatternRecoTool(const std::string& algname, const std::string &name, const IInterface *ifc) :
  base_class(algname, name, ifc)
{
  declareInterface<IFPGATrackSimRoadFinderTool>(this);
}

StatusCode FPGATrackSimGNNPatternRecoTool::getRoads(const std::vector<std::shared_ptr<const FPGATrackSimHit>> & hits, std::vector<std::shared_ptr<const FPGATrackSimRoad>> & roads)
{
    ATH_MSG_DEBUG("hits.size() = " << hits.size()); // Temporary to avoid warnings in build
    roads.clear(); // Temporary to avoid warnings in build
    return StatusCode::SUCCESS;
}

