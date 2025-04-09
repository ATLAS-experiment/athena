/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONMEASVIEWALGS_SEGMENTVIEWALG_H
#define XAODMUONMEASVIEWALGS_SEGMENTVIEWALG_H

#include <AthenaBaseComps/AthReentrantAlgorithm.h>
#include <StoreGate/ReadHandleKeyArray.h>
#include <StoreGate/WriteHandleKey.h>
#include <xAODMuon/MuonSegmentContainer.h>
namespace MuonR4{
    class SegmentViewAlg : public AthReentrantAlgorithm {
        public:
            using AthReentrantAlgorithm::AthReentrantAlgorithm;

            StatusCode initialize() override final;
            StatusCode execute(const EventContext& ctx) const override final;
        private:
            SG::ReadHandleKeyArray<xAOD::MuonSegmentContainer> m_readKeys{this, "SegmentsKeys", {}};
            SG::WriteHandleKey<xAOD::MuonSegmentContainer> m_writeKey{this, "ViewKey",""};

    };
}

#endif