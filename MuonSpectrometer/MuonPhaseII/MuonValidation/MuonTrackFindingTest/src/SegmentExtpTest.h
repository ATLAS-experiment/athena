/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTRACKFINDINGTEST_SEGMENTEXTPTEST_H
#define MUONTRACKFINDINGTEST_SEGMENTEXTPTEST_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"

#include "xAODMuon/MuonSegmentContainer.h"
#include "ActsGeometryInterfaces/IActsExtrapolationTool.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"

namespace MuonValR4{
    /*** @brief Algorithm to test the extrapolation on a fitted segment. For each segment,
     *          the fitted parameters are used to construct Acts Track paramters and to extrapolate
     *          the line onto each measurement's surface. The result is then compared with the obtained
     *          fit */
    class SegmentExtpTest : public AthReentrantAlgorithm{
        public:
            using AthReentrantAlgorithm::AthReentrantAlgorithm;
            virtual ~SegmentExtpTest() = default;
            virtual StatusCode initialize() override final;
            virtual StatusCode execute(const EventContext& ctx) const override final;
        private:
          /** @brief Declare the data dependency on the standard Mdt+Rpc+Tgc segment container */
        SG::ReadHandleKey<xAOD::MuonSegmentContainer> m_readKey{this, "SegmentKey", "MuonSegmentsFromR4"};
        /** @brief IdHelperSvc to decode the Identifiers */
        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "IdHelperSvc",  "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
        /** @brief Track extrapolation tool */
        ToolHandle<IActsExtrapolationTool> m_extrapolationTool{this, "ExtrapolationTool" ,"" };
        /** @brief Dependency on the geometry alignment */
        SG::ReadHandleKey<ActsTrk::GeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};
        /** @brief Detector manager to fetch the sector surfaces */
        const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};
    };
}


#endif