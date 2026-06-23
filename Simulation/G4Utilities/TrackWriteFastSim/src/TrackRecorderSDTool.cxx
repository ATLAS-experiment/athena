/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

// Track Recorder Sensitive Detector Tool.
//

// class header
#include "TrackRecorderSDTool.h"

// package includes
#include "TrackWriteFastSim/TrackFastSimSD.h"

// Athena includes
#include "HitManagement/HitCollectionMap.h"
#include "TrackRecord/TrackRecordCollection.h"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

TrackRecorderSDTool::TrackRecorderSDTool(const std::string& type, const std::string& name, const IInterface* parent)
  : SensitiveDetectorBase( type , name , parent )
  , m_SD_type(1)
{
  declareProperty("SD_type", m_SD_type);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

StatusCode TrackRecorderSDTool::SetupEvent(HitCollectionMap& hitCollections)
{
  hitCollections.Emplace<TrackRecordCollection>(m_outputCollectionNames[0],
                                                m_outputCollectionNames[0]);
  return StatusCode::SUCCESS;
}

StatusCode TrackRecorderSDTool::Gather(HitCollectionMap& hitCollections)
{
  CHECK(hitCollections.Record<TrackRecordCollection>(m_outputCollectionNames[0]));
  return StatusCode::SUCCESS;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VSensitiveDetector* TrackRecorderSDTool::makeSD() const
{
  ATH_MSG_DEBUG( "Initializing SD" );
  //SD name has to be the same as the collection name for now in order
  //to fit with how the old configuration works...
  return new TrackFastSimSD(m_outputCollectionNames[0], m_outputCollectionNames[0], m_SD_type);
}
