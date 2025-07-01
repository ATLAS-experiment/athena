/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONCSVDUMP_HitDumperAlg_H
#define MUONCSVDUMP_HitDumperAlg_H


#include "AthenaBaseComps/AthHistogramAlgorithm.h"

#include <MuonIdHelpers/IMuonIdHelperSvc.h>
#include "StoreGate/ReadHandleKeyArray.h"
#include "StoreGate/ReadDecorHandleKeyArray.h"

#include "MuonPatternEvent/MuonPatternContainer.h"
#include "MuonTesterTree/MuonTesterTreeDict.h"
#include "MuonPRDTestR4/SpacePointTesterModule.h"
#include "xAODMuon/MuonSegmentContainer.h"

namespace MuonR4{
    class MlHitDumperAlg : public AthHistogramAlgorithm {
        public:
            using AthHistogramAlgorithm::AthHistogramAlgorithm;
            virtual StatusCode initialize() override final;
            virtual StatusCode execute() override final;
            virtual StatusCode finalize() override final;
        private:

            SG::ReadHandleKeyArray<SpacePointContainer> m_spacePointKeys{this, "SpacePointKeys", {"MuonSpacePoints"}, 
                                                     "Key to the space point container"};

            SG::ReadHandleKey<xAOD::MuonSegmentContainer> m_truthSegKey{this, "TruthSegmentKey", "TruthSegmentsR4"};
            
            SG::ReadDecorHandleKey<xAOD::MuonSegmentContainer> m_truthLinkKey{this, "TruthLinkKey", m_truthSegKey, "truthParticleLink"};
            SG::ReadHandleKey<ActsGeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};

            MuonVal::MuonTesterTree m_tree{"MuonHitDump","MuonHitDump"};

            std::shared_ptr<MuonVal::IParticleFourMomBranch> m_muonP4{};

            std::shared_ptr<MuonValR4::SpacePointTesterModule> m_spCollection{};
            MuonVal::ThreeVectorBranch m_spGlobPos{m_tree, "spacePoint_globPos"};
            MuonVal::VectorBranch<char>& m_truthLink{m_tree.newVector<char>("spacePoint_truthLink")};

    };
}

#endif