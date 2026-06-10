/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/


// Class header
#include "TrackFastSimSDTool.h"

// Pacakge includes
#include "TrackWriteFastSim/TrackFastSimSD.h"

// Athena includes
#include "HitManagement/HitCollectionMap.h"
#include "TrackRecord/TrackRecordCollection.h"

TrackFastSimSDTool::TrackFastSimSDTool(const std::string& type, const std::string& name, const IInterface* parent)
  : SensitiveDetectorBase( type , name , parent )
{
  m_outputCollectionNames.setValue({"NeutronBG"});
  m_noVolumes.setValue(true);
}

StatusCode TrackFastSimSDTool::SetupEvent(HitCollectionMap& hitCollections)
{
  hitCollections.Emplace<TrackRecordCollection>(m_outputCollectionNames[0],
                                                m_outputCollectionNames[0]);
  return StatusCode::SUCCESS;
}

StatusCode TrackFastSimSDTool::Gather(HitCollectionMap& hitCollections)
{
  CHECK(hitCollections.Record<TrackRecordCollection>(m_outputCollectionNames[0]));
  return StatusCode::SUCCESS;
}

G4VSensitiveDetector* TrackFastSimSDTool::makeSD() const
{
  ATH_MSG_DEBUG( "Initializing SD" );

  // Create a fresh SD
  return new TrackFastSimSD(name(), m_outputCollectionNames[0]);
}
