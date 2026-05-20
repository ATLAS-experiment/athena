/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "ActiveSDTool.h"

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

  std::string ActiveSDTool::hitCollectionName() const
  {
    return m_hitCollName;
  }

  std::string ActiveSDTool::deadHitCollectionName() const
  {
    return m_hitCollName.value() + "_DEAD";
  }

  std::string ActiveSDTool::srHitCollectionName() const
  {
    return m_outputCollectionNames.size() > 1
      ? m_outputCollectionNames[1]
      : "SR_" + hitCollectionName();
  }

  //---------------------------------------------------------------------------
  // Create SDs for current thread
  //---------------------------------------------------------------------------
  G4VSensitiveDetector* ActiveSDTool::makeSD() const
  {
    makeOneSD( "Barrel::Presampler::Module::Calibration", &*m_bpsmodcalc, m_presBarVolumes );
    makeOneSD( "EMB::STAC::Calibration", &*m_embcalc, m_stacVolumes );
    makeOneSD( "EMEC::Pos::InnerWheel::Calibration", &*m_emepiwcalc, m_posIWVolumes );
    makeOneSD( "EMEC::Neg::InnerWheel::Calibration", &*m_emeniwcalc, m_negIWVolumes );
    makeOneSD( "EMEC::Pos::OuterWheel::Calibration", &*m_emepowcalc, m_posOWVolumes );
    makeOneSD( "EMEC::Neg::OuterWheel::Calibration", &*m_emenowcalc, m_negOWVolumes );
    makeOneSD( "Endcap::Presampler::LiquidArgon::Calibration", &*m_emepscalc, m_presECVolumes );
    makeOneSD( "EMEC::Pos::BackOuterBarrette::Calibration", &*m_emepobarcalc, m_pBOBVolumes );
    makeOneSD( "EMEC::Neg::BackOuterBarrette::Calibration", &*m_emenobarcalc, m_nBOBVolumes );
    makeOneSD( "FCAL::Module1::Gap::Calibration", &*m_fcal1calc, m_fcal1Volumes );
    makeOneSD( "FCAL::Module2::Gap::Calibration", &*m_fcal2calc, m_fcal2Volumes );
    makeOneSD( "FCAL::Module3::Gap::Calibration", &*m_fcal3calc, m_fcal3Volumes );
    makeOneSD( "HEC::Module::Depth::Slice::Wheel::Calibration", &*m_heccalc, m_sliceVolumes );
    return nullptr;
  }

} // namespace LArG4
