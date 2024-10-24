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

#include "Acts/MagneticField/MagneticFieldContext.hpp"
#include "Acts/Navigation/DetectorNavigator.hpp"

namespace ActsTrk {
    class MuonDetectorNavTest: public AthHistogramAlgorithm {
        public:
        MuonDetectorNavTest(const std::string& name, ISvcLocator* pSvcLocator);
        
        ~MuonDetectorNavTest() = default;
        
        StatusCode execute() override;
        StatusCode initialize() override;
		StatusCode finalize() override;

        private:

        Amg::Transform3D toGlobalTrf(const ActsGeometryContext& gctx, const Identifier& hitId) const;

        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "IdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

        SG::ReadCondHandleKey<AtlasFieldCacheCondObj> m_fieldCacheCondObjInputKey {this, "AtlasFieldCacheCondObj", "fieldCondObj", "Name of the Magnetic Field conditions object key"};

        SG::ReadHandleKey<ActsGeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};

        SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthParticleKey{this, "TruthKey", "TruthParticles", "key"};

        const MuonGMR4::MuonDetectorManager* m_r4DetMgr{nullptr};

        SG::ReadHandleKeyArray<xAOD::MuonSimHitContainer> m_inSimHitKeys {this, "SimHitKeys",{ "xMdtSimHits","xRpcSimHits","xTgcSimHits","xStgcSimHits","xMmSimHits"}, "xAOD  SimHit collections"};

        ServiceHandle<ActsTrk::IDetectorVolumeSvc> m_detVolSvc{this, "DetectorVolumeSvc", "DetectorVolumeSvc"};

        MuonVal::MuonTesterTree m_tree{"MuonNavigationTestR4", "MuonNavigationTestR4"};
		MuonVal::ScalarBranch<float>& m_distanceBetweenHits{m_tree.newScalar<float>("distanceBetweenHits")};
		MuonVal::ScalarBranch<std::string>& m_propagatedIdentifier{m_tree.newScalar<std::string>("propagatedIdentifier")};
		MuonVal::ScalarBranch<std::string>& m_truthIdentifier{m_tree.newScalar<std::string>("truthIdentifier")};

    };
}
#endif