/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

//************************************************************
//
// Class TileGeoG4CalibSDTool.
// AthTool class for holding the Calibration Sensitive Detector for TileCal
//
//************************************************************

#include "TileGeoG4CalibSDTool.h"

#include <memory>
#include <string>
#include <utility>

#include "TileGeoG4CalibSD.h"
#include "TileHitVectorDMBuilder.h"

#include "CaloSimEvent/CaloCalibrationHitContainer.h"
#include "HitManagement/HitCollectionMap.h"
#include "StoreGate/WriteHandle.h"
#include "TileG4Interfaces/ITileCalculator.h"
#include "TileSimEvent/TileHitVector.h"

TileGeoG4CalibSDTool::TileGeoG4CalibSDTool(const std::string& type, const std::string& name, const IInterface* parent)
  : SensitiveDetectorBase(type, name, parent)
  , m_tileCalculator("TileGeoG4SDCalc", name)
{
  declareProperty("TileCalculator", m_tileCalculator);
}

StatusCode TileGeoG4CalibSDTool::initialize()
{
  ATH_CHECK(m_tileCalculator.retrieve());
  m_tileHits = m_outputCollectionNames[0];
  m_tileActiveCellCalibHits = m_outputCollectionNames[1];
  m_tileInactiveCellCalibHits = m_outputCollectionNames[2];
  m_tileDeadMaterialCalibHits = m_outputCollectionNames[3];
  return StatusCode::SUCCESS;
}

G4VSensitiveDetector* TileGeoG4CalibSDTool::makeSD() const {
  if (m_outputCollectionNames.size() < 4) {
    ATH_MSG_ERROR( "Expected 4 output collection names, found " << m_outputCollectionNames.size()
                   << ". Expect the job to crash when the SD is created.");
  }
  return new TileGeoG4CalibSD(name(), m_outputCollectionNames, &*m_tileCalculator, detStore());
}

StatusCode TileGeoG4CalibSDTool::SetupEvent(HitCollectionMap& hitCollections) {
  ATH_MSG_VERBOSE("Setting up Tile calibration hits for event");

  hitCollections.SetSDHitCollection(m_tileHits, std::make_unique<TileHitVectorDMBuilder>(m_tileHits, m_tileCalculator->GetLookupBuilder()));

  auto set_hitcollection = [&hitCollections](const std::string& collectionName) {
    hitCollections.SetSDHitCollection(collectionName, std::make_unique<CaloCalibrationHitContainer>(collectionName));
  };
  set_hitcollection(m_tileActiveCellCalibHits);
  set_hitcollection(m_tileInactiveCellCalibHits);
  set_hitcollection(m_tileDeadMaterialCalibHits);

  return StatusCode::SUCCESS;
}

StatusCode TileGeoG4CalibSDTool::Gather(HitCollectionMap& hitCollections) {

  {  // WriteHandle scope
    SG::WriteHandle<TileHitVector> handle(m_tileHits);
    auto hitColl = hitCollections.ExtractSDHitCollection<TileHitVectorDMBuilder>(m_tileHits);
    hitColl->ResetCells();
    handle = std::move(hitColl);
  }

  auto extract_hits = [&hitCollections](const std::string& collectionName) {
    SG::WriteHandle<CaloCalibrationHitContainer> handle(collectionName);
    handle = hitCollections.ExtractSDHitCollection<CaloCalibrationHitContainer>(collectionName);
  };
  extract_hits(m_tileActiveCellCalibHits);
  extract_hits(m_tileInactiveCellCalibHits);
  extract_hits(m_tileDeadMaterialCalibHits);
  return StatusCode::SUCCESS;
}

