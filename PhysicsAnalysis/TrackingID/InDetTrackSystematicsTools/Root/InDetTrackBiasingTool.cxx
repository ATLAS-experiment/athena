/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetTrackSystematicsTools/InDetTrackBiasingTool.h"
#include "AsgDataHandles/ReadHandle.h"
#include "PathResolver/PathResolver.h"

#include <cmath>
#include <TH2.h>
#include <TFile.h>

namespace {
  using TrackCorrTool_t = CP::CorrectionTool< xAOD::TrackParticleContainer >;
}

namespace InDet {

  InDetTrackBiasingTool::InDetTrackBiasingTool(const std::string& name) :
    InDetTrackSystematicsTool(name)
  {

#ifndef XAOD_STANDALONE
    declareInterface<IInDetTrackBiasingTool>(this);
#endif

  }

  StatusCode InDetTrackBiasingTool::initialize()
  {
    if (m_biasD0 != 0.) {
      ATH_MSG_INFO( "overall d0 bias added = " << m_biasD0
        << " mm (not part of an official recommendation)" );
    }
    if (m_biasZ0 != 0.) {
      ATH_MSG_INFO( "overall z0 bias added = " << m_biasZ0
        << " mm (not part of an official recommendation)" );
    }
    if (m_biasQoverPsagitta != 0.) {
      ATH_MSG_INFO( "overall QoverP sagitta bias added = " << m_biasQoverPsagitta
        << " TeV^-1 (not part of an official recommendation)" );
    }

    if (m_runNumber > 0) {
      ATH_MSG_WARNING( "Using manually-set run number (" << m_runNumber << ") to determine which calibration file to use." );
    }

    ATH_CHECK( initHistograms() );

    ATH_CHECK(m_evtInfoKey.initialize());

    ATH_CHECK( InDetTrackSystematicsTool::initialize() );

    return StatusCode::SUCCESS;
  }

  InDetTrackBiasingTool::~InDetTrackBiasingTool() {
    m_runNumber = -1;
  }

  CP::CorrectionCode InDetTrackBiasingTool::applyCorrection(xAOD::TrackParticle& track) {
    if (m_activeSysts == nullptr) {
      ATH_MSG_ERROR( "applySystematicVariation() was not called before applyCorrection()." );
      return CP::CorrectionCode::Error;
    }
    return applyBiasWithSyst(track, *m_activeSysts);
  }

  CP::CorrectionCode InDetTrackBiasingTool::applyCorrection(
      xAOD::TrackParticle& track, const CP::SystematicSet& syst) const
  {
    const CP::SystematicSet* filtered = getFilteredSysts(syst);
    if (filtered == nullptr) return CP::CorrectionCode::Error;
    return applyBiasWithSyst(track, *filtered);
  }

  CP::CorrectionCode InDetTrackBiasingTool::applyBiasWithSyst(
      xAOD::TrackParticle& track, const CP::SystematicSet& filtered) const
  {
    if ( !m_isMC ) {
      ATH_MSG_ERROR( "InDetTrackBiasingTool should only be run on MC." );
      return CP::CorrectionCode::Error;
    }

    // determine which run number to use
    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_evtInfoKey);
    static const SG::Accessor<unsigned int> randomRunNumber("RandomRunNumber");
    auto runNumber = randomRunNumber(*eventInfo);

    if (runNumber <= 0) {
      ATH_MSG_WARNING( "Run number not set." );
    }
    // 5 TeV and heavy-ion runs have no biasing maps
    if (runNumber >= 286282 && runNumber <= 287931) {
      ATH_MSG_ERROR( "The 5 TeV and heavy ion runs do not have biasing maps for release 22. "
         "Contact the tracking CP group to discuss the derivation of these maps." );
      return CP::CorrectionCode::Error;
    }

    // find which configured period this run belongs to
    size_t periodIdx = m_calibFiles.size(); // sentinel: no match
    if (m_runNumberBounds.empty()) {
      periodIdx = 0; // single period, accept all run numbers
    } else {
      for (size_t i = 0; i + 1 < m_runNumberBounds.size(); ++i) {
        if (runNumber > m_runNumberBounds[i] && runNumber <= m_runNumberBounds[i+1]) {
          periodIdx = i;
          break;
        }
      }
    }
    if (periodIdx >= m_calibFiles.size()) {
      ATH_MSG_ERROR( "Run number = " << runNumber << " does not fall within any configured calibration period." );
      return CP::CorrectionCode::Error;
    }

    // select histograms for the matched period
    TH2* biasD0Histogram             = m_biasD0Histograms[periodIdx].get();
    TH2* biasZ0Histogram             = m_biasZ0Histograms[periodIdx].get();
    TH2* biasQoverPsagittaHistogram  = m_biasQoverPsagittaHistograms[periodIdx].get();

    bool doD0Bias    = m_applyD0Bias    && biasD0Histogram != nullptr;
    bool doZ0Bias    = m_applyZ0Bias    && biasZ0Histogram != nullptr;
    bool doQoverPBias = m_applyQoverPBias && biasQoverPsagittaHistogram != nullptr;

    if (m_applyD0Bias    && !biasD0Histogram)            ATH_MSG_WARNING( "d0 bias histogram is nullptr. Will not perform d0 bias." );
    if (m_applyZ0Bias    && !biasZ0Histogram)            ATH_MSG_WARNING( "z0 bias histogram is nullptr. Will not perform z0 bias." );
    if (m_applyQoverPBias && !biasQoverPsagittaHistogram) ATH_MSG_WARNING( "q/p bias histogram is nullptr. Will not perform q/p sagitta bias." );

    // declare static accessors to avoid repeating string lookups
    static const SG::Accessor< float > accD0( "d0" );
    static const SG::Accessor< float > accZ0( "z0" );
    static const SG::Accessor< float > accQOverP( "qOverP" );

    const float phi = track.phi0();
    const float eta = track.eta();

    // do the biasing
    // Always apply the correction to the nominal; undo it for systematic variations
    if ( doD0Bias ) {
      const float d0Corr = readHistogram(m_biasD0, biasD0Histogram, phi, eta);
      accD0( track ) += d0Corr;
      if ( isActive( TRK_BIAS_D0_WM, filtered ) ) {
        accD0( track ) -= d0Corr;
      }
    }
    if ( doZ0Bias ) {
      const float z0Corr = readHistogram(m_biasZ0, biasZ0Histogram, phi, eta);
      accZ0( track ) += z0Corr;
      if ( isActive( TRK_BIAS_Z0_WM, filtered ) ) {
        accZ0( track ) -= z0Corr;
      }
    }
    if ( doQoverPBias ) {
      auto sinTheta = 1.0 / std::cosh(eta);
      const float qOverPCorr = 1.e-6*sinTheta*readHistogram(m_biasQoverPsagitta, biasQoverPsagittaHistogram, phi, eta);
      accQOverP( track ) += qOverPCorr;
      if ( isActive( TRK_BIAS_QOVERP_SAGITTA_WM, filtered ) ) {
        accQOverP( track ) -= qOverPCorr;
      }
    }

    return CP::CorrectionCode::Ok;
  }

  StatusCode InDetTrackBiasingTool::initHistograms()
  {

    if (m_calibFiles.empty()) {
      ATH_MSG_ERROR( "No calibration files configured. Set 'calibFiles' and (for multiple periods) "
                     "'runNumberBounds' for the relevant MC campaign via the python configuration." );
      return StatusCode::FAILURE;
    }
    if (!m_runNumberBounds.empty() && m_runNumberBounds.size() != m_calibFiles.size() + 1) {
      ATH_MSG_ERROR( "'runNumberBounds' (size " << m_runNumberBounds.size() << ") must have "
                     "calibFiles.size() + 1 = " << (m_calibFiles.size() + 1) << " entries." );
      return StatusCode::FAILURE;
    }
    if (m_runNumberBounds.empty() && m_calibFiles.size() > 1) {
      ATH_MSG_ERROR( "'runNumberBounds' must be set when multiple calibration files are configured." );
      return StatusCode::FAILURE;
    }

    for (size_t i = 0; i < m_calibFiles.size(); ++i) {
      if (m_runNumberBounds.empty()) {
        ATH_MSG_INFO( "Calibration period 0 (all run numbers): file: "
                      << PathResolverFindCalibFile(m_calibFiles[i]) );
      } else {
        ATH_MSG_INFO( "Calibration period " << i << ": run range ["
                      << m_runNumberBounds[i] << ", " << m_runNumberBounds[i+1]
                      << "], file: " << PathResolverFindCalibFile(m_calibFiles[i]) );
      }
      m_biasD0Histograms.emplace_back(nullptr);
      ATH_CHECK( initObject<TH2>(m_biasD0Histograms.back(), m_calibFiles[i], m_d0_nominal_histName) );
      m_biasZ0Histograms.emplace_back(nullptr);
      ATH_CHECK( initObject<TH2>(m_biasZ0Histograms.back(), m_calibFiles[i], m_z0_nominal_histName) );
      m_biasQoverPsagittaHistograms.emplace_back(nullptr);
      ATH_CHECK( initObject<TH2>(m_biasQoverPsagittaHistograms.back(), m_calibFiles[i], m_sagitta_nominal_histName) );
    }

    return StatusCode::SUCCESS;
  }

  float InDetTrackBiasingTool::readHistogram(float fDefault, TH2* histogram, float phi, float eta) const {
    if (histogram == nullptr) {
      ATH_MSG_ERROR( "Configuration histogram is invalid. Check the run number and systematic configuration combination.");
      throw std::runtime_error( "invalid configuration" );
    }

    // safety measure:
    if( eta>2.499 )  eta= 2.499;
    if( eta<-2.499 ) eta=-2.499;

    float f = histogram->GetBinContent(histogram->FindBin(eta, phi));
    f += fDefault;   // should be zero unless a manual override is provided

    return f;
  }

  CP::CorrectionCode InDetTrackBiasingTool::correctedCopy( const xAOD::TrackParticle& in,
							    xAOD::TrackParticle*& out )
  {
    return TrackCorrTool_t::correctedCopy(in, out);
  }
  
  CP::CorrectionCode InDetTrackBiasingTool::applyContainerCorrection( xAOD::TrackParticleContainer& cont )
  {
    return TrackCorrTool_t::applyContainerCorrection(cont);
  }

  bool InDetTrackBiasingTool::isAffectedBySystematic( const CP::SystematicVariation& syst ) const
  {
    return InDetTrackSystematicsTool::isAffectedBySystematic( syst );
  }

  CP::SystematicSet InDetTrackBiasingTool::affectingSystematics() const
  {
    CP::SystematicSet result;
    if (m_applyD0Bias)     result.insert(InDet::TrackSystematicMap.at(TRK_BIAS_D0_WM));
    if (m_applyZ0Bias)     result.insert(InDet::TrackSystematicMap.at(TRK_BIAS_Z0_WM));
    if (m_applyQoverPBias) result.insert(InDet::TrackSystematicMap.at(TRK_BIAS_QOVERP_SAGITTA_WM));
    return result;
  }

  CP::SystematicSet InDetTrackBiasingTool::recommendedSystematics() const
  {
    return InDetTrackSystematicsTool::recommendedSystematics();
  }

  StatusCode InDetTrackBiasingTool::applySystematicVariation( const CP::SystematicSet& systs )
  {
    return InDetTrackSystematicsTool::applySystematicVariation(systs);
  }


}
