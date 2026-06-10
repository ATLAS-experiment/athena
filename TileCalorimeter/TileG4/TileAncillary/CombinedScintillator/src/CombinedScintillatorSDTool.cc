/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

//************************************************************
//
// Class CombinedScintillatorSDTool
// Sensitive detector for the Scintillator between LAR & Tile
//
//************************************************************

#include "CombinedScintillatorSDTool.hh"

#include "CombinedScintillatorSD.hh"
#include "HitManagement/HitCollectionMap.h"
#include "TileSimEvent/TileHitVector.h"

CombinedScintillatorSDTool::CombinedScintillatorSDTool(const std::string& type, const std::string& name,
                                                       const IInterface* parent)
    : SensitiveDetectorBase(type, name, parent)
{
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

CombinedScintillatorSDTool::~CombinedScintillatorSDTool() {

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

StatusCode CombinedScintillatorSDTool::SetupEvent(HitCollectionMap& hitCollections) {
  ATH_MSG_VERBOSE("CombinedScintillatorSDTool::SetupEvent()");
  hitCollections.Emplace<CombinedScintillatorSD::HitVectorBuilder>(m_outputCollectionNames[0], m_outputCollectionNames[0]);
  return StatusCode::SUCCESS;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

StatusCode CombinedScintillatorSDTool::Gather(HitCollectionMap& hitCollections) {
  ATH_MSG_VERBOSE("CombinedScintillatorSDTool::Gather()");
  return hitCollections.TransformAndRecord<TileHitVector>(m_outputCollectionNames[0], [](TileHitVector& hits) {
    static_cast<CombinedScintillatorSD::HitVectorBuilder&>(hits).Finalize();
  });
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VSensitiveDetector* CombinedScintillatorSDTool::makeSD() const {
  ATH_MSG_DEBUG("Initializing SD");
  // Create a fresh SD
  return new CombinedScintillatorSD(name(), m_outputCollectionNames[0]);
}
