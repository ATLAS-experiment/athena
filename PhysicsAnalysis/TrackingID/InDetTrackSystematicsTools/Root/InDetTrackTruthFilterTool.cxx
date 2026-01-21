/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetTrackSystematicsTools/InDetTrackTruthFilterTool.h"
#include "InDetTrackSystematicsTools/InDetTrackTruthOriginTool.h"
#include "InDetTrackSystematicsTools/InDetTrackTruthOriginDefs.h"

#include "xAODEventInfo/EventInfo.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthVertex.h"
#include "xAODTruth/TruthVertexContainer.h"
#include "AthContainers/ConstAccessor.h"

#include "PathResolver/PathResolver.h"

#include <TH2.h>
#include <TRandom3.h>
#include <TFile.h>

namespace InDet {

  static const CP::SystematicSet FilterSystematics = 
    {
      InDet::TrackSystematicMap.at(TRK_FAKE_RATE_LOOSE),
      InDet::TrackSystematicMap.at(TRK_FAKE_RATE_TIGHT),
      InDet::TrackSystematicMap.at(TRK_EFF_LOOSE_GLOBAL),
      InDet::TrackSystematicMap.at(TRK_EFF_LOOSE_IBL),
      InDet::TrackSystematicMap.at(TRK_EFF_LOOSE_PP0),
      InDet::TrackSystematicMap.at(TRK_EFF_LOOSE_PHYSMODEL),
      InDet::TrackSystematicMap.at(TRK_EFF_TIGHT_GLOBAL),
      InDet::TrackSystematicMap.at(TRK_EFF_TIGHT_IBL),
      InDet::TrackSystematicMap.at(TRK_EFF_TIGHT_PP0),
      InDet::TrackSystematicMap.at(TRK_EFF_TIGHT_PHYSMODEL),
      InDet::TrackSystematicMap.at(TRK_EFF_LOOSE_COMBINED), //combined systematics are to be used in downstream objects such as secondary vertexing ONLY
      InDet::TrackSystematicMap.at(TRK_EFF_TIGHT_COMBINED), //they are not an "additional" systematic, but rather a replacement for the standard efficiencies where four variations are prohibitive
    };

  InDetTrackTruthFilterTool::InDetTrackTruthFilterTool(const std::string& name) :
    InDetTrackSystematicsTool(name)
  {

#ifndef XAOD_STANDALONE
    declareInterface<IInDetTrackTruthFilterTool>(this);
#endif

  }

  InDetTrackTruthFilterTool::~InDetTrackTruthFilterTool() = default;

  StatusCode InDetTrackTruthFilterTool::initialize() {

    ATH_CHECK ( m_trackOriginTool.retrieve() );

    m_rnd = std::make_unique<TRandom3>(m_seed);

    ATH_CHECK ( initTrkEffSystHistogram( m_trkEffSystScale,
           m_trkEffHistLooseGlobal,
           m_calibFileNomEff,
           "OneMinusRatioEfficiencyVSEtaPt_AfterRebinning_NominalVSOverall_5_Loose") );
    ATH_CHECK ( initTrkEffSystHistogram( m_trkEffSystScale,
           m_trkEffHistLooseIBL,
           m_calibFileNomEff,
           "OneMinusRatioEfficiencyVSEtaPt_AfterRebinning_NominalVSIBL_10_Loose") );
    ATH_CHECK ( initTrkEffSystHistogram( m_trkEffSystScale,
           m_trkEffHistLoosePP0,
           m_calibFileNomEff,
           "OneMinusRatioEfficiencyVSEtaPt_AfterRebinning_NominalVSPP0_25_Loose") );
    ATH_CHECK ( initTrkEffSystHistogram( m_trkEffSystScale,
           m_trkEffHistLoosePhysModel,
           m_calibFileNomEff,
           "OneMinusRatioEfficiencyVSEtaPt_AfterRebinning_NominalVSQGSP_BIC_Loose") );
    ATH_CHECK ( initTrkEffSystHistogram( m_trkEffSystScale,
           m_trkEffHistTightGlobal,
           m_calibFileNomEff,
           "OneMinusRatioEfficiencyVSEtaPt_AfterRebinning_NominalVSOverall_5_TightPrimary") );
    ATH_CHECK ( initTrkEffSystHistogram( m_trkEffSystScale,
           m_trkEffHistTightIBL,
           m_calibFileNomEff,
           "OneMinusRatioEfficiencyVSEtaPt_AfterRebinning_NominalVSIBL_10_TightPrimary") );
    ATH_CHECK ( initTrkEffSystHistogram( m_trkEffSystScale,
           m_trkEffHistTightPP0,
           m_calibFileNomEff,
           "OneMinusRatioEfficiencyVSEtaPt_AfterRebinning_NominalVSPP0_25_TightPrimary") );
    ATH_CHECK ( initTrkEffSystHistogram( m_trkEffSystScale,
           m_trkEffHistTightPhysModel,
           m_calibFileNomEff,
           "OneMinusRatioEfficiencyVSEtaPt_AfterRebinning_NominalVSQGSP_BIC_TightPrimary") );

    ATH_MSG_INFO( "Using for nominal track efficiency the calibration file " << PathResolverFindCalibFile(m_calibFileNomEff) );

     m_histMap = {
      {"TRK_EFF_LOOSE_GLOBAL", m_trkEffHistLooseGlobal.get()},
      {"TRK_EFF_LOOSE_IBL", m_trkEffHistLooseIBL.get()},
      {"TRK_EFF_LOOSE_PP0", m_trkEffHistLoosePP0.get()},
      {"TRK_EFF_LOOSE_PHYSMODEL", m_trkEffHistLoosePhysModel.get()},
      {"TRK_EFF_TIGHT_GLOBAL", m_trkEffHistTightGlobal.get()},
      {"TRK_EFF_TIGHT_IBL", m_trkEffHistTightIBL.get()},
      {"TRK_EFF_TIGHT_PP0", m_trkEffHistTightPP0.get()},
      {"TRK_EFF_TIGHT_PHYSMODEL", m_trkEffHistTightPhysModel.get()}
    };

    ATH_CHECK ( InDetTrackSystematicsTool::initialize() );

    return StatusCode::SUCCESS;
  }

  bool InDetTrackTruthFilterTool::accept(const xAOD::TrackParticle* track) const {

    // these checks shouldn't occur because the config should prevent this from being reached -- but just in case!
    bool anyEffSystActive = isActive(TRK_EFF_LOOSE_GLOBAL) || isActive(TRK_EFF_LOOSE_IBL) || isActive(TRK_EFF_LOOSE_PP0) || 
                isActive(TRK_EFF_LOOSE_PHYSMODEL) || isActive(TRK_EFF_TIGHT_GLOBAL) || isActive(TRK_EFF_TIGHT_IBL) || 
                isActive(TRK_EFF_TIGHT_PP0) || isActive(TRK_EFF_TIGHT_PHYSMODEL ) || isActive(TRK_EFF_LOOSE_COMBINED) || isActive(TRK_EFF_TIGHT_COMBINED);

    bool anyFakeRateActive = isActive(TRK_FAKE_RATE_LOOSE) || isActive(TRK_FAKE_RATE_TIGHT);

    if (anyEffSystActive) {
      if (m_calibFileNomEff.empty()) {
      ATH_MSG_ERROR("No calibration file for requested track efficiency set. You may be running an unsupported datataking period, please contact Tracking CP if you believe this message is in error.");
      }
    }

    if (anyFakeRateActive) {
      if (m_fFakeLoose == -1.0 && m_fFakeTight == -1.0) {
      ATH_MSG_ERROR("Requested fake rate is unavailable. You may be running an unsupported datataking period, please contact Tracking CP if you believe this message is in error.");
      }
    }

    float pt = track->pt();
    float eta = track->eta();

    int origin = m_trackOriginTool->getTrackOrigin(track);

    if ( InDet::TrkOrigin::isFake(origin) ) {
      bool isActiveLoose = isActive( TRK_FAKE_RATE_LOOSE );
      bool isActiveTight = isActive( TRK_FAKE_RATE_TIGHT );
      if (isActiveLoose && isActiveTight) {
       throw std::runtime_error( "Both Loose and TightPrimary versions of fake rate systematic are set." );
      }
      if ( isActiveLoose ) {
        // there is no fake-rate histogram - just a flat uncertainty
        if(m_rnd->Uniform(0, 1) < m_fFakeLoose) return false;
      }
      if ( isActiveTight ) {
        if(m_rnd->Uniform(0, 1) < m_fFakeTight) return false;
      }
    }

    if ( InDet::TrkOrigin::isPrimary(origin) ) {
      if ( isActive( TRK_EFF_LOOSE_GLOBAL ) ) {
        float fTrkEffSyst = getFractionDropped(1, m_trkEffHistLooseGlobal, pt, eta);
        if(m_rnd->Uniform(0, 1) < fTrkEffSyst) return false;
      }
      if ( isActive( TRK_EFF_LOOSE_IBL ) ) {
        float fTrkEffSyst = getFractionDropped(1, m_trkEffHistLooseIBL, pt, eta);
        if(m_rnd->Uniform(0, 1) < fTrkEffSyst) return false;
      }
      if ( isActive( TRK_EFF_LOOSE_PP0 ) ) {
        float fTrkEffSyst = getFractionDropped(1, m_trkEffHistLoosePP0, pt, eta);
        if(m_rnd->Uniform(0, 1) < fTrkEffSyst) return false;
      }
      if ( isActive( TRK_EFF_LOOSE_PHYSMODEL ) ) {
        float fTrkEffSyst = getFractionDropped(1, m_trkEffHistLoosePhysModel, pt, eta);
        if(m_rnd->Uniform(0, 1) < fTrkEffSyst) return false;
      }
      if ( isActive( TRK_EFF_TIGHT_GLOBAL ) ) {
        float fTrkEffSyst = getFractionDropped(1, m_trkEffHistTightGlobal, pt, eta);
        if(m_rnd->Uniform(0, 1) < fTrkEffSyst) return false;
      }
      if ( isActive( TRK_EFF_TIGHT_IBL ) ) {
        float fTrkEffSyst = getFractionDropped(1, m_trkEffHistTightIBL, pt, eta);
        if(m_rnd->Uniform(0, 1) < fTrkEffSyst) return false;
      }
      if ( isActive( TRK_EFF_TIGHT_PP0 ) ) {
        float fTrkEffSyst = getFractionDropped(1, m_trkEffHistTightPP0, pt, eta);
        if(m_rnd->Uniform(0, 1) < fTrkEffSyst) return false;
      }
      if ( isActive( TRK_EFF_TIGHT_PHYSMODEL ) ) {
        float fTrkEffSyst = getFractionDropped(1, m_trkEffHistTightPhysModel, pt, eta);
        if(m_rnd->Uniform(0, 1) < fTrkEffSyst) return false;
      }

      // combined systematics represent the sum in quadrature of the individual systematics
      if ( isActive( TRK_EFF_LOOSE_COMBINED ) ) {
        float fTrkEffSyst = sqrt( pow(getFractionDropped(1, m_trkEffHistLooseGlobal, pt, eta), 2) +
                                  pow(getFractionDropped(1, m_trkEffHistLooseIBL, pt, eta), 2) +
                                  pow(getFractionDropped(1, m_trkEffHistLoosePP0, pt, eta), 2) +
                                  pow(getFractionDropped(1, m_trkEffHistLoosePhysModel, pt, eta), 2) );
        if(m_rnd->Uniform(0, 1) < fTrkEffSyst) return false;
      }
      if ( isActive( TRK_EFF_TIGHT_COMBINED ) ) {
        float fTrkEffSyst = sqrt( pow(getFractionDropped(1, m_trkEffHistTightGlobal, pt, eta), 2) +
                                  pow(getFractionDropped(1, m_trkEffHistTightIBL, pt, eta), 2) +
                                  pow(getFractionDropped(1, m_trkEffHistTightPP0, pt, eta), 2) +
                                  pow(getFractionDropped(1, m_trkEffHistTightPhysModel, pt, eta), 2) );
        if(m_rnd->Uniform(0, 1) < fTrkEffSyst) return false;
      }
    }

    return true;
  }

  StatusCode InDetTrackTruthFilterTool::initTrkEffSystHistogram(float scale, std::unique_ptr<TH2>& histogram, std::string rootFileName, std::string histogramName) const {

    ATH_CHECK( initObject<TH2>(histogram, rootFileName, histogramName) );

    for(int binx=1; binx<=histogram->GetNbinsX(); binx++) {
      for(int biny=1; biny<=histogram->GetNbinsY(); biny++) {
        // now using a histogram which is 1 - eff1/eff2, so we don't apply the "1 - content" correction ourselves
        float content = histogram->GetBinContent(binx, biny); // get the bin content
        content = fabs(content); // retrieve uncertainty and symmetrize (should be positive in histogram anyway)
        content *= scale; // scale systematic
        if(content > 1) content = 1; // protection: no larger than 100% uncertainty

        histogram->SetBinContent(binx, biny, content);
      }
    }

    return StatusCode::SUCCESS;
  }

  float InDetTrackTruthFilterTool::getFractionDropped(float fDefault, const std::unique_ptr<TH2>& histogram, float x, float y, bool xAxisIspT) const {

    if(histogram==nullptr) {
      return fDefault;
    }

    if(xAxisIspT)
      {
        x *= 1.e-3; // unit conversion to GeV
        if( x >= histogram->GetXaxis()->GetXmax() ) x = histogram->GetXaxis()->GetXmax() - 0.001;
      }

    float frac = histogram->GetBinContent(histogram->FindFixBin(x, y));
    if( frac > 1. ) {
      ATH_MSG_WARNING( "Fraction from histogram " << histogram->GetName()
           << " is greater than 1. Setting fraction to 1." );
      frac = 1.;
    }
    if( frac < 0. ) {
      ATH_MSG_WARNING( "Fraction from histogram " << histogram->GetName()
           << " is less than 0. Setting fraction to 0." );
      frac = 0.;
    }
    return frac;
  }

  float InDetTrackTruthFilterTool::getTrackUncertainty(const xAOD::TrackParticle* track, const std::string& systName) const {

    auto it = m_histMap.find(systName);
    if (it == m_histMap.end()) {
      ATH_MSG_ERROR( "getTrackUncertainty: Standard track systematic name " << systName << " is not recognized. Returning 0." );
      return 0.;
    }

    TH2* hist = it->second;
    if (hist == nullptr) {
      ATH_MSG_ERROR( "Standard tracking efficiency histogram for " << systName << " is not properly initialized!" );
      return 0.;
    }

    //convert pt to GeV
    float pt = track->pt() * 1.e-3;

    //check that track pt does not go beyond range
    if( pt >= hist->GetXaxis()->GetXmax() ) pt = hist->GetXaxis()->GetXmax() - 0.001;
    return hist->GetBinContent(hist->FindBin(pt, track->eta()));
  }

  bool InDetTrackTruthFilterTool::isAffectedBySystematic( const CP::SystematicVariation& syst ) const
  {
    return InDetTrackSystematicsTool::isAffectedBySystematic( syst );
  }

  CP::SystematicSet InDetTrackTruthFilterTool::affectingSystematics() const
  {
    return FilterSystematics;
  }

  CP::SystematicSet InDetTrackTruthFilterTool::recommendedSystematics() const
  {
    return InDetTrackSystematicsTool::recommendedSystematics();
  }

  StatusCode InDetTrackTruthFilterTool::applySystematicVariation( const CP::SystematicSet& systs )
  {
    return InDetTrackSystematicsTool::applySystematicVariation(systs);
  }

} // namespace InDet
