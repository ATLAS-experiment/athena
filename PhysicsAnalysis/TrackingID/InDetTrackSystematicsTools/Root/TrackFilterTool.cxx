/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetTrackSystematicsTools/TrackFilterTool.h"
#include "InDetTrackSystematicsTools/InDetTrackTruthOriginTool.h"
#include "InDetTrackSystematicsTools/InDetTrackTruthOriginDefs.h"

#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthVertex.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTruth/TruthVertexContainer.h"
#include "AthContainers/ConstAccessor.h"

#include "PathResolver/PathResolver.h"
#include "CxxUtils/checker_macros.h"
#include "CxxUtils/FastReseededPRNG.h"
#include "InDetTrackSystematicsTools/getEventNumber.h"
#include "FourMomUtils/xAODP4Helpers.h"

#include <TH2.h>
#include <TFile.h>
#include <random>
#include <stdexcept>
#include <utility>


namespace InDet {

  static const CP::SystematicSet FilterSystematics =
    {
      InDet::TrackSystematicMap.at(TRK_FAKE_RATE_LOOSE),
      InDet::TrackSystematicMap.at(TRK_FAKE_RATE_TIGHT),
      InDet::TrackSystematicMap.at(TRK_FAKE_RATE_LOOSE_TIDE),
      InDet::TrackSystematicMap.at(TRK_FAKE_RATE_LOOSE_INCL),
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

  TrackFilterTool::TrackFilterTool(const std::string& name) :
    InDetTrackSystematicsTool(name)
  {
#ifndef XAOD_STANDALONE
    declareInterface<ITrackFilterTool>(this);
#endif
  }

  TrackFilterTool::~TrackFilterTool() = default;

  StatusCode TrackFilterTool::initialize()
  {

    ATH_CHECK ( m_trackOriginTool.retrieve() );
    ATH_CHECK ( InDetTrackSystematicsTool::initialize() );

    return StatusCode::SUCCESS;
  }

  bool TrackFilterTool::accept( const xAOD::TrackParticle* track ) const
  {
    return acceptInclusive(track);
  }

  bool TrackFilterTool::accept( const xAOD::TrackParticle* track,
                                const xAOD::Jet* jet ) const
  {
    if (jet == nullptr || !isInJet(track, jet)) {
        return acceptInclusive(track);
    }

    return acceptJet(track, jet);
  }

  bool TrackFilterTool::accept( const xAOD::TrackParticle* track,
                                const xAOD::JetContainer* jets ) const
  {
    if (jets == nullptr || jets->empty()) {
        return acceptInclusive(track);
    }

    for (const auto* jet : *jets) {
        if (isInJet(track, jet)) {
            return acceptJet(track, jet);
        }
    }

    return acceptInclusive(track);
  }

  bool TrackFilterTool::acceptInclusive( const xAOD::TrackParticle* track ) const
  {
    if (track == nullptr) {
        ATH_MSG_DEBUG("Pointer to track is null!");
        return false;
    }

    const bool isActiveLoose = isActive(TRK_FAKE_RATE_LOOSE);
    const bool isActiveTight = isActive(TRK_FAKE_RATE_TIGHT);
    const bool isActiveLooseIncl = isActive(TRK_FAKE_RATE_LOOSE_INCL);

    if ((isActiveLoose || isActiveLooseIncl) && isActiveTight) {        throw std::runtime_error(
            "Both Loose and TightPrimary versions of fake rate systematic "
            "are set.");
    }

    // No inclusive fake-rate systematic active.
    if (!isActiveLoose && !isActiveTight && !isActiveLooseIncl) {
        return true;
    }

    FastReseededPRNG prng(
        m_truthSeed,
        activeVariationMask(),
        static_cast<uint32_t>(std::abs(track->phi()) * 1e6),
        static_cast<uint32_t>(std::abs(track->eta()) * 1e3),
        InDet::getEventNumber(evtStore()));

    int origin = m_trackOriginTool->getTrackOrigin(track);

    // If the truth link is broken, fall back to the truth-origin label.
    if (m_trackOriginTool->getTruth(track) == nullptr) {
      int exclusive_origin = -1;
      const static SG::ConstAccessor<int> acc_ftagTruthOrigin("ftagTruthOriginLabel");
      if (acc_ftagTruthOrigin.isAvailable(*track)) {
        exclusive_origin = acc_ftagTruthOrigin(*track);
        origin = InDet::TrkOrigin::getTrkOrigin(exclusive_origin);
      } // else stays as pileup, which is what getTrackOrigin returns when element link is broken
    }


    if (!InDet::TrkOrigin::isFake(origin))
        return true;

    if (isActiveLoose || isActiveLooseIncl) {
        if(m_fFakeLoose < 0 || m_fFakeLoose > 1) {
            throw std::runtime_error("m_fFakeLoose is set to an invalid value. It should be between 0 and 1.");
        }
        if (std::uniform_real_distribution<double>(0, 1)(prng) < m_fFakeLoose)
        return false;
    }

    if (isActiveTight) {
        if(m_fFakeTight < 0 || m_fFakeTight > 1) {
            throw std::runtime_error("m_fFakeTight is set to an invalid value. It should be between 0 and 1.");
        }
        if (std::uniform_real_distribution<double>(0, 1)(prng) < m_fFakeTight)
        return false;
    }

    return true;
  }


  bool TrackFilterTool::acceptJet( const xAOD::TrackParticle* track,
                                   const xAOD::Jet* jet ) const
  {
    if (track == nullptr) {
        ATH_MSG_DEBUG("Pointer to track is null!");
        return false;
    }

    if (jet == nullptr)
        return acceptInclusive(track);

    const bool isActiveLoose = isActive(TRK_FAKE_RATE_LOOSE);
    const bool isActiveTight = isActive(TRK_FAKE_RATE_TIGHT);
    const bool isActiveLooseTIDE = isActive(TRK_FAKE_RATE_LOOSE_TIDE);
    const bool isActiveLooseIncl = isActive(TRK_FAKE_RATE_LOOSE_INCL);


    if (!isActiveLoose && !isActiveTight && !isActiveLooseTIDE) {
        return true;
    }

    const int origin = m_trackOriginTool->getTrackOrigin(track);

    if (!InDet::TrkOrigin::isFake(origin))
        return true;

    FastReseededPRNG prng(
        m_jetSeed,
        activeVariationMask(),
        static_cast<uint32_t>(std::abs(track->phi()) * 1e6),
        static_cast<uint32_t>(std::abs(track->eta()) * 1e3),
        InDet::getEventNumber(evtStore()));


    if (isActiveLoose || isActiveLooseTIDE) {
        //check that m_fFakeUncertTIDE is set to a value between 0 and 1, otherwise throw an error
        if (m_fFakeUncertTIDE < 0 || m_fFakeUncertTIDE > 1) {
          throw std::runtime_error("m_fFakeUncertTIDE is set to an invalid value. It should be between 0 and 1.");
        }
        if (std::uniform_real_distribution<double>(0, 1)(prng) < m_fFakeUncertTIDE)
        return false;
    }

    if (isActiveTight || isActiveLooseIncl) {
        return acceptInclusive(track);
    }

    return true;
  }

  bool TrackFilterTool::isInJet( const xAOD::TrackParticle* track,
                                 const xAOD::Jet* jet ) const
  {
    if (track == nullptr || jet == nullptr)
        return false;

    // Jets below the configured pT threshold are ignored.
    if (jet->pt() < m_minJetPt)
        return false;

    constexpr bool useRapidity = false;

    return xAOD::P4Helpers::isInDeltaR(
        *track, *jet, m_deltaR, useRapidity);
  }




  StatusCode TrackFilterTool::initTrkEffSystHistogram(float scale, std::unique_ptr<TH2>& histogram, const std::string & rootFileName, const std::string & histogramName) const {

    ATH_CHECK( initObject<TH2>(histogram, rootFileName, histogramName) );

    for(int binx=1; binx<=histogram->GetNbinsX(); binx++) {
      for(int biny=1; biny<=histogram->GetNbinsY(); biny++) {
        // now using a histogram which is 1 - eff1/eff2, so we don't apply the "1 - content" correction ourselves
        float content = histogram->GetBinContent(binx, biny); // get the bin content
        content = std::fabs(content); // retrieve uncertainty and symmetrize (should be positive in histogram anyway)
        content *= scale; // scale systematic
        if(content > 1) content = 1; // protection: no larger than 100% uncertainty

        histogram->SetBinContent(binx, biny, content);
      }
    }

    return StatusCode::SUCCESS;
  }


  float TrackFilterTool::getNomTrkJetEff(const xAOD::TrackParticle* track) const
  {
    if (m_trkJetNomEff == nullptr) {
      ATH_MSG_ERROR( "Nominal track efficiency histogram is not property initialized!" );
      return 0.;
    }
    // this histogram has pt on the x-axis and eta on the y-axis, unlike some other histograms used in this package
    // make sure to convert to GeV
    return m_trkJetNomEff->GetBinContent(std::as_const(m_trkJetNomEff)->FindBin(track->pt()*1e-3, track->eta()));
  }

  bool TrackFilterTool::isAffectedBySystematic( const CP::SystematicVariation& syst ) const
  {
    return InDetTrackSystematicsTool::isAffectedBySystematic( syst );
  }

  CP::SystematicSet TrackFilterTool::affectingSystematics() const
  {
    return FilterSystematics;
  }

  CP::SystematicSet TrackFilterTool::recommendedSystematics() const
  {
    return InDetTrackSystematicsTool::recommendedSystematics();
  }

  StatusCode TrackFilterTool::applySystematicVariation( const CP::SystematicSet& systs )
  {
    return InDetTrackSystematicsTool::applySystematicVariation(systs);
  }




  bool TrackFilterTool::accept(
      const xAOD::TrackParticle* track,
      const CP::SystematicSet& syst) const
  {
    std::lock_guard<std::mutex> lock(m_sysLock);
    TrackFilterTool* nc_this ATLAS_THREAD_SAFE =
        const_cast<TrackFilterTool*>(this);
    if (nc_this->applySystematicVariation(syst).isFailure())
      throw std::invalid_argument("Systematic '" + syst.name()
          + "' was not pre-registered in initialize()");
    return accept(track);
  }


  bool TrackFilterTool::accept(
      const xAOD::TrackParticle* track,
      const xAOD::JetContainer* jets,
      const CP::SystematicSet& syst) const
  {
    std::lock_guard<std::mutex> lock(m_sysLock);
    TrackFilterTool* nc_this ATLAS_THREAD_SAFE =
        const_cast<TrackFilterTool*>(this);
    if (nc_this->applySystematicVariation(syst).isFailure())
      throw std::invalid_argument("Systematic '" + syst.name()
          + "' was not pre-registered in initialize()");
    return accept(track, jets);
  }

} // namespace InDet