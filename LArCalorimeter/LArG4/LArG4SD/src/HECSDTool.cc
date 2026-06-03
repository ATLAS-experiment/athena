/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "HECSDTool.h"

namespace LArG4
{

  //---------------------------------------------------------------------------
  // Constructor
  //---------------------------------------------------------------------------
  HECSDTool::HECSDTool(const std::string& type, const std::string& name,
                       const IInterface* parent)
    : SimpleSDTool(type, name, parent)
  {
  }

  //---------------------------------------------------------------------------
  // Initialization of Athena-components
  //---------------------------------------------------------------------------
  StatusCode HECSDTool::initializeCalculators()
  {
    ATH_CHECK(m_heccalc.retrieve());

    return StatusCode::SUCCESS;
  }

  //---------------------------------------------------------------------------
  // Create the SDs for current worker thread
  //---------------------------------------------------------------------------
  G4VSensitiveDetector* HECSDTool::makeSD() const
  {
    m_heccalc->initializeForSDCreation();
    makeOneSD("LAr::HEC::Module::Depth::Slice::Wheel", &*m_heccalc, m_wheelVolumes);
    return nullptr;
  }

} // namespace LArG4
