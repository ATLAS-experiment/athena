/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

//###############################################
//   BLM Sensitive Detector tool
//
//###############################################

// Class header
#include "BLMSensorSDTool.h"

// Package headers
#include "BLMSensorSD.h"
#include "HitManagement/HitCollectionMap.h"
#include "InDetSimEvent/SiHitCollection.h"


BLMSensorSDTool::BLMSensorSDTool(const std::string& type, const std::string& name, const IInterface *parent)
  : SensitiveDetectorBase(type,name,parent)
{
}

StatusCode BLMSensorSDTool::SetupEvent(HitCollectionMap& hitCollections)
{
  hitCollections.Emplace<SiHitCollection>(m_outputCollectionNames[0]);
  return StatusCode::SUCCESS;
}

StatusCode BLMSensorSDTool::Gather(HitCollectionMap& hitCollections)
{
  hitCollections.Record<SiHitCollection>(m_outputCollectionNames[0]);
  return StatusCode::SUCCESS;
}

G4VSensitiveDetector* BLMSensorSDTool::makeSD() const
{
  ATH_MSG_DEBUG( "Initializing SD" );

  // Create a fresh SD
  return new BLMSensorSD(name(), m_outputCollectionNames[0]);
}
