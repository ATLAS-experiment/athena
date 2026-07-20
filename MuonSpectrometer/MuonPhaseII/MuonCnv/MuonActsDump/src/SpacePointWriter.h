/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */
#ifndef MUONACTSDUMP_SPACEPOINTWRITER_H
#define MUONACTSDUMP_SPACEPOINTWRITER_H


#include "AthenaBaseComps/AthHistogramAlgorithm.h"
#include "MuonTesterTree/MuonTesterTree.h"
#include "MuonTesterTree/ThreeVectorBranch.h"
#include "MuonTesterTree/CoordTransformBranch.h"
#include "MuonSpacePoint/SpacePointContainer.h"


#include "StoreGate/ReadHandleKeyArray.h"
#include "ActsGeometryInterfaces/ITrackingGeometryTool.h"

#include "MuonIdHelpers/IMuonIdHelperSvc.h"

#include "Acts/Geometry/GeometryIdentifier.hpp"

namespace MuonValR4{
    /** @brief Algorithm to write the Space points in a format that can
     *         later be read by the Acts Examples framework. Space points
     *         are expresed in the chamber frame and the local position
     *         as well as the two directions along the sensor and to the
     *         next sensor are dumped in a tree. Covariances and the 
     *         Identifiers complement the content */
    class SpacePointWriter : public AthHistogramAlgorithm {
        public:
            using AthHistogramAlgorithm::AthHistogramAlgorithm;
            virtual StatusCode initialize() override final;
            virtual StatusCode execute(const EventContext& ctx) override final;
            virtual StatusCode finalize() override final;
        private:
            /** @brief Encode the space point's identifier into the Identifier understood
             *         by ActsExamples */
            std::uint32_t encodeId(const MuonR4::SpacePoint& spacePoint,
                                   std::uint32_t gasGap) const;
            /** @brief Service handle towards the IdHelper svc */
            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

            /** @brief The tool handle of the tracking geometry tool */
            PublicToolHandle<ActsTrk::ITrackingGeometryTool> m_trackingGeometryTool{this, "TrackingGeometryTool", ""};
            
            SG::ReadHandleKeyArray<MuonR4::SpacePointContainer> m_spacePointKeys{this, "SpacePointKeys", {"MuonSpacePoints", "NswSpacePoints"} };

            /** @brief instance to the Event tree */
            MuonVal::MuonTesterTree m_tree{"MuonSpacePoints", "ActsMuonSpacePointDump"};
            /** @brief The event number in this event */
            MuonVal::ScalarBranch<std::uint32_t>& m_eventId{m_tree.newScalar<std::uint32_t>("event_id")};

            /** @brief Geometry identifier of the associated surface */
            using GeoId_t = Acts::GeometryIdentifier::Value;
            MuonVal::VectorBranch<GeoId_t>& m_geometryId{m_tree.newVector<GeoId_t>("spacePoint_geometryId")};
            /** @brief Bucket counter in the event */
            MuonVal::VectorBranch<std::uint16_t>& m_bucketId{m_tree.newVector<std::uint16_t>("spacePoint_bucketId")};
            /** @brief Identifier encoding the staion name && the coordinates measured by the space point */
            MuonVal::VectorBranch<std::uint32_t>& m_muonId{m_tree.newVector<std::uint32_t>("spacePoint_muonId")};
            /** @brief Position of the measurement **/
            MuonVal::ThreeVectorBranch m_localPosition{m_tree, "spacePoint_localPos"};
            /** @brief Orientation of the wire or strip  */
            MuonVal::UnitThreeVectorBranch m_sensorDirection{m_tree, "spacePoint_sensorDir"};
            /// @brief Vector pointing to the next channel in the same measurement plane
            MuonVal::UnitThreeVectorBranch m_toNextSensor{m_tree, "spacePoint_toNextDir"};
            /// @brief Covariance value along the non-bending direction
            MuonVal::VectorBranch<float>& m_covLoc0{m_tree.newVector<float>( "spacePoint_covLoc0")};
            /// @brief Covaraiance value along the bending direction
            MuonVal::VectorBranch<float>& m_covLoc1{m_tree.newVector<float>( "spacePoint_covLoc1")};
            /// @brief Time covariance value
            MuonVal::VectorBranch<float>& m_covT{m_tree.newVector<float>( "spacePoint_covT")};
            /// @brief Drift radius of the straw measurements
            MuonVal::VectorBranch<float>& m_driftR{m_tree.newVector<float>( "spacePoint_driftRadius")};
            /// @brief Recorded measurement time.
            MuonVal::VectorBranch<float>& m_time{m_tree.newVector<float>( "spacePoint_time")};
            /** @brief Coordinate transformation from the local measurement's frame
             *         to the common sector frame */
            MuonVal::CoordSystemsBranch m_toMeasFrame{m_tree, "spacePoint_toSectorFrame"};
    };
}

#endif