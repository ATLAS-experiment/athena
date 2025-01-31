// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

/**
 * @file FPGATrackSimRoadUnionTool.cxx
 * @author Riley Xu - riley.xu@cern.ch
 * @date November 20th, 2020
 * @brief See header file.
 */


#include "FPGATrackSimHough/FPGATrackSimRoadUnionTool.h"
//one of the includes are needed below or all of them dont know for sure
#include "FPGATrackSimObjects/FPGATrackSimTypes.h"
#include "FPGATrackSimObjects/FPGATrackSimConstants.h"
#include "FPGATrackSimConfTools/IFPGATrackSimEventSelectionSvc.h"
#include "FPGATrackSimObjects/FPGATrackSimHit.h"
#include "FPGATrackSimObjects/FPGATrackSimConstants.h"
#include "FPGATrackSimObjects/FPGATrackSimTowerInputHeader.h"
#include "FPGATrackSimMaps/IFPGATrackSimMappingSvc.h"
#include "FPGATrackSimMaps/FPGATrackSimPlaneMap.h"
#include "FPGATrackSimMaps/FPGATrackSimRegionMap.h"
#include "FPGATrackSimBanks/IFPGATrackSimBankSvc.h"
#include "FPGATrackSimBanks/FPGATrackSimSectorBank.h"
#include "FPGATrackSimHoughTransformTool.h"

#include <sstream>
#include <cmath>
#include <algorithm>

FPGATrackSimRoadUnionTool::FPGATrackSimRoadUnionTool(const std::string& algname, const std::string &name, const IInterface *ifc) :
    AthAlgTool(algname,name,ifc),
    m_tools(this)
{
    declareProperty("tools", m_tools, "Array of FPGATrackSimRoadFinderTools");
}


StatusCode FPGATrackSimRoadUnionTool::initialize()
{
    // Retrieve
    ATH_MSG_INFO("Using " << m_tools.size() << " tools");
    ATH_CHECK(m_tools.retrieve());

    if (m_tools.empty()) {
      ATH_MSG_FATAL("initialize() Tool list empty");
      return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
}

StatusCode FPGATrackSimRoadUnionTool::getRoads(const std::vector<std::shared_ptr<const FPGATrackSimHit>> & hits, std::vector<std::shared_ptr<const FPGATrackSimRoad>> & roads) 
{
    ATH_CHECK(m_FPGATrackSimMapping.retrieve());

    // Create one "tower" per slice for this event.
    if (m_slicedHitHeader) {
        for (unsigned ireg = 0; ireg < m_tools.size(); ireg++) {
            FPGATrackSimTowerInputHeader tower = FPGATrackSimTowerInputHeader(ireg);
           m_slicedHitHeader->addTower(tower);
        }
    }

    // We separately need to pass a vector of *pointers* to hit objects to the road finder tools.
    // Makes a vector of slices that have a vector of hits assiociated with that slice
    std::vector<std::vector<std::shared_ptr<const FPGATrackSimHit>>> sliceHits(m_tools.size());
    const FPGATrackSimPlaneMap *pmap = nullptr;
    int toolNum = 0;//same as sliceNum
    for (auto &tool : m_tools) {
        pmap = m_FPGATrackSimMapping->PlaneMap_1st(toolNum);
        auto *subrmap = m_FPGATrackSimMapping->SubRegionMap();
        for (auto & iHit:hits)
        {
            std::shared_ptr<FPGATrackSimHit> hitCopy = std::make_shared<FPGATrackSimHit>(*iHit);
            pmap->map(*hitCopy);
            if ((subrmap->isInRegion(tool->getSubRegion(), *hitCopy)) || m_noHitFilter) {
                // Do we really need to do both of these? can we make the tower class produce a vector of shared pointers?
                if (m_slicedHitHeader) m_slicedHitHeader->getTower(toolNum)->addHit(*hitCopy);
                sliceHits[toolNum].push_back(hitCopy);
            }
        }
        toolNum++;
    }
    roads.clear();
    for (auto & tool : m_tools)
    {
        std::vector<std::shared_ptr<const FPGATrackSimRoad>> r;
        ATH_CHECK(tool->getRoads(sliceHits[tool->getSubRegion()], r, *getTruthTracks()));
        roads.insert(roads.end(), std::make_move_iterator(r.begin()), std::make_move_iterator(r.end()));
    }


    return StatusCode::SUCCESS;
}
