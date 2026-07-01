/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONSEGMENTCNV_SEGMENTFITPARDECORALG_H
#define MUONSEGMENTCNV_SEGMENTFITPARDECORALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "StoreGate/WriteDecorHandleKey.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"

#include "MuonReadoutGeometryR4/MuonDetectorManager.h"
#include "ActsGeometryInterfaces/GeometryContext.h"
#include "xAODMuon/MuonSegmentContainer.h"
#include "xAODMuonPrepData/MuonMeasurementContainer.h"
namespace MuonR4{
    /** @brief Algorithm to decorate the local segment parameters onto the xAOD::MuonSegments
     *         produced by the legacy muon reconstruction chain. Additionally, the links to the
     *         uncalibrated muon measurements are appended using the Identifiers of the RIO_OnTrack
     *         objects of the associated Trk::MuonSegment */
    class SegmentFitParDecorAlg : public AthReentrantAlgorithm{
        public:
            using AthReentrantAlgorithm::AthReentrantAlgorithm;

            virtual StatusCode initialize() override final;
            virtual StatusCode execute(const EventContext& ctx) const override final;
        private:
            using PrdCont_t = xAOD::MuonMeasurementContainer;
            using PrdLink_t = ElementLink<xAOD::UncalibratedMeasurementContainer>;
            using PrdLinkVec = std::vector<PrdLink_t>;    
            using MeasKey_t = SG::ReadHandleKey<PrdCont_t>;
            using TechIdx_t = Muon::MuonStationIndex::TechnologyIndex;

            /** @brief Fetch the read handle key to the Muon measurement container
             *  @param idx: The technology index indicating which key should be returned */
            const MeasKey_t& fetchKey(const TechIdx_t idx) const;
            
            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", 
                                                                "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
            
            ActsTrk::GeoContextReadKey_t m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};

            const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};

            SG::ReadHandleKey<xAOD::MuonSegmentContainer> m_segmentKey{this, "SegmentKey", "Segments"};

            /** @brief Decoration key of the local parameters */
            SG::WriteDecorHandleKey<xAOD::MuonSegmentContainer> m_locParKey{this, "LocParKey", m_segmentKey, "localSegPars"};
            /** @brief Decoration key of the associated prep data objects */
            SG::WriteDecorHandleKey<xAOD::MuonSegmentContainer> m_prdLinkKey{this, "PrdLinkKey", m_segmentKey, "prdLinks"};

            MeasKey_t m_keyTgc{this, "TgcKey", "xTgcStrips"};
            MeasKey_t m_keyRpc{this, "RpcKey", "xRpcMeasurements"};
            MeasKey_t m_keyMdt{this, "MdtKey", "xMdtMeasurements"};
            MeasKey_t m_keysTgc{this, "sTgcKey", "xAODsTgcMeasurements"};
            MeasKey_t m_keyMM{this, "MmKey", "xAODMMClusters"};

    };
}

#endif
