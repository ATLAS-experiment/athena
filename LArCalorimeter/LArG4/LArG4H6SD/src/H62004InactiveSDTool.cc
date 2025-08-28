/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "H62004InactiveSDTool.h"

// LArG4 includes
#include "LArG4Code/SDWrapper.h"

namespace LArG4
{

  //---------------------------------------------------------------------------
  // Constructor
  //---------------------------------------------------------------------------
  H62004InactiveSDTool::H62004InactiveSDTool(const std::string& type,
                                             const std::string& name,
                                             const IInterface* parent)
    : H62004CalibSDTool(type, name, parent)
  {
  }

  StatusCode H62004InactiveSDTool::initializeCalculators()
  {
    ATH_CHECK(m_emepiwcalc.retrieve());
    ATH_CHECK(m_heccalc.retrieve());
    ATH_CHECK(m_fcal1calc.retrieve());
    ATH_CHECK(m_fcal2calc.retrieve());
    return StatusCode::SUCCESS;
  }

  //---------------------------------------------------------------------------
  // Create the SD wrapper for current worker thread
  //---------------------------------------------------------------------------
  G4VSensitiveDetector* H62004InactiveSDTool::makeSD() const
  {
    // Create the wrapper
    auto *sdWrapper = new CalibSDWrapper("LArH62004InactiveSDWrapper", m_hitCollName);

    // Add the SDs.
    // Lots of singleton calculators !!!

    if (!m_emecVolumes.value().empty()) {
      sdWrapper->addSD( makeOneSD(
                                  "LAr::EMEC::InnerModule::Inactive::H6", &*m_emepiwcalc, m_emecVolumes.value() ) );
    }
    if (!m_hecVolumes.value().empty()) {
      sdWrapper->addSD( makeOneSD(
                                  "LAr::HEC::Local::Inactive::H6", &*m_heccalc, m_hecVolumes.value() ) );
    }
    if (!m_fcal1Volumes.value().empty()) {
      sdWrapper->addSD( makeOneSD(
                                  "LAr::FCAL::Inactive1::H6", &*m_fcal1calc, m_fcal1Volumes.value() ) );
    }
    if (!m_fcal2Volumes.value().empty()) {
      sdWrapper->addSD( makeOneSD(
                                  "LAr::FCAL::Inactive2::H6", &*m_fcal2calc, m_fcal2Volumes.value() ) );
    }

    // Return the wrapper as my SD
    return sdWrapper;
  }

} // namespace LArG4
