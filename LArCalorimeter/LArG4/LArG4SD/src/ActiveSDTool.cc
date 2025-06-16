/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "ActiveSDTool.h"

#include "LArG4Code/SDWrapper.h"


namespace LArG4
{

  //---------------------------------------------------------------------------
  // Constructor
  //---------------------------------------------------------------------------
  ActiveSDTool::ActiveSDTool(const std::string& type, const std::string& name,
                             const IInterface *parent)
    : CalibSDTool(type, name, parent)
  {
  }

  //---------------------------------------------------------------------------
  // Initialization of Athena-components
  //---------------------------------------------------------------------------
  StatusCode ActiveSDTool::initializeCalculators()
  {
    // Lots of calculators !!!
    ATH_CHECK(m_bpsmodcalc.retrieve());
    ATH_CHECK(m_embcalc.retrieve());
    ATH_CHECK(m_emepiwcalc.retrieve());
    ATH_CHECK(m_emeniwcalc.retrieve());
    ATH_CHECK(m_emepowcalc.retrieve());
    ATH_CHECK(m_emenowcalc.retrieve());
    ATH_CHECK(m_emepscalc.retrieve());
    ATH_CHECK(m_emepobarcalc.retrieve());
    ATH_CHECK(m_emenobarcalc.retrieve());
    ATH_CHECK(m_heccalc.retrieve());
    ATH_CHECK(m_fcal1calc.retrieve());
    ATH_CHECK(m_fcal2calc.retrieve());
    ATH_CHECK(m_fcal3calc.retrieve());

    return StatusCode::SUCCESS;
  }

  //---------------------------------------------------------------------------
  // Create SD wrapper for current thread
  //---------------------------------------------------------------------------
  G4VSensitiveDetector* ActiveSDTool::makeSD() const
  {
    const std::string deadHitCollName=m_hitCollName+"_DEAD";
    // Create the wrapper
    auto *sdWrapper = new CalibSDWrapper("LArActiveSDWrapper", m_hitCollName, deadHitCollName);

    // Create the SDs.
    sdWrapper->addSD( makeOneSD( "Barrel::Presampler::Module::Calibration", &*m_bpsmodcalc, m_presBarVolumes ) );
    sdWrapper->addSD( makeOneSD( "EMB::STAC::Calibration", &*m_embcalc, m_stacVolumes ) );
    sdWrapper->addSD( makeOneSD( "EMEC::Pos::InnerWheel::Calibration", &*m_emepiwcalc, m_posIWVolumes ) );
    sdWrapper->addSD( makeOneSD( "EMEC::Neg::InnerWheel::Calibration", &*m_emeniwcalc, m_negIWVolumes ) );
    sdWrapper->addSD( makeOneSD( "EMEC::Pos::OuterWheel::Calibration", &*m_emepowcalc, m_posOWVolumes ) );
    sdWrapper->addSD( makeOneSD( "EMEC::Neg::OuterWheel::Calibration", &*m_emenowcalc, m_negOWVolumes ) );
    sdWrapper->addSD( makeOneSD( "Endcap::Presampler::LiquidArgon::Calibration", &*m_emepscalc, m_presECVolumes ) );
    sdWrapper->addSD( makeOneSD( "EMEC::Pos::BackOuterBarrette::Calibration", &*m_emepobarcalc, m_pBOBVolumes ) );
    sdWrapper->addSD( makeOneSD( "EMEC::Neg::BackOuterBarrette::Calibration", &*m_emenobarcalc, m_nBOBVolumes ) );
    sdWrapper->addSD( makeOneSD( "FCAL::Module1::Gap::Calibration", &*m_fcal1calc, m_fcal1Volumes ) );
    sdWrapper->addSD( makeOneSD( "FCAL::Module2::Gap::Calibration", &*m_fcal2calc, m_fcal2Volumes ) );
    sdWrapper->addSD( makeOneSD( "FCAL::Module3::Gap::Calibration", &*m_fcal3calc, m_fcal3Volumes ) );
    sdWrapper->addSD( makeOneSD( "HEC::Module::Depth::Slice::Wheel::Calibration", &*m_heccalc, m_sliceVolumes ) );

    return sdWrapper;
  }

} // namespace LArG4
