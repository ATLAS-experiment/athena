/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/  
#ifndef ACTSEVENTCNV_xAODtoTrkConverterAlgxAODtoTrkConverterAlg_h
#define ACTSEVENTCNV_xAODtoTrkConverterAlgxAODtoTrkConverterAlg_h


#include "ActsToolInterfaces/ITrackConverterTool.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"


#include "TrkTrack/TrackCollection.h"
#include "xAODTracking/TrackParticleContainer.h"

#include "TrkTrack/TrackCollection.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"

namespace ActsTrk {
    /** @brief Algorithm that converts the Acts::Tracks from which the xAOD::TrackParticle is
     *         made into a Trk::Track object and fills the `trackLink` of the xAOD::TrackParticle */
    class xAODtoTrkConverterAlg : public AthReentrantAlgorithm {
        public:
            using AthReentrantAlgorithm::AthReentrantAlgorithm;
            virtual StatusCode initialize() override final;
            virtual StatusCode execute(const EventContext& ctx) const override final;
        private:
            /** @brief Input read handle key collection */
            SG::ReadHandleKey<xAOD::TrackParticleContainer> m_trackKey{this, "TrackParticles", "InDetTrackParticles"};
            /** @brief Declare the data dependency on the trackLink */
            SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_linkKey{this, "LinkKey", m_trackKey, "trackLink"};
            /** @brief Declare the output container of the track collection */
            SG::WriteHandleKey<TrackCollection> m_writeKey{this, "OutTrackContainer", "CombinedInDetTracks"};
            /** @brief Conversion tool to convert from Acts -> Trk tracks */
            ToolHandle<ITrackConverterTool> m_ATLASConverterTool{this, "ATLASConverterTool", ""};

    };
}
#endif