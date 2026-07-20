/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */
#ifndef MUONACTSDUMP_TRUTHSEGMENTWRITER_H
#define MUONACTSDUMP_TRUTHSEGMENTWRITER_H

#include "AthenaBaseComps/AthHistogramAlgorithm.h"

#include "MuonTesterTree/MuonTesterTree.h"
#include "MuonTesterTree/ThreeVectorBranch.h"
#include "MuonPRDTest/SegmentVariables.h"

#include "ActsGeometryInterfaces/ITrackingGeometryTool.h"
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"
#include "xAODMuon/MuonSegmentContainer.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadDecorHandleKey.h"

#include "MuonIdHelpers/IMuonIdHelperSvc.h"


namespace Acts{
    class Surface;
}

namespace MuonValR4{
    class TruthSegmentWriter : public AthHistogramAlgorithm{
        public:
            using AthHistogramAlgorithm::AthHistogramAlgorithm;
            virtual StatusCode initialize() override final;
            virtual StatusCode execute(const EventContext& ctx)override final;
            virtual StatusCode finalize() override final;
        private:
            /** @brief Retrieves the surface associated with a sim hit identifier */
            const Acts::Surface& getSurface(const Identifier& simHitId) const;
            /** @brief Retrieves the surfaces associated with a segment */
            const Acts::Surface& getSurface(const xAOD::MuonSegment& segment) const;
            /** @brief The tool handle of the tracking geometry tool */
            PublicToolHandle<ActsTrk::ITrackingGeometryTool> m_trackingGeometryTool{this, "TrackingGeometryTool", ""};

            /** @brief Data dependency on the truth segment container */
            SG::ReadHandleKey<xAOD::MuonSegmentContainer> m_segmentKey{this, "segmentKey", "MuonTruthSegments"};
            /** @brief Data dependency on the truth particle link */
            SG::ReadDecorHandleKey<xAOD::MuonSegmentContainer> m_truthLinkKey{this, "truthLinkKey", m_segmentKey, "truthParticleLink"};
            
            /** @brief Service handle towards the IdHelper svc */
            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
            /** @brief The muon detector manager*/
            const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};

            /** @brief instance to the Event tree */
            MuonVal::MuonTesterTree m_tree{"MuonTruth", "ActsMuonTruthDump"};
            /** @brief The event number in this event */
            MuonVal::ScalarBranch<std::uint32_t>& m_eventId{m_tree.newScalar<std::uint32_t>("event_id")};
            /** @brief The pointer to the segment variables */
            std::shared_ptr<MuonPRDTest::SegmentVariables> m_segmentBranches{};
            /** @brief Pointer to the associated track vari */
            std::shared_ptr<MuonVal::CoordSystemsBranch> m_segmentFrames{};
            /** @brief The pointer to the associated truth particles */
            std::shared_ptr<MuonVal::IParticleFourMomBranch> m_truthTrks{};


    
    };
}


#endif
