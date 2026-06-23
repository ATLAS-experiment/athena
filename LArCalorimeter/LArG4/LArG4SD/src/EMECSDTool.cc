/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "EMECSDTool.h"

namespace LArG4
{

  //---------------------------------------------------------------------------
  // Constructor
  //---------------------------------------------------------------------------
  EMECSDTool::EMECSDTool(const std::string& type, const std::string& name,
                                 const IInterface *parent)
    : SimpleSDTool(type, name, parent)
  {
  }

  //---------------------------------------------------------------------------
  // Initialization of Athena-components
  //---------------------------------------------------------------------------
  StatusCode EMECSDTool::initializeCalculators()
  {
    ATH_CHECK(m_emepiwcalc.retrieve());
    ATH_CHECK(m_emeniwcalc.retrieve());
    ATH_CHECK(m_emepowcalc.retrieve());
    ATH_CHECK(m_emenowcalc.retrieve());
    ATH_CHECK(m_emepscalc.retrieve());
    ATH_CHECK(m_emepobarcalc.retrieve());
    ATH_CHECK(m_emenobarcalc.retrieve());

    return StatusCode::SUCCESS;
  }

  //---------------------------------------------------------------------------
  // Create the SDs for current worker thread
  //---------------------------------------------------------------------------
  G4VSensitiveDetector* EMECSDTool::makeSD() const
  {
    makeOneSD( "LAr::EMEC::Pos::InnerWheel", &*m_emepiwcalc, m_posIWVolumes );
    makeOneSD( "LAr::EMEC::Neg::InnerWheel", &*m_emeniwcalc, m_negIWVolumes );
    makeOneSD( "LAr::EMEC::Pos::OuterWheel", &*m_emepowcalc, m_posOWVolumes );
    makeOneSD( "LAr::EMEC::Neg::OuterWheel", &*m_emenowcalc, m_negOWVolumes );
    makeOneSD( "LAr::Endcap::Presampler::LiquidArgon", &*m_emepscalc, m_presVolumes );
    makeOneSD( "LAr::EMEC::Pos::BackOuterBarrette::Module::Phidiv", &*m_emepobarcalc, m_posBOBVolumes );
    makeOneSD( "LAr::EMEC::Neg::BackOuterBarrette::Module::Phidiv", &*m_emenobarcalc, m_negBOBVolumes );
    return nullptr;
  }

}
