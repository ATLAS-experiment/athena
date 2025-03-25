/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONDETECTORNAVTEST_MUONDETECTORNAVTEST_H
#define MUONDETECTORNAVTEST_MUONDETECTORNAVTEST_H

#include "xAODTruth/TruthParticleContainer.h"
#include "xAODMuonSimHit/MuonSimHitContainer.h"

#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>

#include <ActsGeometryInterfaces/ActsGeometryContext.h>
#include "ActsGeometry/ATLASMagneticFieldWrapper.h"
#include <ActsGeometryInterfaces/IDetectorVolumeSvc.h>
#include <StoreGate/ReadCondHandleKey.h>
#include <StoreGate/ReadHandleKeyArray.h>
#include <StoreGate/ReadHandleKey.h>


#include <MuonReadoutGeometryR4/MuonDetectorManager.h>
#include "MuonTesterTree/MuonTesterTree.h"
#include "MuonTesterTree/ThreeVectorBranch.h"
#include "MuonTesterTree/IdentifierBranch.h"

#include "MuonReadoutGeometry/MuonDetectorManager.h"

#include "Acts/MagneticField/MagneticFieldContext.hpp"
#include "Acts/Navigation/DetectorNavigator.hpp"
#include "TrkExInterfaces/IExtrapolator.h"

namespace ActsTrk {
    class MuonDetectorNavTest: public AthHistogramAlgorithm {
        public:
        using AthHistogramAlgorithm::AthHistogramAlgorithm;
        
        ~MuonDetectorNavTest() = default;
        
        StatusCode execute() override;
        StatusCode initialize() override;
        StatusCode finalize() override;

        private:

        Amg::Transform3D toGlobalTrf(const ActsGeometryContext& gctx, const Identifier& hitId) const;

        Amg::Transform3D toLocalTrf(const ActsGeometryContext& gctx, const Identifier& hitId) const;

        IdentifierHash layerHash(const Identifier& id) const;

        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "IdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

        SG::ReadCondHandleKey<AtlasFieldCacheCondObj> m_fieldCacheCondObjInputKey {this, "AtlasFieldCacheCondObj", "fieldCondObj", "Name of the Magnetic Field conditions object key"};

        SG::ReadHandleKey<ActsGeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};

        SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthParticleKey{this, "TruthKey", "MuonTruthParticles"};

        SG::ReadDecorHandleKey<xAOD::TruthParticleContainer> m_truthSegLinkKey{this, "TruthKeyToSeg", m_truthParticleKey, "truthSegLinks", "TruthParticle to TruthSegment link"};

        const MuonGMR4::MuonDetectorManager* m_r4DetMgr{nullptr};

        BooleanProperty m_drawEvent{this, "DrawEvent", false};

        DoubleProperty m_pathLimit{this, "PathLimit", 23.*Gaudi::Units::m};

        DoubleProperty m_maxStepSize{this, "MaxStepSize", 1.*Gaudi::Units::m};

        UnsignedIntegerProperty m_maxSteps{this, "MaxSteps", 100000};

        DoubleProperty m_stepTolerance{this, "StepTolerance", 1e-10};

        UnsignedIntegerProperty m_maxTargetSkipping{this, "MaxTargetSkipping", 10000};            

        ServiceHandle<ActsTrk::IDetectorVolumeSvc> m_detVolSvc{this, "DetectorVolumeSvc", "DetectorVolumeSvc"};

        ToolHandle<Trk::IExtrapolator> m_extrapolator{this, "Extrapolator",
          "Trk::Extrapolator/AtlasExtrapolator" "Tool for ATLAS Extrapolator"};  

        SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_detMgrKey{this, "MuonManagerKey", 
          "MuonDetectorManager", "MuonManager ReadKey for IOV Range intersection"};
    
        Gaudi::Property<bool> m_startFromFirstHit{this, "StartFromFirstHit", false, "Start from first hit"};

        MuonVal::MuonTesterTree m_tree{"MuonNavigationTestR4", "MuonNavigationTestR4"};
        MuonVal::MuonIdentifierBranch m_detId{m_tree, "detId"};
        MuonVal::VectorBranch<unsigned short>& m_techIdx{m_tree.newVector<unsigned short>("detId_techIdx")};
        MuonVal::VectorBranch<unsigned short>& m_gasGapId{m_tree.newVector<unsigned short>("detId_gasGap")};
        MuonVal::VectorBranch<float>& m_actsPropMomentum{m_tree.newVector<float>("actsPropMomentum")};
        MuonVal::VectorBranch<float>& m_atlasPropMomentum{m_tree.newVector<float>("atlasPropMomentum")};
        MuonVal::ThreeVectorBranch m_startGlob{m_tree, "startGlob"};
        MuonVal::ThreeVectorBranch m_truthLoc{m_tree, "truthHitLoc"};
        MuonVal::ThreeVectorBranch m_truthDir{m_tree, "truthHitDir"};
        MuonVal::ThreeVectorBranch m_truthGlob{m_tree, "truthHitGlob"};
        MuonVal::ThreeVectorBranch m_actsPropLoc{m_tree, "actsPropLoc"};
        MuonVal::ThreeVectorBranch m_atlasPropLoc{m_tree, "atlasPropLoc"};
        MuonVal::ThreeVectorBranch m_actsPropGlob{m_tree, "actsPropGlob"};
        MuonVal::ThreeVectorBranch m_atlasPropGlob{m_tree, "atlasPropGlob"};
        MuonVal::ThreeVectorBranch m_actsPropDir{m_tree, "actsPropDir"};
        MuonVal::ThreeVectorBranch m_atlasPropDir{m_tree, "atlasPropDir"};
        MuonVal::VectorBranch<unsigned short>& m_isPropagated{m_tree.newVector<unsigned short>("isPropagated")};
        MuonVal::ScalarBranch<unsigned int>& m_propSteps{m_tree.newScalar<unsigned int>("propSteps")};
        MuonVal::ScalarBranch<float>& m_propLength{m_tree.newScalar<float>("propLength")};
        MuonVal::VectorBranch<float>& m_actsStepSize{m_tree.newVector<float>("stepSize")};
        MuonVal::ScalarBranch<float>& m_matchedTruthFraction{m_tree.newScalar<float>("matchedTruthFraction")};
        MuonVal::ScalarBranch<float>& m_matchedPropFraction{m_tree.newScalar<float>("matchedPropFraction")};
        MuonVal::ScalarBranch<float>& m_truthPt{m_tree.newScalar<float>("truthPt")};
        MuonVal::ScalarBranch<float>& m_truthP{m_tree.newScalar<float>("truthP")};
        MuonVal::ScalarBranch<unsigned int>& m_event{m_tree.newScalar<unsigned int>("event")};

    };
}
#endif
