/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//************************************************************
//
// Class MinBiasScintillatorSDTool
// Tool for configuring the Sensitive detector for the Minimum Bias Scintillator
//
//************************************************************

#include "MinBiasScintillatorSDTool.h"

#include "HitManagement/HitCollectionMap.h"
#include "MinBiasScintillatorSD.h"
#include "TileSimEvent/TileHitVector.h"

MinBiasScintillatorSDTool::MinBiasScintillatorSDTool(const std::string& type, const std::string& name, const IInterface *parent)
  : SensitiveDetectorBase( type , name , parent )
{
}

StatusCode MinBiasScintillatorSDTool::initialize()
{
  m_options.deltaTHit = m_deltaTHit.value();
  m_options.timeCut = m_timeCut.value();
  m_options.tileTB = m_tileTB.value();
  m_options.doBirk = m_doBirk.value();
  m_options.birk1 = m_birk1.value();
  m_options.birk2 = m_birk2.value();
  m_options.doTOFCorrection = m_doTOFCorrection.value();

  return StatusCode::SUCCESS;
}

G4VSensitiveDetector* MinBiasScintillatorSDTool::makeSD() const
{
  ATH_MSG_VERBOSE( "Creating a copy of the MinBiasScintillatorSD!" );

  return new MinBiasScintillatorSD(name(), m_outputCollectionNames[0], m_options);
}

StatusCode MinBiasScintillatorSDTool::SetupEvent(HitCollectionMap& hitCollections)
{
  ATH_MSG_VERBOSE( "SetupEvent()" );
  hitCollections.Emplace<MinBiasScintillatorSD::HitVectorBuilder>(m_outputCollectionNames[0], m_outputCollectionNames[0]);
  return StatusCode::SUCCESS;
}

StatusCode MinBiasScintillatorSDTool::Gather(HitCollectionMap& hitCollections)
{
  ATH_MSG_VERBOSE( "Gather()" );
  return hitCollections.TransformAndRecord<TileHitVector>(m_outputCollectionNames[0], [](TileHitVector& hits) {
    static_cast<MinBiasScintillatorSD::HitVectorBuilder&>(hits).Finalize();
  });
}
