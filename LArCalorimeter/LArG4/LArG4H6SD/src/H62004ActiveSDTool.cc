/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "H62004ActiveSDTool.h"

// LArG4 includes
#include "LArG4Code/SDWrapper.h"

namespace LArG4
{

  //---------------------------------------------------------------------------
  // Constructor
  //---------------------------------------------------------------------------
  H62004ActiveSDTool::H62004ActiveSDTool(const std::string& type, const std::string& name,
                                         const IInterface *parent)
    : H62004CalibSDTool(type, name, parent)
    , m_hitCollName("LArCalibrationHitActive")
  {
  }

  StatusCode H62004ActiveSDTool::initializeCalculators()
  {
    ATH_CHECK(m_emepiwcalc.retrieve());
    ATH_CHECK(m_heccalc.retrieve());
    ATH_CHECK(m_fcal1calc.retrieve());
    ATH_CHECK(m_fcal2calc.retrieve());
    ATH_CHECK(m_fcalcoldcalc.retrieve());
    return StatusCode::SUCCESS;
  }

  //---------------------------------------------------------------------------
  // Create the SD wrapper for current worker thread
  //---------------------------------------------------------------------------
  G4VSensitiveDetector* H62004ActiveSDTool::makeSD() const
  {
    // Create the wrapper
    auto *sdWrapper = new CalibSDWrapper("LArH62004ActiveSDWrapper", m_hitCollName);

    // Add the SDs.
    // Lots of singleton calculators !!!

    if (!m_emecVolumes.value().empty()) {
      sdWrapper->addSD( makeOneSD(
                                  "EMEC::InnerModule::Calibration::H6", &*m_emepiwcalc, m_emecVolumes.value() ) );
    }
    if (!m_hecVolumes.value().empty()) {
      sdWrapper->addSD( makeOneSD(
                                  "HEC::Module::Depth::Slice::Local::Calibration::H6", &*m_heccalc, m_hecVolumes.value() ) );
    }
    if (!m_fcal1Volumes.value().empty()) {
      sdWrapper->addSD( makeOneSD(
                                  "LAr::FCAL::Module1::Gap::Calibration::H6", &*m_fcal1calc, m_fcal1Volumes.value() ) );
    }
    if (!m_fcal2Volumes.value().empty()) {
      sdWrapper->addSD( makeOneSD(
                                  "LAr::FCAL::Module2::Gap::Calibration::H6", &*m_fcal2calc, m_fcal2Volumes.value() ) );
    }
    if (!m_fcalColdVolumes.value().empty()) {
      sdWrapper->addSD( makeOneSD(
                                  "LAr::FCAL::ColdTC::Gap::Calibration::H6", &*m_fcalcoldcalc, m_fcalColdVolumes.value() ) );
    }

    // Return the wrapper as my SD
    return sdWrapper;
  }

} // namespace LArG4
