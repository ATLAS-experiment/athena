/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONSEGMENTCNV_TRKSEGMENTCNVALG_H
#define MUONSEGMENTCNV_TRKSEGMENTCNVALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadHandleKeyArray.h"

#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonPrepRawData/MuonPrepDataContainer.h"
#include "MuonPatternEvent/MuonPatternContainer.h"
#include "TrkSegment/SegmentCollection.h"

#include "MuonRecToolInterfaces/IMdtDriftCircleOnTrackCreator.h"
#include "MuonRecToolInterfaces/IMuonClusterOnTrackCreator.h"
#include "MuonRecToolInterfaces/IMuonCompetingClustersOnTrackCreator.h"
#include "MuonRecHelperTools/MuonEDMPrinterTool.h"

namespace MuonR4{
    /** @brief The TrkSegmentCnvAlg converts the SegmentSeeds produced by the R4 pattern recognition chain
     *         into the segment seeds that can be consumed by the legacy muon segment maker
     */
    class TrkSegmentCnvAlg : public AthReentrantAlgorithm {
        public:

            using AthReentrantAlgorithm::AthReentrantAlgorithm;

            StatusCode initialize() override final;
            StatusCode execute(const EventContext& ctx) const override final;
    
        private:
            /** @brief Convert the R4 segment and fill the converted segment into the SegmentCollection
             *  @param ctx: EventContext to access the alignment & calibration constants
             *  @param segment: Reference to the segment to convert
             *  @param outContainer: Reference to the output container to parse */
            StatusCode convert(const EventContext& ctx,
                               const MuonR4::Segment& segment,
                               Trk::SegmentCollection& outContainer) const;
            /*** @brief Fetches a MuonPrepData object from the PrepData container by matching the parsed Identifier.
               *        Nullptr is returned if the object does not exist and an error message is printed
               * @param prdId: Identifier of the measurement to fetch
               * @param prdContainer: Pointer to the MuonPrepData container to fetch the object from.  */
            template <class PrdType> 
                const PrdType* fetchPrd(const Identifier& prdId,
                                        const Muon::MuonPrepDataContainerT<PrdType>* prdContainer) const;

            using RotVec = std::vector<std::unique_ptr<Trk::RIO_OnTrack>>;
            template <class PrdType>
                StatusCode convertMeasurement(const MuonR4::Segment& segment,
                                              const CalibratedSpacePoint& spacePoint,
                                              const Muon::MuonPrepDataContainerT<PrdType>* prdContainer,
                                              RotVec& convMeasVec) const;

            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", 
                                                                "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
            
            PublicToolHandle<Muon::MuonEDMPrinterTool> m_printer{this, "printerTool", "Muon::MuonEDMPrinterTool/MuonEDMPrinterTool"};
            /** @brief Prep data container keys */
            SG::ReadHandleKey<Muon::TgcPrepDataContainer> m_keyTgc{this, "TgcKey", "TGC_MeasurementsAllBCs"};
            SG::ReadHandleKey<Muon::RpcPrepDataContainer> m_keyRpc{this, "RpcKey", "RPC_Measurements"};
            SG::ReadHandleKey<Muon::MdtPrepDataContainer> m_keyMdt{this, "MdtKey", "MDT_DriftCircles"};
            SG::ReadHandleKey<Muon::sTgcPrepDataContainer> m_keysTgc{this, "sTgcKey", "STGC_Measurements"};
            SG::ReadHandleKey<Muon::MMPrepDataContainer> m_keyMM{this, "MmKey", "MM_Measurements"};

            // ACTS geometry context
            SG::ReadHandleKey<ActsTrk::GeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};

           
            SG::ReadHandleKeyArray<SegmentContainer> m_readKeys{this, "ReadSegments", {"R4MuonSegments"}};

            SG::WriteHandleKey<Trk::SegmentCollection> m_writeKey{this, "WriteKey", "TrackMuonSegmentsR4"};


            ToolHandle<Muon::IMdtDriftCircleOnTrackCreator> m_mdtCreator{this,"MdtRotCreator",""};  //<! pointer to mdt rio ontrack creator
            ToolHandle<Muon::IMuonClusterOnTrackCreator> m_clusterCreator{this,"ClusterRotCreator",""};

            ToolHandle<Muon::IMuonCompetingClustersOnTrackCreator> m_compClusterCreator{this, "CompetingRotCreator", ""};  //<! competing clusters rio ontrack creator

    };

}

#endif