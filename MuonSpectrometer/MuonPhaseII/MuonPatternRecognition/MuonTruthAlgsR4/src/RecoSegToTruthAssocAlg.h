/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTRUTHALGSR4_RECOSEGTOTRUTHASSOCALG_H
#define MUONTRUTHALGSR4_RECOSEGTOTRUTHASSOCALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "xAODMuon/MuonSegmentContainer.h"
#include "xAODMuonSimHit/MuonSimHit.h"
#include "xAODMeasurementBase/UncalibratedMeasurementContainer.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/ReadDecorHandleKeyArray.h"

namespace MuonR4{
    /** @brief Algorithm to match the reconstructed muon segment with the truth segment & 
     *         with the truth particle. */
    class RecoSegToTruthAssocAlg : public AthReentrantAlgorithm{
        public:
            using AthReentrantAlgorithm::AthReentrantAlgorithm;
            virtual StatusCode initialize() override final;
            virtual StatusCode execute(const EventContext& ctx) const override final;
        private:
            /** @brief Helper struct of segments with simHits & associated spectrometer sector */
            struct SegmentWithTruth{
                /** @brief segment pointer of interest */
                const xAOD::MuonSegment* segment{nullptr};
                /** @brief list of associated sim hits */
                std::unordered_set<const xAOD::MuonSimHit*> hits{};
            };
            using SegWithTruthVec_t = std::vector<SegmentWithTruth>;
            /** @brief Loops over the segment container and fetches the segments with truth matched hits */
            std::vector<SegmentWithTruth> matchSimHits(const xAOD::MuonSegmentContainer& segments) const;
            /** @brief Key to the truth segment container */
            SG::ReadHandleKey<xAOD::MuonSegmentContainer> m_truthSegKey{this, "TruthSegKey", "MuonTruthSegments"};
            /** @brief Key to the truth segment -> truth particle association */
            SG::ReadDecorHandleKey<xAOD::MuonSegmentContainer> m_truthSegLinkKey{this, "TruthSegLinkKey", m_truthSegKey, "truthParticleLink"};
            /** @brief Key to the reconstructed segment container to truth match */
            SG::ReadHandleKey<xAOD::MuonSegmentContainer> m_segmentKey{this, "SegmentKey", "RecoSegs"};
            /** @brief Key to the associated uncalibrated measurement decoration */
            SG::ReadDecorHandleKey<xAOD::MuonSegmentContainer> m_segPrdLinkKey{this, "SegPrdLinkKey", m_segmentKey, "prdLinks"};
            /** @brief Output key to the associated truth segment link decoration */
            SG::WriteDecorHandleKey<xAOD::MuonSegmentContainer> m_segTruthSegLinkKey{this, "SegToTruthSegLinkKey", m_segmentKey, "truthSegmentLink"};
            /** @brief Output key to the associated truth particle decoration */
            SG::WriteDecorHandleKey<xAOD::MuonSegmentContainer> m_segTruthLinkKey{this, "SegTruthLinkKey", m_segmentKey, "truthParticleLink"};
            /** @brief Decoration key of the prd -> simHit association */
            Gaudi::Property<std::string> m_simLink{this, "SimHitLink", "simHitLink"};
            /** @brief Key to the prd containers in the event */
            SG::ReadHandleKeyArray<xAOD::UncalibratedMeasurementContainer> m_measKeys{this, "PrdContainer",{}};
            /** @brief Dependency on the sim hit decoration */
            SG::ReadDecorHandleKeyArray<xAOD::UncalibratedMeasurementContainer> m_prdLinkKeys{this, "PrdLinkKeys", {}};
    };
}
#endif
