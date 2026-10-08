/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TrackingGeometryTest_TrackingGeometryTest_H
#define TrackingGeometryTest_TrackingGeometryTest_H

#include "xAODTruth/TruthParticleContainer.h"
#include "xAODMuonSimHit/MuonSimHitContainer.h"

#include "AthenaBaseComps/AthHistogramAlgorithm.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"

#include "MuonTesterTree/MuonTesterTree.h"
#include "MuonTesterTree/ThreeVectorBranch.h"
#include "MuonTesterTree/TwoVectorBranch.h"
#include "MuonTesterTree/IdentifierBranch.h"

#include "ActsGeometryInterfaces/ITrackingGeometrySvc.h"
#include "ActsGeometryInterfaces/IExtrapolationTool.h"
#include "ActsGeometry/ContextUtility.h"

#include "StoreGate/ReadHandleKey.h"


namespace MuonValR4 {

    /** Extrapolation test for the ActsMuonTrackingGeometry for gen3 */

    class TrackingGeometryTest: public AthHistogramAlgorithm {

      public:
        using AthHistogramAlgorithm::AthHistogramAlgorithm;

        ~TrackingGeometryTest() = default;

        StatusCode initialize() override;
        StatusCode execute(const EventContext& ctx) override;       
        StatusCode finalize() override;

      private:
        /** @brief Returns the associated surface to an Identifier */
        std::shared_ptr<const Acts::Surface> surface(const Identifier& id) const;
        /** @brief Returns the hash to fetch a surface */
        IdentifierHash surfaceHash(const Identifier& id) const;

        std::optional<Acts::BoundTrackParameters>
            createBoundPars(const Acts::GeometryContext& tgContext,
                            const xAOD::MuonSimHit& simHit) const;


        /** @brief Context utility to access the magnetic field context */
        ActsTrk::ContextUtility m_ctxProvider{this};
        /** @brief the muon Detector manager */
        const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};

        /** @brief Service handle to the tracking geometry service */
        ServiceHandle<ActsTrk::ITrackingGeometrySvc> m_trackingGeometrySvc{this, "TrackingGeometrySvc", "ActsTrackingGeometrySvc"};
        /** @brief Service handle to the muon idHelper Svc */
        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "IdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
        /** @brief Tool handle to the extrapolation tool */
        ToolHandle<ActsTrk::IExtrapolationTool> m_extrapolationTool{this, "ExtrapolationTool", ""};
        /** @brief Data dependency on the truth particles from the event */
        SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthParticleKey{this, "TruthKey", "MuonTruthParticles"};
        /** @brief Data dependency on the truth segment matching */
        SG::ReadDecorHandleKey<xAOD::TruthParticleContainer> m_truthSegLinkKey{this, "TruthKeyToSeg", m_truthParticleKey, "truthSegmentLinks", "TruthParticle to TruthSegment link"};

        Gaudi::Property<bool> m_startFromFirstHit{this, "StartFromFirstHit", false, "Start from first hit"};

        MuonVal::MuonTesterTree m_tree{"MuonNavigationTest", "MuonNavigationTestGen3R4"};
        
        /** @brief Particle information  */
        MuonVal::ScalarBranch<float>& m_truthPt{m_tree.newScalar<float>("truth_pt")};
        MuonVal::ScalarBranch<float>& m_eta{m_tree.newScalar<float>("truth_eta")};
        MuonVal::ScalarBranch<float>& m_phi{m_tree.newScalar<float>("truth_phi")};
        MuonVal::ScalarBranch<char>& m_q{m_tree.newScalar<char>("truth_q")};
        MuonVal::ScalarBranch<std::uint16_t>& m_pdgId{m_tree.newScalar<std::uint16_t>("truth_pdgId")};
        /** @brief Needed time to propagate the particle through  */
        MuonVal::ScalarBranch<float>& m_propTime{m_tree.newScalar<float>("propTime")};
        
        /** @brief Identifier branches of the simulated hits */
        MuonVal::MuonIdentifierBranch m_detId{m_tree, "simHit_id"};
        MuonVal::VectorBranch<unsigned short>& m_techIdx{m_tree.newVector<unsigned short>("simHit_id_techIdx")};
        MuonVal::VectorBranch<unsigned short>& m_gasGapId{m_tree.newVector<unsigned short>("simHit_id_gasGap")};
        /** @brief The two loc0 and loc1 parameters */
        MuonVal::TwoVectorBranch m_truthLoc{m_tree, "simHit_locPos"};
        /** @brief The direction in global frame */
        MuonVal::UnitThreeVectorBranch m_truthDir{m_tree, "simHit_globDir"};
        /** @brief The position of the sim hit in global frame */
        MuonVal::ThreeVectorBranch m_truthGlob{m_tree, "simHit_globPos"};
        /** @brief The truth hit momentum */
        MuonVal::VectorBranch<float>& m_truthP{m_tree.newVector<float>("simHit_p")};
        /** @brief The Acts geometry Identifier */
        MuonVal::VectorBranch<Acts::GeometryIdentifier::Value>& m_truthGeoId{
                m_tree.newVector<Acts::GeometryIdentifier::Value>("simHit_geoId")};
        /** @brief Flag to toggle whether a propagation was found */
        MuonVal::VectorBranch<std::uint8_t>& m_isPropagated{m_tree.newVector<std::uint8_t>("simHit_extpPars")};

        /** @brief Local position of the extrapolated parameters */
        MuonVal::TwoVectorBranch m_propLocPos{m_tree, "extp_locPos"};
        /** @brief The direction in the ATLAS frame */
        MuonVal::UnitThreeVectorBranch m_propDir{m_tree, "extp_globDir"};
        /** @brief The extrapolated parameter position in the global frame */
        MuonVal::ThreeVectorBranch m_propGlobPos{m_tree, "extp_globPos"};
        /** @brief The momentum of the extrapolated parameters */
        MuonVal::VectorBranch<float>& m_propP{m_tree.newVector<float>("extp_p")};
        /** @brief The charge of the extrapolated parameters */
        MuonVal::VectorBranch<char>& m_propQ{m_tree.newVector<char>("extp_q")};
        /** @brief The type of the surface of the extrapolated parameters */
        MuonVal::VectorBranch<std::uint8_t>& m_propSurfType{m_tree.newVector<std::uint8_t>("extp_surfaceType")};
        /** @brief Link to the associated hit*/
        MuonVal::VectorBranch<std::uint8_t>& m_propHitLink{m_tree.newVector<std::uint8_t>("extp_hitLink")};
        /** @brief The Acts geometry Identifier */
        MuonVal::VectorBranch<Acts::GeometryIdentifier::Value>& m_propGeoId{
                m_tree.newVector<Acts::GeometryIdentifier::Value>("extp_geoId")};
    };
}

#endif