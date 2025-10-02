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
#include "MinBiasScintillatorSD.h"

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

StatusCode MinBiasScintillatorSDTool::Gather()
{
  ATH_MSG_VERBOSE( "Gather()" );
  if(!getSD()) {
    ATH_MSG_ERROR ("Gather: MinBiasScintillatorSD never created!");
    return StatusCode::FAILURE;
  } else {
    MinBiasScintillatorSD *localSD = dynamic_cast<MinBiasScintillatorSD*>(getSD());
    if(!localSD){
      ATH_MSG_ERROR ("Gather: Failed to cast m_SD into MinBiasScintillatorSD.");
      return StatusCode::FAILURE;
    }
    localSD->EndOfAthenaEvent();
  }
  return StatusCode::SUCCESS;
}

