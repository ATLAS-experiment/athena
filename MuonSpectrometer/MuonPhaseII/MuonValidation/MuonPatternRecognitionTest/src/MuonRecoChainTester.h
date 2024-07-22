/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONVALR4_MuonRecoChainTester_H
#define MUONVALR4_MuonRecoChainTester_H

// Framework includes
#include "AthenaBaseComps/AthHistogramAlgorithm.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "MuonTesterTree/MuonTesterTreeDict.h"

#include "StoreGate/ReadHandleKey.h"

namespace MuonValR4{

  class MuonRecoChainTester : public AthHistogramAlgorithm {
    	public:
            MuonRecoChainTester(const std::string& name, ISvcLocator* pSvcLocator);
            virtual ~MuonRecoChainTester()  = default;

            virtual StatusCode initialize() override;
            virtual StatusCode execute() override;
            virtual StatusCode finalize() override;

  private:
        // // output tree - allows to compare the sim and fast-digitised hits
        MuonVal::MuonTesterTree m_tree{"MuonRecoObjTest","MuonEtaHoughTransformTest"}; 

        Gaudi::Property<bool> m_isMC{this, "isMC", false};

        /** @brief Keys to the segment collections */
        Gaudi::Property<std::string> m_legacySegmentKey{this, "LegacySegmentKey", "MuonSegments"};
        Gaudi::Property<std::string> m_r4PatternSegmentKey{this, "R4SegmentKey", "MuonSegmentsFromR4"};

        /** @brief Key to the track collections */
        SG::ReadHandleKey<xAOD::TrackParticleContainer> m_legacyTrackKey{this,"LegacyTrackKey", "MuonSpectrometerTrackParticles"};
        SG::ReadHandleKey<xAOD::TrackParticleContainer> m_r4TrackKey{this, "R4TrackKey", "MuonSpectrometerTrackParticlesR4"};
        /** @brief Key to the truth particle collection */
        SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthKey{this, "TruthKey", "TruthParticles"};
  
        std::shared_ptr<MuonVal::IParticleFourMomBranch> m_legacyTrks{};
        std::shared_ptr<MuonVal::IParticleFourMomBranch> m_r4Trks{};
        std::shared_ptr<MuonVal::IParticleFourMomBranch> m_truthTrks{};
  
  };
}

#endif 