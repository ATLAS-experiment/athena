/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

//************************************************************
//
// Class PhantomBarrelSD.
// Tool for configuring the sensitive detector for the phantom calorimeter in combined 2004
//
//************************************************************

#include "PhantomBarrelSDTool.hh"

#include "HitManagement/HitCollectionMap.h"
#include "PhantomBarrelSD.hh"
#include "TileSimEvent/TileHitVector.h"

PhantomBarrelSDTool::PhantomBarrelSDTool(const std::string& type, const std::string& name, const IInterface* parent)
    : SensitiveDetectorBase(type, name, parent) {
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PhantomBarrelSDTool::~PhantomBarrelSDTool() {

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

StatusCode PhantomBarrelSDTool::SetupEvent(HitCollectionMap& hitCollections) {
  ATH_MSG_VERBOSE("PhantomBarrelSDTool::SetupEvent()");
  hitCollections.Emplace<PhantomBarrelSD::HitVectorBuilder>(m_outputCollectionNames[0], m_outputCollectionNames[0]);
  return StatusCode::SUCCESS;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

StatusCode PhantomBarrelSDTool::Gather(HitCollectionMap& hitCollections) {
  ATH_MSG_VERBOSE("PhantomBarrelSDTool::Gather()");
  return hitCollections.TransformAndRecord<TileHitVector>(m_outputCollectionNames[0], [](TileHitVector& hits) {
    static_cast<PhantomBarrelSD::HitVectorBuilder&>(hits).Finalize();
  });
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VSensitiveDetector* PhantomBarrelSDTool::makeSD() const {
  ATH_MSG_DEBUG("Initializing SD");
  // Create a fresh SD
  return new PhantomBarrelSD(name(), m_outputCollectionNames[0]);
}
