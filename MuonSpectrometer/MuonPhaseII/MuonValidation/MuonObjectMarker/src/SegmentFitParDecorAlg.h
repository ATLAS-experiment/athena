/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONSEGMENTCNV_SEGMENTFITPARDECORALG_H
#define MUONSEGMENTCNV_SEGMENTFITPARDECORALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "StoreGate/WriteDecorHandleKey.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"

#include "MuonReadoutGeometryR4/MuonDetectorManager.h"
#include "ActsGeometryInterfaces/ActsGeometryContext.h"
#include "xAODMuon/MuonSegmentContainer.h"
#include "xAODMeasurementBase/UncalibratedMeasurementContainer.h"
namespace MuonR4{
    /** @brief Algorithm to decorate the segment fit parameters in the chamber's frame onto the xAOD::MuonSegment
      *        Additionally, the ElementLinks to the associated measurements are decorated */
    class SegmentFitParDecorAlg : public AthReentrantAlgorithm{
        public:
            using AthReentrantAlgorithm::AthReentrantAlgorithm;

            virtual StatusCode initialize() override final;
            virtual StatusCode execute(const EventContext& ctx) const override final;
        private:
            using MeasKey_t = SG::ReadHandleKey<xAOD::UncalibratedMeasurementContainer>;
            StatusCode fetchMeasurement(const EventContext& ctx,
                                        const MeasKey_t& key,
                                        const Identifier& measId,
                                        const xAOD::UncalibratedMeasurement*& meas) const;
            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", 
                                                                "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
            
            SG::ReadHandleKey<ActsGeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};

            const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};

            SG::ReadHandleKey<xAOD::MuonSegmentContainer> m_segmentKey{this, "SegmentKey", "Segments"};

            /** @brief Decoration key of the local parameters */
            SG::WriteDecorHandleKey<xAOD::MuonSegmentContainer> m_locParKey{this, "LocParKey", m_segmentKey, "localSegPars"};
            /** @brief Decoration key of the associated prep data objects */
            SG::WriteDecorHandleKey<xAOD::MuonSegmentContainer> m_prdLinkKey{this, "PrdLinkKey", m_segmentKey, "prdLinks"};

            MeasKey_t m_keyTgc{this, "TgcKey", "xTgcStrips"};
            MeasKey_t m_keyRpc{this, "RpcKey", "xRpcMeasurements"};
            MeasKey_t m_keyMdt{this, "MdtKey", "xMdtMeasurements"};
            MeasKey_t m_keysTgc{this, "sTgcKey", "STGC_Measurements"};
            MeasKey_t m_keyMM{this, "MmKey", "xAODMMClusters"};

    };
}

#endif