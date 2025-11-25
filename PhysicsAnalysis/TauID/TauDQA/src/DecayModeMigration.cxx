/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "DecayModeMigration.h"

namespace Tau{

  DecayModeMigration::DecayModeMigration(PlotBase* pParent, const std::string& sDir, std::string sTauJetContainerName):
    PlotBase(pParent, sDir),
    m_sTauJetContainerName(std::move(sTauJetContainerName))
  {
  }

  DecayModeMigration::~DecayModeMigration()
  {
  }

  void DecayModeMigration::initializePlots()
  {
    m_migration_panTau = Book1D("panTau_migration",m_sTauJetContainerName + " panTau migration",DECAYSIZE,0,DECAYSIZE);
    m_migration_panTauProto = Book1D("panTauProto_migration",m_sTauJetContainerName + " panTau proto migration",DECAYSIZE,0,DECAYSIZE);
    m_migration_panTau->GetXaxis()->SetLabelSize(0.05);
    m_migration_panTauProto->GetXaxis()->SetLabelSize(0.05);
    for(int i=1; i<= DECAYSIZE;i++){
      m_migration_panTauProto->GetXaxis()->SetBinLabel(i,m_lable[i-1]);
      m_migration_panTau->GetXaxis()->SetBinLabel(i,m_lable[i-1]);
    }
  }


  void DecayModeMigration::fill(const xAOD::TauJet& thisTau, xAOD::TauJetParameters::DecayMode trueMode, float weight)
  {
    int isPanTauCandidate = 0;
    bool foundDetail = thisTau.panTauDetail(xAOD::TauJetParameters::PanTauDetails::PanTau_isPanTauCandidate, isPanTauCandidate);
    if ( !foundDetail || !isPanTauCandidate ) return;

    // panTau
    int recMode = xAOD::TauJetParameters::DecayMode::Mode_Error;
    foundDetail = thisTau.panTauDetail(xAOD::TauJetParameters::PanTauDetails::PanTau_DecayMode, recMode);
    if ( foundDetail ) decayModeFill(trueMode, recMode, m_migration_panTau, weight);

    // panTauProto
    recMode = xAOD::TauJetParameters::DecayMode::Mode_Error;
    foundDetail = thisTau.panTauDetail(xAOD::TauJetParameters::PanTauDetails::PanTau_DecayModeProto, recMode);
    if ( foundDetail ) {
      decayModeFill(trueMode, recMode, m_migration_panTauProto, weight);
    }

  }

  void DecayModeMigration::decayModeFill(int trueMode, int recMode, TH1 *histo, float weight)
  {
    if ( recMode >= xAOD::TauJetParameters::DecayMode::Mode_Other || trueMode >= xAOD::TauJetParameters::DecayMode::Mode_Other ) return;
   
    switch ( trueMode ) {
    case xAOD::TauJetParameters::DecayMode::Mode_1p0n:
      switch ( recMode ) {
      case xAOD::TauJetParameters::DecayMode::Mode_1p0n:
	histo->Fill(t10r10 + 0.5, weight);
	break;
      case xAOD::TauJetParameters::DecayMode::Mode_1p1n:
	histo->Fill(t10r11 + 0.5, weight);
	break;
      case xAOD::TauJetParameters::DecayMode::Mode_1pXn:
	histo->Fill(t10r1x + 0.5, weight);
	break;
      default:
	histo->Fill(t1r3 + 0.5, weight);
	break;
      }
      break;
    case xAOD::TauJetParameters::DecayMode::Mode_1p1n:
      switch ( recMode ) {
      case xAOD::TauJetParameters::DecayMode::Mode_1p0n:
	histo->Fill(t11r10 + 0.5, weight);
	break;
      case xAOD::TauJetParameters::DecayMode::Mode_1p1n:
	histo->Fill(t11r11 + 0.5, weight);
	break;
      case xAOD::TauJetParameters::DecayMode::Mode_1pXn:
	histo->Fill(t11r1x + 0.5, weight);
	break;
      default:
	histo->Fill(t1r3 + 0.5, weight);
	break;
      }
      break;
    case xAOD::TauJetParameters::DecayMode::Mode_1pXn:
      switch ( recMode ) {
      case xAOD::TauJetParameters::DecayMode::Mode_1p0n:
	histo->Fill(t1xr10 + 0.5, weight);
	break;
      case xAOD::TauJetParameters::DecayMode::Mode_1p1n:
	histo->Fill(t1xr11 + 0.5, weight);
	break;
      case xAOD::TauJetParameters::DecayMode::Mode_1pXn:
	histo->Fill(t1xr1x + 0.5, weight);
	break;
      default:
	histo->Fill(t1r3 + 0.5, weight);
	break;
      }
      break;
    case xAOD::TauJetParameters::DecayMode::Mode_3p0n:
      switch ( recMode ) {
      case xAOD::TauJetParameters::DecayMode::Mode_3p0n:
	histo->Fill(t30r30 + 0.5, weight);
	break;
      case xAOD::TauJetParameters::DecayMode::Mode_3pXn:
	histo->Fill(t30r3x + 0.5, weight);
	break;
      default:
	histo->Fill(t3r1 + 0.5, weight);
	break;
      }
      break;
    case xAOD::TauJetParameters::DecayMode::Mode_3pXn:
      switch ( recMode ) {
      case xAOD::TauJetParameters::DecayMode::Mode_3p0n:
	histo->Fill(t3xr30 + 0.5, weight);
	break;
      case xAOD::TauJetParameters::DecayMode::Mode_3pXn:
	histo->Fill(t3xr3x + 0.5, weight);
	break;
      default:
	histo->Fill(t3r1 + 0.5, weight);
	break;
      }
      break;
    }
    return;
  }
}
