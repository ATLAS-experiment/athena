/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "H62004FCALSDTool.h"

// LArG4 includes
#include "LArG4Code/SDWrapper.h"

namespace LArG4
{

  //---------------------------------------------------------------------------
  // Constructor
  //---------------------------------------------------------------------------
  H62004FCALSDTool::H62004FCALSDTool(const std::string& type, const std::string& name,
                                     const IInterface *parent)
    : H62004SimpleSDTool(type, name, parent)
  {
  }

  StatusCode H62004FCALSDTool::initializeCalculators()
  {
    ATH_CHECK(m_fcal1calc.retrieve());
    ATH_CHECK(m_fcal2calc.retrieve());
    ATH_CHECK(m_fcalcoldcalc.retrieve());
    return StatusCode::SUCCESS;
  }

  //---------------------------------------------------------------------------
  // Create the SD wrapper for current worker thread
  //---------------------------------------------------------------------------
  G4VSensitiveDetector* H62004FCALSDTool::makeSD() const
  {
    // Create the wrapper
    auto *sdWrapper = new SimpleSDWrapper("LArH62004FCALSDWrapper", m_hitCollName);

    // Add the SDs
    sdWrapper->addSD( makeOneSD( "LAr::FCAL::Module1::Gap::H6", &*m_fcal1calc, m_fcal1Volumes.value() ) );
    sdWrapper->addSD( makeOneSD( "LAr::FCAL::Module2::Gap::H6", &*m_fcal2calc, m_fcal2Volumes.value() ) );
    sdWrapper->addSD( makeOneSD( "LAr::FCAL::ColdTC::H6", &*m_fcalcoldcalc, m_fcalColdVolumes.value() ) );

    // Return the wrapper as my SD
    return sdWrapper;
  }

} // namespace LArG4
