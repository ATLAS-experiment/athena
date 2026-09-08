/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetTrackSystematicsTools/InclusiveTrackFilterTool.h"
#include "InDetTrackSystematicsTools/getEventNumber.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/TrackingPrimitives.h"
#include "AthContainers/ConstAccessor.h"
#include "CxxUtils/checker_macros.h"
#include "CxxUtils/FastReseededPRNG.h"
#include <random>

namespace InDet {

  static const CP::SystematicSet FilterSystematics = 
    {
      InDet::TrackSystematicMap.at(TRK_EFF_LARGED0_GLOBAL),
    };

  InclusiveTrackFilterTool::InclusiveTrackFilterTool(const std::string& name) :
    InDetTrackSystematicsTool(name)
  {
#ifndef XAOD_STANDALONE
    declareInterface<IInclusiveTrackFilterTool>(this);
#endif
  }

  StatusCode InclusiveTrackFilterTool::initialize()
  {
    if (m_calibFileLRTEff.empty()) {
      ATH_MSG_ERROR("No calibration file for requested LRT track efficiency set. You may be running an unsupported datataking period, please contact Tracking CP if you believe this message is in error.");
      return StatusCode::FAILURE;
    }

    ATH_CHECK( initObject<TH2>( m_trkLRTEff,
               m_calibFileLRTEff, 
               m_calibHistLRTEff) );

    ATH_CHECK ( InDetTrackSystematicsTool::initialize() );

    return StatusCode::SUCCESS;
  }


  InclusiveTrackFilterTool::~InclusiveTrackFilterTool()
  {
  }

  bool InclusiveTrackFilterTool::accept(const xAOD::TrackParticle* track) const
  {

    if ( track == nullptr) {
      ATH_MSG_DEBUG( "Pointer to track is null!" );
      return false;
    }

    static const SG::ConstAccessor<unsigned long> patternRecoInfoAcc ("patternRecoInfo");
    if (patternRecoInfoAcc.isAvailable(*track) ) {
      const std::bitset<xAOD::NumberOfTrackRecoInfo> patternReco = track->patternRecoInfo();
      if(not patternReco.test(xAOD::SiSpacePointsSeedMaker_LargeD0)) {
        ATH_MSG_DEBUG( "Applying LRT uncertainties to non-LRT track! Skipping" );
        return true;
      }
    }

    if ( isActive( TRK_EFF_LARGED0_GLOBAL ) ) {
      float probDrop = std::abs(m_trkEffSystScale);
      probDrop *= std::abs(getTrackUncertainty( track ));
      FastReseededPRNG prng(
          m_seed,
          static_cast<uint32_t>(std::abs(track->phi()) * 1e6),
          static_cast<uint32_t>(std::abs(track->eta()) * 1e3),
          InDet::getEventNumber(evtStore()));
      if (std::uniform_real_distribution<double>(0, 1)(prng) < probDrop)
          return false;
    }

    return true;
  }

  float InclusiveTrackFilterTool::getTrackUncertainty(const xAOD::TrackParticle* track) const
  {
    if (m_trkLRTEff == nullptr) {
      ATH_MSG_ERROR( "LRT track efficiency histogram is not property initialized!" );
      return 0.;
    }
    return m_trkLRTEff->GetBinContent(std::as_const(m_trkLRTEff)->FindBin(track->radiusOfFirstHit(), track->eta()));
  }

  bool InclusiveTrackFilterTool::isAffectedBySystematic( const CP::SystematicVariation& syst ) const
  {
    return InDetTrackSystematicsTool::isAffectedBySystematic( syst );
  }

  CP::SystematicSet InclusiveTrackFilterTool::affectingSystematics() const
  {
    return FilterSystematics;
  }

  CP::SystematicSet InclusiveTrackFilterTool::recommendedSystematics() const
  {
    return InDetTrackSystematicsTool::recommendedSystematics();
  }

  StatusCode InclusiveTrackFilterTool::applySystematicVariation( const CP::SystematicSet& systs )
  {
    return InDetTrackSystematicsTool::applySystematicVariation(systs);
  }

  bool InclusiveTrackFilterTool::accept(
      const xAOD::TrackParticle* track,
      const CP::SystematicSet& syst) const
  {
    std::lock_guard<std::mutex> lock(m_sysLock);
    InclusiveTrackFilterTool* nc_this ATLAS_THREAD_SAFE =
        const_cast<InclusiveTrackFilterTool*>(this);
    if (nc_this->applySystematicVariation(syst).isFailure())
      throw std::invalid_argument("Systematic '" + syst.name()
          + "' was not pre-registered in initialize()");
    return accept(track);
  }

} // namespace InDet
