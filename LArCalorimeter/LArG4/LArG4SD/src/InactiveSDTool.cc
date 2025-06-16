/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "InactiveSDTool.h"
#include "LArG4Code/SDWrapper.h"

namespace LArG4
{
  //---------------------------------------------------------------------------
  // Constructor
  //---------------------------------------------------------------------------
  InactiveSDTool::InactiveSDTool(const std::string& type, const std::string& name,
                                 const IInterface *parent)
    : CalibSDTool(type, name, parent)
  {
  }

  //---------------------------------------------------------------------------
  // Initialization of Athena-components
  //---------------------------------------------------------------------------
  StatusCode InactiveSDTool::initializeCalculators()
  {
    ATH_CHECK(m_embpscalc.retrieve());
    ATH_CHECK(m_embcalc.retrieve());
    ATH_CHECK(m_emepiwcalc.retrieve());
    ATH_CHECK(m_emepowcalc.retrieve());
    ATH_CHECK(m_emeniwcalc.retrieve());
    ATH_CHECK(m_emenowcalc.retrieve());
    ATH_CHECK(m_heccalc.retrieve());
    ATH_CHECK(m_fcal1calc.retrieve());
    ATH_CHECK(m_fcal2calc.retrieve());
    ATH_CHECK(m_fcal3calc.retrieve());

    return StatusCode::SUCCESS;
  }

  //---------------------------------------------------------------------------
  // Create SD wrapper for current thread
  //---------------------------------------------------------------------------
  G4VSensitiveDetector* InactiveSDTool::makeSD() const
  {
    const std::string deadHitCollName=m_hitCollName+"_DEAD";
    // Create the wrapper
    auto *sdWrapper = new CalibSDWrapper("LArInactiveSDWrapper", m_hitCollName, deadHitCollName);

    sdWrapper->addSD(
      makeOneSD("LAr::Barrel::Presampler::Inactive", &*m_embpscalc, m_barPreVolumes)
    );
    sdWrapper->addSD(
      makeOneSD("LAr::Barrel::Inactive", &*m_embcalc, m_barVolumes)
    );
    sdWrapper->addSD(
      makeOneSD("LAr::EMEC::Pos::InnerWheel::Inactive", &*m_emepiwcalc, m_ECPosInVolumes)
    );
    sdWrapper->addSD(
      makeOneSD("LAr::EMEC::Pos::OuterWheel::Inactive", &*m_emepowcalc, m_ECPosOutVolumes)
    );
    sdWrapper->addSD(
      makeOneSD("LAr::EMEC::Neg::InnerWheel::Inactive", &*m_emeniwcalc, m_ECNegInVolumes)
    );
    sdWrapper->addSD(
      makeOneSD("LAr::EMEC::Neg::OuterWheel::Inactive", &*m_emenowcalc, m_ECNegOutVolumes)
    );

    //sdWrapper->addSD(
    //  makeOneSD("LAr::HEC::Inactive",
    //            new HEC::CalibrationCalculator(HEC::kInactive),
    //            m_HECVolumes)
    //);

    //sdWrapper->addSD(
    //  makeOneSD("LAr::HEC::Local::Inactive",
    //            new HEC::LocalCalibrationCalculator(HEC::kLocInactive),
    //            m_HECLocVolumes)
    //);

    sdWrapper->addSD(
      makeOneSD("LAr::HEC::Wheel::Inactive", &*m_heccalc, m_HECWheelVolumes)
    );
    sdWrapper->addSD(
      makeOneSD("LAr::FCAL::Inactive1", &*m_fcal1calc, m_fcal1Volumes)
    );
    sdWrapper->addSD(
      makeOneSD("LAr::FCAL::Inactive2", &*m_fcal2calc, m_fcal2Volumes)
    );
    sdWrapper->addSD(
      makeOneSD("LAr::FCAL::Inactive3", &*m_fcal3calc, m_fcal3Volumes)
    );

    // Return the wrapper as my SD
    return sdWrapper;
  }

} // namespace LArG4
