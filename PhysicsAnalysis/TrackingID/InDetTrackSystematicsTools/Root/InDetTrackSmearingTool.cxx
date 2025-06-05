/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// ROOT include(s):
#include <TRandom.h>
#include <TH2F.h>
#include <TFile.h>

// std includes
#include <utility>

// EDM include(s):
#include "xAODEventInfo/EventInfo.h"
//#include "AthenaBaseComps/AthCheckMacros.h"

#include "PathResolver/PathResolver.h"

// Local include(s):
#include "InDetTrackSystematicsTools/InDetTrackSmearingTool.h"

namespace {
  using TrackCorrTool_t = CP::CorrectionTool< xAOD::TrackParticleContainer >;
}

namespace InDet {

  static const CP::SystematicSet RecommendedSystematics = 
    {
      InDet::TrackSystematicMap.at(TRK_RES_D0_MEAS),
      InDet::TrackSystematicMap.at(TRK_RES_Z0_MEAS),
    };

  InDetTrackSmearingTool::InDetTrackSmearingTool( const std::string& name )
  : InDet::InDetTrackSystematicsTool( name )
  {

#ifndef XAOD_STANDALONE
    declareInterface<IInDetTrackSmearingTool>(this);
#endif

  }


  StatusCode InDetTrackSmearingTool::initialize() {

    // Greet the user:
    ATH_MSG_INFO( "Initializing..." );

    ATH_MSG_INFO( "Using seed of " << m_seed << " to initialize RNG" );
    m_rnd = std::make_unique<TRandom3>(m_seed);

    ATH_MSG_INFO( "Using dedicated CTIDE smearing maps for tracks in jets" );
    ATH_MSG_INFO( "Using for the full pT range the CTIDE calibration file " << PathResolverFindCalibFile(m_calibFileIP_CTIDE) );

    std::string rootfileName = m_calibFileIP_CTIDE;

    ATH_CHECK( initObject<TH2>(m_smearD0, rootfileName, "d0quaddiff_Pt_Eta" ) );
    ATH_CHECK( initObject<TH2>(m_smearZ0, rootfileName, "z0quaddiff_Pt_Eta" ) );

    // do common initialization (at time of writing, register affecting systematics)
    ATH_CHECK( InDetTrackSystematicsTool::initialize() );

    // Return gracefully:
    return StatusCode::SUCCESS;
  }

  float InDetTrackSmearingTool::GetSmearD0Sigma(const xAOD::TrackParticle& track) const {
    float pt = 1.e-3*track.pt(); // need to convert pt to GeV
    float eta = track.eta();
    float sigma_D0 = 0.f;

    bool isActiveD0Meas = isActive(TRK_RES_D0_MEAS);

    if (isActiveD0Meas == 0) {
      // pass-through D0Meas, return sigma_D0 early
      return std::sqrt(sigma_D0);
    }

    TH2* d0hist = m_smearD0.get();

    if (d0hist == nullptr) ATH_MSG_ERROR( "d0 histogram is null!" );

    float d0Smear = readHistogram(d0hist, pt, eta);

    // Apply a minimum smearing in the case where there is an empty bin
    // TODO: Remove for next round of recommendations
    if (d0Smear == 0) {
      d0Smear = 0.002;
    }

    sigma_D0 += d0Smear*d0Smear;

    return std::sqrt(sigma_D0);
  }


  float InDetTrackSmearingTool::GetSmearZ0Sigma(const xAOD::TrackParticle& track) const {
    float pt = 1.e-3*track.pt(); // need to convert pt to GeV
    float eta = track.eta();
    float sigma_Z0 = 0.f;

    bool isActiveZ0Meas = isActive(TRK_RES_Z0_MEAS);

    if (isActiveZ0Meas == 0) {
      // pass-through Z0Meas, return sigma_Z0 early
      return std::sqrt(sigma_Z0);
    }

    TH2* z0hist = m_smearZ0.get();

    if (z0hist == nullptr) ATH_MSG_ERROR( "z0 histogram is null!" );

    float z0Smear = readHistogram(z0hist, pt, eta);

    // Apply a minimum smearing in the case where there is an empty bin
    // TODO: Remove for next round of recommendations
    if (z0Smear == 0) {
      z0Smear = 0.03;
    }

    sigma_Z0 += z0Smear*z0Smear;

    return std::sqrt(sigma_Z0);
  }

CP::CorrectionCode InDetTrackSmearingTool::applyCorrection( xAOD::TrackParticle& track ) {

    float sigmaD0 = GetSmearD0Sigma( track );
    float sigmaZ0 = GetSmearZ0Sigma( track );

    static const SG::AuxElement::Accessor< float > accD0( "d0" );
    static const SG::AuxElement::Accessor< float > accZ0( "z0" );

    //NB: only call the RNG if the widths are greater than 0
    if ( sigmaD0 > 0. ) accD0( track ) = m_rnd->Gaus( track.d0(), sigmaD0 );
    if ( sigmaZ0 > 0. ) accZ0( track ) = m_rnd->Gaus( track.z0(), sigmaZ0 );

    return CP::CorrectionCode::Ok;
  }

  CP::CorrectionCode InDetTrackSmearingTool::correctedCopy( const xAOD::TrackParticle& in,
                  xAOD::TrackParticle*& out )
  {
    return TrackCorrTool_t::correctedCopy(in, out);
  }

  CP::CorrectionCode InDetTrackSmearingTool::applyContainerCorrection( xAOD::TrackParticleContainer& cont )
  {
    return TrackCorrTool_t::applyContainerCorrection(cont);
  }

  bool InDetTrackSmearingTool::isAffectedBySystematic( const CP::SystematicVariation& syst ) const
  {
    return InDetTrackSystematicsTool::isAffectedBySystematic( syst );
  }

  CP::SystematicSet InDetTrackSmearingTool::affectingSystematics() const
  {
    return RecommendedSystematics;
  }

  CP::SystematicSet InDetTrackSmearingTool::recommendedSystematics() const
  {
    //    return InDetTrackSystematicsTool::recommendedSystematics();
    return RecommendedSystematics;
  }

  StatusCode InDetTrackSmearingTool::applySystematicVariation( const CP::SystematicSet& systs )
  {
    // should do check in here and return StatusCode::FAILURE if LOOSE and TIGHT versions are set simultaneously
    return InDetTrackSystematicsTool::applySystematicVariation(systs);
  }

  float InDetTrackSmearingTool::readHistogram(const TH2* histogram, float pt, float eta) const {
    // safety measure:
    if( eta>2.499 )  eta= 2.499;
    if( eta<-2.499 ) eta=-2.499;
    if ( pt < histogram->GetXaxis()->GetXmin() ) {
      pt = histogram->GetXaxis()->GetXmin() + 0.01;
    }
    if ( pt >= histogram->GetXaxis()->GetXmax() ) {
      pt = histogram->GetXaxis()->GetXmax() - 0.01;
    }

    int bin = histogram->FindFixBin(pt, eta);
    float val = histogram->GetBinContent(bin);
    return val;
  }

} // namespace InDet

