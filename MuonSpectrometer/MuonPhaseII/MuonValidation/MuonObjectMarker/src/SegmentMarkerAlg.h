/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONOBJECTMARKER_SEGMENTMARKERALG_H
#define MUONOBJECTMARKER_SEGMENTMARKERALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"

#include "xAODMuon/MuonSegmentContainer.h"
#include "xAODMuon/MuonContainer.h"

namespace MuonR4{
    /*** @brief SegmentMakerAlg copies selection decorations from the muon
     *          onto the xAOD::MuonSegment objects */
    class SegmentMarkerAlg: public AthReentrantAlgorithm{
       public:
         using AthReentrantAlgorithm::AthReentrantAlgorithm;
         ~SegmentMarkerAlg() = default;
          
          virtual StatusCode initialize() override final;
          virtual StatusCode execute(const EventContext& ctx) const override final;
        private:
            /** @brief Key to the primary muon container to select the muon from  */
            SG::ReadHandleKey<xAOD::MuonContainer> m_muonKey{this, "MuonKey", "Muons" };
            /** @brief Key to the decoration to fetch the marked muons */
            SG::ReadDecorHandleKey<xAOD::MuonContainer> m_readMarkKey{this, "SelectMuons", m_muonKey, ""};
            /** @brief Key to the segment container to fetch the marked segments */
            SG::ReadHandleKey<xAOD::MuonSegmentContainer> m_segKey{this, "SegmentKey", "Segments"};
            /** @brief Key to the decoration. Will be copied from m_readMarkKey */
            SG::WriteDecorHandleKey<xAOD::MuonSegmentContainer> m_writeMarkKey{this, "OutMarkerKey", m_segKey, ""};
    };
}

#endif