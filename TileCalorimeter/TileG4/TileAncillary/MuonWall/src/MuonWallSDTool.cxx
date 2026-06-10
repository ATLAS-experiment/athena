/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

//************************************************************
//
// Class MuonWallSD.
// Sensitive detector for the muon wall
//
// Author: franck Martin <Franck.Martin@cern.ch>
// january 12, 2004
//
//************************************************************

#include "MuonWallSDTool.h"

#include "HitManagement/HitCollectionMap.h"
#include "MuonWallSD.h"
#include "TileSimEvent/TileHitVector.h"

MuonWallSDTool::MuonWallSDTool(const std::string& type, const std::string& name, const IInterface* parent)
    : SensitiveDetectorBase(type, name, parent) {
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

MuonWallSDTool::~MuonWallSDTool() {

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

StatusCode MuonWallSDTool::SetupEvent(HitCollectionMap& hitCollections) {
  ATH_MSG_VERBOSE("MuonWallSDTool::SetupEvent()");
  hitCollections.Emplace<MuonWallSD::HitVectorBuilder>(m_outputCollectionNames[0], m_outputCollectionNames[0]);
  return StatusCode::SUCCESS;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

StatusCode MuonWallSDTool::Gather(HitCollectionMap& hitCollections) {
  ATH_MSG_VERBOSE("MuonWallSDTool::Gather()");
  return hitCollections.TransformAndRecord<TileHitVector>(m_outputCollectionNames[0], [](TileHitVector& hits) {
    static_cast<MuonWallSD::HitVectorBuilder&>(hits).Finalize();
  });
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VSensitiveDetector* MuonWallSDTool::makeSD() const {
  ATH_MSG_DEBUG("Initializing SD");

  int verboseLevel=1;
  if (msgLvl(MSG::VERBOSE))    { verboseLevel = 10; }
  else if (msgLvl(MSG::DEBUG)) { verboseLevel = 5;  }

  // Create a fresh SD
  return new MuonWallSD(name(), m_outputCollectionNames[0], verboseLevel);
}
