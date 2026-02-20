/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTRUTHSEGMENTMAKER_TruthSegToTruthPartAssocAlg_H
#define MUONTRUTHSEGMENTMAKER_TruthSegToTruthPartAssocAlg_H

#include <AthenaBaseComps/AthReentrantAlgorithm.h>

#include <StoreGate/ReadHandleKey.h>
#include <StoreGate/ReadDecorHandleKeyArray.h>
#include <StoreGate/WriteDecorHandleKey.h>
#include <StoreGate/WriteDecorHandle.h>

#include "xAODTruth/TruthParticleContainer.h"
#include "xAODMuon/MuonSegmentContainer.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"
#include "ActsGeometryInterfaces/IExtrapolationTool.h"
#include "ActsGeometryInterfaces/ITrackingGeometryTool.h"


namespace MuonR4{
    /** @brief The TruthSegToTruthPartAssocAlg associates the TruthSegments with the primary TruthParticle
     *         from the IP. At the same time, the truth particles are linked to all the associated segment candidates.
     *         
     *         To perform the matching, the SDO identifiers decorated to the TruthParticle are compared with the SDO
     *         identifiers of the truth hits making up the truth segment.  */
    class TruthSegToTruthPartAssocAlg: public AthReentrantAlgorithm {
        public:
            using AthReentrantAlgorithm::AthReentrantAlgorithm;

            virtual StatusCode initialize() override final;
            virtual StatusCode execute(const EventContext& ctx) const override final;
        private:
            using TruthPartLink_t = ElementLink<xAOD::TruthParticleContainer>;
            using TruthPartDecor_t = SG::WriteDecorHandle<xAOD::MuonSegmentContainer, TruthPartLink_t>;
            using TruthSegLinkVec_t = std::vector<ElementLink<xAOD::MuonSegmentContainer>>;
            using TruthSegLinkDecor_t = SG::WriteDecorHandle<xAOD::TruthParticleContainer, TruthSegLinkVec_t>;

            /** @brief Match truth muons without any HEPMC link with truth segments reconstructed
             *         in the MS
             * @param ctx: EventContext to access the geometry & conditions
             * @param pileUpMuons: List of background muons
             * @param pileUpSegments: List of pile-up segments to match
             * @param truthPartDecor: Decorator to decorate the associated muon to the segment
             * @param truthSegDecor: Decorator to pin the pile-up segments to the truth muon */
            void matchPileupSegments(const EventContext& ctx,
                                     const std::vector<const xAOD::TruthParticle*>& pileUpMuons,
                                     const std::vector<const xAOD::MuonSegment*>& pileUpSegments,
                                     TruthPartDecor_t& truthPartDecor,
                                     TruthSegLinkDecor_t& truthSegDecor) const;
            /** @brief IdHelperSvc to decode the Identifiers */
            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "IdHelperSvc",  "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
            /** @brief Key to the truth particle container to associate */
            SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthKey{this, "TruthKey", "MuonTruthParticles"};
            /** @brief List of simHit id decorations to read from the truth particle */
            Gaudi::Property<std::vector<std::string>> m_simHitIds{this, "SimHitIds", {}};
            /** @brief Declaration of the dependency on the simHit decorations */
            SG::ReadDecorHandleKeyArray<xAOD::TruthParticleContainer> m_simHitKeys{this, "TruthSimHitIdKeys", {}};
            /** @brief Declaration of the segmentLink to the truth particle */
            SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_segLinkKey{this, "SegmentToPartKey", m_truthKey, "truthSegmentLinks"};
            /** @brief Key to the truth segment container to associate */
            SG::ReadHandleKey<xAOD::MuonSegmentContainer> m_segmentKey{this, "SegmentKey", "MuonTruthSegments"};
            /** @brief Key of the truthParticleLink decorated onto the segment */
            SG::WriteDecorHandleKey<xAOD::MuonSegmentContainer> m_truthLinkKey{this, "TruthToPartKey", m_segmentKey, "truthParticleLink"};
            /** @brief Property checking whether a pile-up muons shall be matched to pile-up segments */
            Gaudi::Property<bool> m_includePileUpObjs{this, "includePileUpObjs", false};
            /** @brief Delta phi cut between particle momentum & segment position to start the extrapolation */
            Gaudi::Property<double> m_pileUpObjDPhiCut{this,"deltaPhiSegBkgMuon", 10.*Gaudi::Units::deg};
            /** @brief Cut on the delta x0 between the bkg segment and the extrapolated parameters */
            Gaudi::Property<double> m_pileUpObjExtpDxCut{this,"BkgMatchingExtpX0", 10.*Gaudi::Units::cm};
            /** @brief Cut on the delta y0 between the bkg segment and the extrapolated parameters */
            Gaudi::Property<double> m_pileUpObjExtpDyCut{this,"BkgMatchingExtpY0", 10.*Gaudi::Units::mm};
            /** @brief Cut on the delta theta between the bkg segment and the extrapolated parameters */
            Gaudi::Property<double> m_pileUpObjExtpDthetaCut{this,"BkgMatchingExtpTheta", 0.5*Gaudi::Units::deg};
            /** @brief Cut on the delta phi between the bkg segment and the extrapolated parameters */
            Gaudi::Property<double> m_pileUpObjExtpDphiCut{this,"BkgMatchingExtpPhi", 5.*Gaudi::Units::deg};
             
            /** @brief Tracking geometry tool */
            PublicToolHandle<ActsTrk::ITrackingGeometryTool> m_trackingGeometryTool{this, "TrackingGeometryTool", ""};
            /** @brief Track extrapolation tool */
            ToolHandle<ActsTrk::IExtrapolationTool> m_extrapolationTool{this, "ExtrapolationTool" ,"" };
            /** @brief Pointer to the muon detector manager */
            const MuonGMR4::MuonDetectorManager* m_detMgr{};

    };
}
#endif