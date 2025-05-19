/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonHitTimingTool.h"
#include "MuonRIO_OnTrack/RpcClusterOnTrack.h"
#include "MuonCompetingRIOsOnTrack/CompetingMuonClustersOnTrack.h"


namespace Muon {

  MuonHitTimingTool::MuonHitTimingTool(const std::string& t, const std::string& n, const IInterface* p):
    base_class(t,n,p),
    m_hitTimingTools(this) {
    using namespace MuonStationIndex;
    for( int tech = 0;tech< toInt(TechnologyIndex::TechnologyIndexMax);++tech ){
      if( tech == toInt(TechnologyIndex::RPC) ) m_hitTimingTools.push_back(ToolHandle<IMuonHitTimingTool>("Muon::RPC_TimingTool/RPC_TimingTool"));
      else                                      m_hitTimingTools.push_back(ToolHandle<IMuonHitTimingTool>(""));
    }
    
  }

  StatusCode MuonHitTimingTool::initialize() {

    ATH_CHECK(m_idHelperSvc.retrieve());
    using namespace MuonStationIndex;
    // ensure that the number of tool handles corresponds to the number of technologies
    if( static_cast<int>(m_hitTimingTools.size()) != toInt(TechnologyIndex::TechnologyIndexMax) ){
      ATH_MSG_ERROR(" The MuonHitTimingTool ToolHandleArray SHOULD contain exactly " 
                    << toInt(TechnologyIndex::TechnologyIndexMax) << " ToolHandles (they can be empty). ");
      return StatusCode::FAILURE;
    }

    // loop over timing tool handles and check that they handle the technology they are supposed to, if not return a FAILURE
    for( int tech = 0;tech<toInt(TechnologyIndex::TechnologyIndexMax);++tech ){
        // get handle, accept empty handles
        auto& toolHandle = m_hitTimingTools[tech];
        ATH_MSG_DEBUG(" tech " << technologyName(static_cast<MuonStationIndex::TechnologyIndex>(tech)) << " " << toolHandle);
        if( toolHandle.empty() ) continue;
        ATH_CHECK(toolHandle.retrieve());
    }
    return StatusCode::SUCCESS;
  }

  IMuonHitTimingTool::TimingResult MuonHitTimingTool::calculateTimingResult( const std::vector<const MuonClusterOnTrack*>& hits ) const {
    
    // treat case of no hits and the case the first pointer is zero (should not happen)
    if( hits.empty() || !hits.front()) return {};

    // for now assume that all hits are of the same technolgy
    Identifier id = hits.front()->identify();
    using namespace MuonStationIndex;
    TechnologyIndex tech = m_idHelperSvc->technologyIndex(id);
    
    // get handle and use it if it is not empty
    const ToolHandle<IMuonHitTimingTool>& toolHandle = m_hitTimingTools[toInt(tech)];
    if( toolHandle.empty() ) {
      ATH_MSG_VERBOSE("Unable to fill timing, timing tool missing. Tech = " << technologyName(tech) );
      return {};
    }
    return toolHandle->calculateTimingResult(hits);
  }
}
