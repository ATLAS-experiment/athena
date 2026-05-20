/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "FCALSDTool.h"

namespace LArG4
{

  //---------------------------------------------------------------------------
  // Constructor
  //---------------------------------------------------------------------------
  FCALSDTool::FCALSDTool(const std::string& type, const std::string& name,
                         const IInterface* parent)
    : SimpleSDTool(type, name, parent)
  {
  }

  //---------------------------------------------------------------------------
  // Initialization of Athena-components
  //---------------------------------------------------------------------------
  StatusCode FCALSDTool::initializeCalculators()
  {
    ATH_CHECK(m_fcal1calc.retrieve());
    ATH_CHECK(m_fcal2calc.retrieve());
    ATH_CHECK(m_fcal3calc.retrieve());

    return StatusCode::SUCCESS;
  }

  //---------------------------------------------------------------------------
  // Create the SDs for current worker thread
  //---------------------------------------------------------------------------
  G4VSensitiveDetector* FCALSDTool::makeSD() const
  {
    makeOneSD("LAr::FCAL::Module1::Gap", &*m_fcal1calc, m_fcal1Volumes);
    makeOneSD("LAr::FCAL::Module2::Gap", &*m_fcal2calc, m_fcal2Volumes);
    makeOneSD("LAr::FCAL::Module3::Gap", &*m_fcal3calc, m_fcal3Volumes);
    return nullptr;
  }

} // namespace LArG4
