/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "GaudiKernel/ServiceHandle.h"

#include "TrackSystematicsAlg.h"

namespace InDet {
  TrackSystematicsAlg::TrackSystematicsAlg( const std::string& name, ISvcLocator* pSvcLocator ) : 
    AthReentrantAlgorithm( name, pSvcLocator ){}

  StatusCode TrackSystematicsAlg::initialize() {

    ATH_CHECK(m_inTrackKey.initialize());
    ATH_CHECK(m_outTrackKey.initialize());
    ATH_CHECK(m_decorDeps.initialize(m_inTrackKey, m_outTrackKey));

    // Retrieve the tools:
    ATH_CHECK(m_trackFilterToolLRT.retrieve());
    ATH_CHECK(m_trackFilterToolSTD.retrieve());

    CP::SystematicSet systSetLRT = {  
      CP::SystematicVariation("TRK_EFF_LARGED0_GLOBAL")
    };
    CP::SystematicSet systSetSTD = {  
      CP::SystematicVariation("TRK_EFF_LOOSE_COMBINED"),
    };

    ATH_CHECK(m_trackFilterToolLRT->applySystematicVariation(systSetLRT));
    ATH_CHECK(m_trackFilterToolSTD->applySystematicVariation(systSetSTD));

    return StatusCode::SUCCESS;
  }

  StatusCode TrackSystematicsAlg::execute(const EventContext& ctx) const {

    //Retrieve the tracks:
    SG::ReadHandle<xAOD::TrackParticleContainer> inTracks(m_inTrackKey, ctx);

    auto selectedTracks = std::make_unique<ConstDataVector<xAOD::TrackParticleContainer> >( SG::VIEW_ELEMENTS );

    for(const xAOD::TrackParticle* track : *inTracks) {

      const std::bitset<xAOD::NumberOfTrackRecoInfo> patternReco = track->patternRecoInfo();
      bool passFilter = false;

      // LRT track
      if(patternReco.test(49)) {
        passFilter = m_trackFilterToolLRT->accept(track);
      }
      // standard track
      else {
        passFilter = m_trackFilterToolSTD->accept(track);
      }
      if (passFilter) {
        ATH_MSG_DEBUG("Track accepted!");
        selectedTracks->push_back( track  );
      }
      else {
        ATH_MSG_DEBUG("Track rejected!");
      }
    }
    
    std::unique_ptr<const xAOD::TrackParticleContainer> outTracks(selectedTracks.release()->asDataVector());
    SG::WriteHandle<xAOD::TrackParticleContainer> outTrackHandle(m_outTrackKey, ctx);

    ATH_MSG_DEBUG( "Initial number of tracks: " << inTracks->size() );
    ATH_MSG_DEBUG( "Selected number of tracks: " << outTracks->size() );

    if(not outTrackHandle.put(std::move(outTracks))){
      ATH_MSG_ERROR("Failed to record " << m_outTrackKey.key() << " as const xAOD::TrackParticleContainer!");
      return StatusCode::FAILURE;
    }
    ATH_CHECK(m_decorDeps.linkDecors (m_inTrackKey, ctx));


    return StatusCode::SUCCESS;
  }
}
