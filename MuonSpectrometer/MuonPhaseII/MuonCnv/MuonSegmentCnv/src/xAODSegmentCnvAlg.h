/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONSEGMENTCNV_XAODSEGMENTCNVALG_H
#define MUONSEGMENTCNV_XAODSEGMENTCNVALG_H


#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"

#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonPatternEvent/MuonPatternContainer.h"
#include "xAODMuon/MuonSegmentContainer.h"
#include "xAODMuonPrepData/CombinedMuonStripContainer.h"
#include "xAODMeasurementBase/UncalibratedMeasurementContainer.h" 

namespace MuonR4{
    /** @brief The xAODSegmentCnvAlg takes MuonR4::Segments and converts them into a
     *         xAOD::MuonSegmentContainer */
    class xAODSegmentCnvAlg: public AthReentrantAlgorithm {
        public:
            using AthReentrantAlgorithm::AthReentrantAlgorithm;

            virtual StatusCode initialize() override final;
            virtual StatusCode execute(const EventContext& ctx) const override final;
        
        private:
            /** @brief IdHelperSvc for Identifier printing & manipulation */
            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc",  
                                                                "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
            /** @brief Input segment container key */
            SG::ReadHandleKeyArray<SegmentContainer> m_readKeys{this, "InSegmentKeys", {"R4MuonSegments"}};
            /** @brief Output segment container key */
            SG::WriteHandleKey<xAOD::MuonSegmentContainer> m_writeKey{this, "OutSegmentKey", "MuonSegmentsFromR4"};
            /** @brief Alignment container key */            
            SG::ReadHandleKey<ActsTrk::GeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};
            /** @brief Abrivation of the extra declared auxVariables  */
            using DecorKey_t = SG::WriteDecorHandleKey<xAOD::MuonSegmentContainer>;
            /** @brief Decoration to the links to the associated Uncalibrated measurements */
            DecorKey_t m_prdLinkKey{this, "PrdLinkKey",  m_writeKey, "prdLinks" };
            /** @brief Decoration to the PrdLink state (I.e. outlier or valid) */
            DecorKey_t m_prdStateKey{this, "PrdStateKey", m_writeKey, "prdState"};
            /** @brief Decoration to the local segment parameters */
            DecorKey_t m_localSegParKey{this, "LocalSegParKey", m_writeKey, "localSegPars"};
            /** @brief Decoration of the original segment */
            DecorKey_t m_parentSegKey{this, "ParentSegmentKey", m_writeKey, "parentSegment"};
            /** @brief Auxiliary container to model two measurements in the same gas gap as a single track state */
            SG::WriteHandleKey<xAOD::CombinedMuonStripContainer> m_combMeasKey{this, "combinedPrdKey", "CombinedMuonPrds"};
    };
}
#endif