/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "./TrackRecordFilter.h"
#include "TruthUtils/HepMCHelpers.h"

TrackRecordFilter::TrackRecordFilter(const std::string& name,
                                     ISvcLocator* pSvcLocator):
  AthReentrantAlgorithm(name, pSvcLocator)
{
}

StatusCode TrackRecordFilter::initialize() {
  ATH_CHECK(m_inputKey.initialize());
  ATH_CHECK(m_outputKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode TrackRecordFilter::execute(const EventContext& ctx) const {

  // Get message service
  ATH_MSG_DEBUG ( "TrackRecordFilter::execute()" );

  // retrieve the collection
  SG::ReadHandle<TrackRecordCollection> trackCollection(m_inputKey, ctx);
  if (!trackCollection.isPresent() || !trackCollection.isValid()) {
    ATH_MSG_DEBUG ( "Could not find TrackRecord collection" );
    return StatusCode::SUCCESS;
  }
  ATH_MSG_DEBUG ( "There are " << trackCollection->size() << "tracks in this container " );

  SG::WriteHandle<TrackRecordCollection> filterCollection(m_outputKey, ctx);
  // create and record a new collection
  CHECK( filterCollection.record(std::make_unique<TrackRecordCollection>()) );

  // iterate over the collection
  for (const auto& trkit : *trackCollection) {
    int pdgId(trkit.GetPDGCode());
    ATH_MSG_VERBOSE ( "Track found with pdg id= " << trkit.GetPDGCode() << " with energy "<< trkit.GetEnergy() );

    if(pdgId) { //Geant makes particle with pdgid=0...
      // get rid of neutral particles
      if (std::fabs(MC::charge(pdgId) ) >0.5 && trkit.GetEnergy() > m_cutOff) {
        filterCollection->push_back(TrackRecord(trkit));
      }
    }
  }


  ATH_MSG_DEBUG ( "There are " << filterCollection->size() << "that satisfy the filter " );
  return StatusCode::SUCCESS;
}

StatusCode TrackRecordFilter::finalize() {
  return StatusCode::SUCCESS;
}

