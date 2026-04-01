/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonCompetingClustersOnTrackCreator.h"
#include "MuonCompetingRIOsOnTrack/CompetingMuonClustersOnTrack.h"

#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "TrkPrepRawData/PrepRawData.h"
#include "MuonRIO_OnTrack/MuonClusterOnTrack.h"

#include <list>
#include <vector>


namespace Muon {

  StatusCode MuonCompetingClustersOnTrackCreator::initialize()
  {

    ATH_MSG_VERBOSE("MuonCompetingClustersOnTrackCreator::Initializing");
    ATH_CHECK( m_clusterCreator.retrieve() );

    return StatusCode::SUCCESS;
  }
  
  std::unique_ptr<CompetingMuonClustersOnTrack>
  MuonCompetingClustersOnTrackCreator::createBroadCluster(const std::list< const Trk::PrepRawData * > & prds, const double errorScaleFactor ) const
  {
    if (prds.empty()) {
      ATH_MSG_WARNING("Empty prepdata list");
      return nullptr;
    }
// implement cluster formation
    auto rios = std::vector <const Muon::MuonClusterOnTrack* >() ;
    auto assocProbs = std::vector < double >();
    std::list< const Trk::PrepRawData* >::const_iterator  it = prds.begin();
    std::list< const Trk::PrepRawData* >::const_iterator  it_end = prds.end();
    const double prob = 1./(errorScaleFactor*errorScaleFactor);
    for( ;it!=it_end;++it ){
      Identifier id = (*it)->identify();
      const Trk::TrkDetElementBase* detEl = (*it)->detectorElement();
      const Amg::Vector3D gHitPos = detEl->center(id);
      const Muon::MuonClusterOnTrack* cluster = m_clusterCreator->createRIO_OnTrack( **it, gHitPos );
      if (!cluster) {
          ATH_MSG_WARNING("No cluster..."<<(**it));
          continue;
      }
      rios.push_back( cluster );
      assocProbs.push_back( prob );
    }
    return std::make_unique<CompetingMuonClustersOnTrack>( std::move(rios), std::move(assocProbs) );
  }
}
