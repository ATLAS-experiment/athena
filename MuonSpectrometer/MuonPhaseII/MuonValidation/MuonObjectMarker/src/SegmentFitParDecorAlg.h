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
      *        Additionally, the ElementLinks to the associated measurements are decorated. For the latter,
      *        the Identifiers from the associated Trk::Segment are exploited. */
    class SegmentFitParDecorAlg : public AthReentrantAlgorithm{
        public:
            using AthReentrantAlgorithm::AthReentrantAlgorithm;

            virtual StatusCode initialize() override final;
            virtual StatusCode execute(const EventContext& ctx) const override final;
        private:
            using PrdCont_t = xAOD::UncalibratedMeasurementContainer;
            using PrdLink_t = ElementLink<PrdCont_t>;
            using PrdLinkVec = std::vector<PrdLink_t>;    
            using MeasKey_t = SG::ReadHandleKey<PrdCont_t>;
            
            /** @brief Tries to load the PrdCont_t from StoreGate and then to find the uncalibrated measurement
             *         with the same Identifier as the parsed one
             * @param ctx: EventContext to ease the access to StoreGate
             * @param key: Key of the container to fetch the measurement from
             * @param measId: Identifier of the measurement to find
             * @param meas: Reference to output pointer to which the found prd is assigned */
            StatusCode fetchMeasurement(const EventContext& ctx,
                                        const MeasKey_t& key,
                                        const Identifier& measId,
                                        const xAOD::UncalibratedMeasurement*& meas) const;
            /** @brief Tries to add the Link to the uncalibrated measurement corresponding to the passed ROT id
              * @param ctx: EventContext to ease the access to StoreGate
              * @param rotId: Identifier to link
              * @param prdLink: Reference to the target vector to which the new link is appended */
            StatusCode addLink(const EventContext& ctx,
                               const Identifier& rotId,
                                PrdLinkVec& prdLinks) const;

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
            MeasKey_t m_keysTgc{this, "sTgcKey", "xAODsTgcMeasurements"};
            MeasKey_t m_keyMM{this, "MmKey", "xAODMMClusters"};

    };
}

#endif
