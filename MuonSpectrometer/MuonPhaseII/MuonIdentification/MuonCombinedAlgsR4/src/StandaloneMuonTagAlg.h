/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONCOMBINEDALGSR4_STANDALONEMUONTAGALG_H
#define MUONCOMBINEDALGSR4_STANDALONEMUONTAGALG_H


#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include "MuonRecToolInterfacesR4/ITrackSummaryTool.h"

#include "ActsEvent/TrackContainer.h"
#include "MuonTrackEvent/MuonTag.h"

namespace MuonCombinedR4{
    /** @brief Algorithm to transform the produced MS tracks into a muon tag container
     *         and to associate the segments with the MuonTag */
    class StandaloneMuonTagAlg : public AthReentrantAlgorithm{
        public:
            using AthReentrantAlgorithm::AthReentrantAlgorithm;

            StatusCode initialize() override final;
            StatusCode execute(const EventContext& ctx) const override final;
        private:
            /** @brief Key of the input muon track container */
            SG::ReadHandleKey<xAOD::TrackParticleContainer> m_msTrackKey{this, "MsTracks", "MsTrackParticlesR4"};
            /** @brief Key to write the tag output container */
            SG::WriteHandleKey<MuonR4::MuonTagContainer> m_tagKey{this, "TagKey" , "MuonTagsSA"};
            /** @brief Handle to the muon summary tool */
            ToolHandle<MuonR4::ITrackSummaryTool> m_summaryTool{this, "TrackSummaryTool" , ""};
   };
}


#endif