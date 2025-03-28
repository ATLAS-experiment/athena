/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetTrackSystematicsTools/InclusiveTrackFilterTool.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "AthContainers/ConstAccessor.h"

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
    m_rnd = std::make_unique<TRandom3>(m_seed);

    if (m_calibFileLRTEff.empty()) {
      ATH_MSG_ERROR("No calibration file for requested LRT track efficiency set. You may be running an unsupported datataking period, please contact Tracking CP if you believe this message is in error.");
      return StatusCode::FAILURE;
    }

    TH2* trkLRTEff_tmp = nullptr;
    ATH_CHECK( initObject<TH2>( trkLRTEff_tmp,
               m_calibFileLRTEff, 
               m_calibHistLRTEff) );

    m_trkLRTEff = std::unique_ptr<TH2>(trkLRTEff_tmp);

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
      if(not patternReco.test(49)) {
        ATH_MSG_DEBUG( "Applying LRT uncertainties to non-LRT track! Skipping" );
        return true;
      }
    }

    if ( isActive( TRK_EFF_LARGED0_GLOBAL ) ) {
      float probDrop = std::abs(m_trkEffSystScale); // default is one; adjust this parameter to increase / decrease the effect
      probDrop *= std::abs(getTrackUncertainty( track ));

      if ( m_rnd->Uniform(0, 1) < probDrop ) return false;
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


} // namespace InDet
