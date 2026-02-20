/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSMUONTRACKINGGEOMETRYTEST_ACTSMUONTRACKINGGEOMETRYTEST_H
#define ACTSMUONTRACKINGGEOMETRYTEST_ACTSMUONTRACKINGGEOMETRYTEST_H

#include "xAODTruth/TruthParticleContainer.h"
#include "xAODMuonSimHit/MuonSimHitContainer.h"

#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include "AthenaKernel/IAthRNGSvc.h"
#include <MuonIdHelpers/IMuonIdHelperSvc.h>
#include <MuonReadoutGeometryR4/MuonDetectorManager.h>
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonTesterTree/MuonTesterTree.h"
#include "MuonTesterTree/ThreeVectorBranch.h"
#include "MuonTesterTree/IdentifierBranch.h"

#include "ActsGeometry/ActsTrackingGeometrySvc.h"
#include "ActsGeometryInterfaces/ITrackingGeometryTool.h"
#include "ActsGeometryInterfaces/IExtrapolationTool.h"
#include "ActsGeometry/ATLASMagneticFieldWrapper.h"


#include <StoreGate/ReadCondHandleKey.h>
#include <StoreGate/ReadHandleKeyArray.h>
#include <StoreGate/ReadHandleKey.h>

class IAthRNGSvc;


namespace ActsTrk {

    /** Extrapolation test for the ActsMuonTrackingGeometry for gen3 */

    class ActsMuonTrackingGeometryTest: public AthHistogramAlgorithm {

      public:
        using AthHistogramAlgorithm::AthHistogramAlgorithm;

        ~ActsMuonTrackingGeometryTest() = default;

        StatusCode initialize() override;
        StatusCode execute() override;       
        StatusCode finalize() override;

      private:

        const MuonGMR4::MuonDetectorManager* m_r4DetMgr{nullptr};

        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "IdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

        ServiceHandle<IAthRNGSvc> m_rndmGenSvc{this, "AthRNGSvc", "AthRNGSvc"};

        PublicToolHandle<ActsTrk::ITrackingGeometryTool> m_trackingGeometryTool{this, "TrackingGeometryTool", "ActsTrackingGeometryTool"};

        SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_detMgrKey{this, "MuonManagerKey",
          "MuonDetectorManager", "MuonManager ReadKey for IOV Range intersection"};
        
        SG::ReadCondHandleKey<AtlasFieldCacheCondObj> m_fieldCacheCondObjInputKey {this, "AtlasFieldCacheCondObj", 
          "fieldCondObj", "Name of the Magnetic Field conditions object key"};

        SG::ReadHandleKey<ActsTrk::GeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};

        SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthParticleKey{this, "TruthKey", "MuonTruthParticles"};

        SG::ReadDecorHandleKey<xAOD::TruthParticleContainer> m_truthSegLinkKey{this, "TruthKeyToSeg", m_truthParticleKey, "truthSegmentLinks", "TruthParticle to TruthSegment link"};

        UnsignedIntegerProperty m_nEvents{this, "nEvents", 100};

        DoubleProperty m_pathLimit{this, "PathLimit", 30.*Gaudi::Units::m};

        DoubleProperty m_maxStepSize{this, "MaxStepSize", 1*Gaudi::Units::m};

        UnsignedIntegerProperty m_maxSteps{this, "MaxSteps", 100000};

        UnsignedIntegerProperty m_maxTargetSkipping{this, "MaxTargetSkipping", 10000}; 

        DoubleProperty m_stepTolerance{this, "StepTolerance", 1e-10};

        Gaudi::Property<bool> m_startFromFirstHit{this, "StartFromFirstHit", false, "Start from first hit"};

        Amg::Transform3D toLocalTrf(const ActsTrk::GeometryContext& gctx, const Identifier& hitId) const;

        Amg::Transform3D toGlobalTrf(const ActsTrk::GeometryContext& gctx, const Identifier& hitId) const;

        IdentifierHash layerHash(const Identifier& id) const;

        Gaudi::Property<std::vector<double>> m_etaRange{this, "EtaRange", {-3, 3}, "The eta range for particles"};
        
        MuonVal::MuonTesterTree m_tree{"MuonNavigationTestGen3R4", "MuonNavigationTestGen3R4"};
        MuonVal::MuonIdentifierBranch m_detId{m_tree, "detId"};
        MuonVal::VectorBranch<unsigned short>& m_techIdx{m_tree.newVector<unsigned short>("detId_techIdx")};
        MuonVal::VectorBranch<unsigned short>& m_gasGapId{m_tree.newVector<unsigned short>("detId_gasGap")};
        MuonVal::ThreeVectorBranch m_truthLoc{m_tree, "truthLoc"};
        MuonVal::ThreeVectorBranch m_truthGlob{m_tree, "truthGlob"};
        MuonVal::ThreeVectorBranch m_truthDir{m_tree, "truthDir"};
        MuonVal::ScalarBranch<float>& m_truthPt{m_tree.newScalar<float>("truthPt")};
        MuonVal::ScalarBranch<float>& m_truthP{m_tree.newScalar<float>("truthP")};
        MuonVal::ThreeVectorBranch m_startGlob{m_tree, "startGlob"};
        MuonVal::VectorBranch<float>& m_actsPropabsMomentum{m_tree.newVector<float>("actsPropabsMomentum")};
        MuonVal::ThreeVectorBranch m_actsPropLoc{m_tree, "actsPropLoc"};
        MuonVal::ThreeVectorBranch m_actsPropGlob{m_tree, "actsPropGlob"};
        MuonVal::ThreeVectorBranch m_actsPropDir{m_tree, "actsPropDir"};
        MuonVal::VectorBranch<float>& m_actsHitWireDist{m_tree.newVector<float>("actsHitWireDist")};
        MuonVal::VectorBranch<float>& m_actsStepSize{m_tree.newVector<float>("actsStepSize")};
        MuonVal::ScalarBranch<float>& m_propLength{m_tree.newScalar<float>("propLength")};
        MuonVal::VectorBranch<unsigned short>& m_isPropagated{m_tree.newVector<unsigned short>("isPropagated")};
        MuonVal::ScalarBranch<unsigned int>& m_propSteps{m_tree.newScalar<unsigned int>("propSteps")};   
        MuonVal::ScalarBranch<unsigned int>& m_event{m_tree.newScalar<unsigned int>("event")};   


    };
}

#endif