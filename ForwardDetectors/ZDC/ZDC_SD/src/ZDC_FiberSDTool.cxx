/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


// Class header
#include "ZDC_FiberSDTool.h"

// For the SD itself
#include "ZDC_FiberSD.h"
#include "HitManagement/HitCollectionMap.h"
#include "ZDC_HitCollectionBuilders.h"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

ZDC_FiberSDTool::ZDC_FiberSDTool(const std::string& type, const std::string& name, const IInterface* parent)
  : SensitiveDetectorBase(type,name,parent)
{
  declareProperty("readoutPos",m_readoutPos = 511.8);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

StatusCode ZDC_FiberSDTool::SetupEvent(HitCollectionMap& hitCollections)
{
  hitCollections.Emplace<ZDC_SimFiberHitCollectionBuilder>(m_outputCollectionNames[0],
                                                           m_outputCollectionNames[0]);
  return StatusCode::SUCCESS;
}

StatusCode ZDC_FiberSDTool::Gather(HitCollectionMap& hitCollections)
{
  ATH_MSG_VERBOSE( "ZDC_FiberSDTool::Gather()" );
  auto* hitCollection = hitCollections.Find<ZDC_SimFiberHitCollectionBuilder>(m_outputCollectionNames[0]);
  if (!hitCollection) {
    return StatusCode::FAILURE;
  }
  hitCollection->Finalize();
  CHECK(hitCollections.Record<ZDC_SimFiberHit_Collection>(m_outputCollectionNames[0]));
  return StatusCode::SUCCESS;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VSensitiveDetector* ZDC_FiberSDTool::makeSD() const
{
  ATH_MSG_DEBUG( "Initializing ZDC_FiberSD" );

  return new ZDC_FiberSD(name(), m_outputCollectionNames[0], m_readoutPos);
}
