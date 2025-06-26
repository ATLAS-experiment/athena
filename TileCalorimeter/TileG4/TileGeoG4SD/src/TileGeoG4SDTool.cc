/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//************************************************************
//
// Class TileGeoG4SDTool
// AthTool class for holding the Tile sensitive detector
//
//************************************************************

#include "TileGeoG4SDTool.h"

#include <memory>

#include "TileGeoG4SD.hh"
#include "TileGeoG4SD/TileHitVectorBuilder.hh"

#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "HitManagement/HitCollectionMap.h"

TileGeoG4SDTool::TileGeoG4SDTool(const std::string& type, const std::string& name, const IInterface* parent)
  : SensitiveDetectorBase(type,name,parent)
{
}

StatusCode TileGeoG4SDTool::initialize()
{
  ATH_CHECK(m_tileCalculator.retrieve());
  return StatusCode::SUCCESS;
}

StatusCode TileGeoG4SDTool::SetupEvent(HitCollectionMap& hitCollections)
{
  ATH_MSG_VERBOSE( "Setting up Tile hits for event");
  hitCollections.Emplace<TileHitVectorBuilder>(m_outputCollectionNames[0], m_outputCollectionNames[0], m_tileCalculator->GetLookupBuilder());
  return StatusCode::SUCCESS;
}

StatusCode TileGeoG4SDTool::Gather(HitCollectionMap& hitCollections)
{
  hitCollections.TransformAndRecord<TileHitVector>(m_outputCollectionNames[0], [](TileHitVector& hits){
    // Because ISF transports multiple G4Event per Athena event, ResetCells must be called here.
    // Once we have a one-to-one G4Event to Athena event mapping, this should be moved to G4VSensitiveDetector::EndOfEvent
    static_cast<TileHitVectorBuilder&>(hits).ResetCells();
  });
  return StatusCode::SUCCESS;
}


G4VSensitiveDetector* TileGeoG4SDTool::makeSD() const
{
  // Make sure the job has been set up properly
  ATH_MSG_DEBUG( "Initializing SD" );

  // Create a fresh SD
  return new TileGeoG4SD(name(), m_outputCollectionNames[0], &*m_tileCalculator);
}

