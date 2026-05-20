/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "EMBSDTool.h"

namespace LArG4
{

  //---------------------------------------------------------------------------
  // Constructor
  //---------------------------------------------------------------------------
  EMBSDTool::EMBSDTool(const std::string& type, const std::string& name,
                       const IInterface* parent)
    : SimpleSDTool(type, name, parent)
  {
  }

  //---------------------------------------------------------------------------
  // Initialization of Athena-components
  //---------------------------------------------------------------------------
  StatusCode EMBSDTool::initializeCalculators()
  {
    ATH_CHECK(m_pscalc.retrieve());
    ATH_CHECK(m_embcalc.retrieve());

    return StatusCode::SUCCESS;
  }

  //---------------------------------------------------------------------------
  // Create the SDs for current worker thread
  //---------------------------------------------------------------------------
  G4VSensitiveDetector* EMBSDTool::makeSD() const
  {
    m_pscalc->initializeForSDCreation();
    m_embcalc->initializeForSDCreation();
    makeOneSD("LAr::Barrel::Presampler::Module", &*m_pscalc, m_presVolumes);
    makeOneSD("LAr::EMB::STAC", &*m_embcalc, m_stacVolumes);
    return nullptr;
  }

} // namespace LArG4
