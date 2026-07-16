/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetTrackSystematicsTools/JetTrackFilterTool.h"
#include "InDetTrackSystematicsTools/InDetTrackTruthOriginTool.h"
#include "InDetTrackSystematicsTools/InDetTrackTruthOriginDefs.h"
#include "xAODTracking/TrackParticleContainer.h"

#include "CxxUtils/FastReseededPRNG.h"
#include "InDetTrackSystematicsTools/getEventNumber.h"
#include "FourMomUtils/xAODP4Helpers.h"
#include "PathResolver/PathResolver.h"

#include "CxxUtils/checker_macros.h"

#include <TH2.h>
#include <TFile.h>
#include <random>
#include <stdexcept>
#include <utility>

namespace InDet {

  static const CP::SystematicSet FilterSystematics = 
    {
      InDet::TrackSystematicMap.at(TRK_EFF_LOOSE_TIDE),
      InDet::TrackSystematicMap.at(TRK_FAKE_RATE_TIGHT_TIDE),
      InDet::TrackSystematicMap.at(TRK_FAKE_RATE_LOOSE_TIDE)
    };

  JetTrackFilterTool::JetTrackFilterTool(const std::string& name) :
    InDetTrackSystematicsTool(name)
  {
#ifndef XAOD_STANDALONE
    declareInterface<IJetTrackFilterTool>(this);
#endif
  }

  JetTrackFilterTool::~JetTrackFilterTool() = default;

  StatusCode JetTrackFilterTool::initialize()
  {

    ATH_CHECK( initObject<TH2>( m_trkNomEff,
               m_calibFileNomEff, 
               "EfficiencyVSEtaPt_AfterRebinningNominal_Loose" ) );

    ATH_MSG_INFO( "Using for nominal track efficiency the calibration file " << PathResolverFindCalibFile(m_calibFileNomEff) );

    ATH_CHECK ( m_trackOriginTool.retrieve() );

    ATH_CHECK ( InDetTrackSystematicsTool::initialize() );

    return StatusCode::SUCCESS;
  }


  bool JetTrackFilterTool::accept(const xAOD::TrackParticle* track, const xAOD::Jet* jet) const
  {

    if ( track == nullptr) {
      ATH_MSG_DEBUG( "Pointer to track is null!" );
      return false;
    }
    if ( jet == nullptr ) {
      ATH_MSG_DEBUG( "Pointer to jet is null." );
      return true;
    }

    FastReseededPRNG prng(
        m_seed,
        static_cast<uint32_t>(std::abs(track->phi()) * 1e6),
        static_cast<uint32_t>(std::abs(track->eta()) * 1e3),
        InDet::getEventNumber(evtStore()));

    // Uncertainties are only applicable inside high pT jets
    if (jet->pt() < m_minJetPt) return true;

    // if the track is outside of the range of this jet, then it is allowed to pass
    constexpr bool useRapidity = false; // use eta instead of rapidity - the default for this function is true
    if ( !xAOD::P4Helpers::isInDeltaR( *track, *jet, m_deltaR, useRapidity ) ) return true;

    if ( isActive( TRK_EFF_LOOSE_TIDE ) ) {
      // the probability to drop a track scales with the tracking efficiency,
      // on account of the method used to derive the uncertainties
      float probDrop = std::fabs(m_trkEffSystScale); // default is one; adjust this parameter to increase / decrease the effect
      probDrop *= m_effUncertTIDE;
      probDrop *= getNomTrkEff( track );
      if ( std::uniform_real_distribution<double>(0, 1)(prng) < probDrop ) return false;
    }

    int origin = m_trackOriginTool->getTrackOrigin(track);

    if( isActive( TRK_FAKE_RATE_LOOSE_TIDE ) ){
      if ( InDet::TrkOrigin::isFake(origin) ) {
        if(std::uniform_real_distribution<double>(0, 1)(prng) <  m_fakeUncertTIDE) return false;
      }
    }

    if( isActive( TRK_FAKE_RATE_TIGHT_TIDE ) ){
      if ( InDet::TrkOrigin::isFake(origin) ) {
        ATH_MSG_DEBUG("Track fakes in jets uncertainty (Tight) covered by inclusive (Tight) uncertainty - operating in pass-through mode...");
        return true;
      }
    }

    return true;
  }

  bool JetTrackFilterTool::accept( const xAOD::TrackParticle* track, const xAOD::JetContainer* jets ) const
  {
    if ( jets == nullptr ) {
      ATH_MSG_DEBUG( "Pointer to jet container is null." );
      return true;
    }
    // check that the track passes every jet
    for ( const auto* jet : *jets ) {
      if ( !accept( track, jet ) ) return false;
    }
    return true;
  }

  float JetTrackFilterTool::getNomTrkEff(const xAOD::TrackParticle* track) const
  {
    if (m_trkNomEff == nullptr) {
      ATH_MSG_ERROR( "Nominal track efficiency histogram is not property initialized!" );
      return 0.;
    }
    // this histogram has pt on the x-axis and eta on the y-axis, unlike some other histograms used in this package
    // make sure to convert to GeV
    return m_trkNomEff->GetBinContent(std::as_const(m_trkNomEff)->FindBin(track->pt()*1e-3, track->eta()));
  }

  bool JetTrackFilterTool::isAffectedBySystematic( const CP::SystematicVariation& syst ) const
  {
    return InDetTrackSystematicsTool::isAffectedBySystematic( syst );
  }

  CP::SystematicSet JetTrackFilterTool::affectingSystematics() const
  {
    return FilterSystematics;
  }

  CP::SystematicSet JetTrackFilterTool::recommendedSystematics() const
  {
    return InDetTrackSystematicsTool::recommendedSystematics();
  }

  StatusCode JetTrackFilterTool::applySystematicVariation( const CP::SystematicSet& systs )
  {
    return InDetTrackSystematicsTool::applySystematicVariation(systs);
  }

  bool JetTrackFilterTool::accept(
      const xAOD::TrackParticle* track,
      const xAOD::JetContainer* jets,
      const CP::SystematicSet& syst) const
  {
    std::lock_guard<std::mutex> lock(m_sysLock);
    JetTrackFilterTool* nc_this ATLAS_THREAD_SAFE =
        const_cast<JetTrackFilterTool*>(this);
    if (nc_this->applySystematicVariation(syst).isFailure())
      throw std::invalid_argument("Systematic '" + syst.name()
          + "' was not pre-registered in initialize()");
    return accept(track, jets);
  }

} // namespace InDet
