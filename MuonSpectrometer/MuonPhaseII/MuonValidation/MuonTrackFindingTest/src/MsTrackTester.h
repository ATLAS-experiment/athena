/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTRACKFINDINGTEST_MSTRACKTESTER_H
#define MUONTRACKFINDINGTEST_MSTRACKTESTER_H

#include "AthenaBaseComps/AthHistogramAlgorithm.h"
#include "MuonTesterTree/MuonTesterTreeDict.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadDecorHandleKeyArray.h"


#include "MuonRecToolInterfacesR4/ISegmentSelectionTool.h"

#include "xAODTruth/TruthParticleContainer.h"
#include "xAODMuonSimHit/MuonSimHitContainer.h"
#include "xAODMuon/MuonSegmentContainer.h"
#include "MuonPRDTest/SegmentVariables.h"

#include "MuonPatternEvent/MuonPatternContainer.h"
#include "MuonTrackEvent/MsTrackSeed.h"

namespace MuonValR4{
  class MsTrackTester : public AthHistogramAlgorithm {
      public:
          using AthHistogramAlgorithm::AthHistogramAlgorithm;

          StatusCode initialize() override final;
          StatusCode execute() override final;
          StatusCode finalize() override final;
      private:
        

        using Location = MuonR4::MsTrackSeed::Location;

        double expressAtRefPlane(const xAOD::MuonSegment& segment,
                                 const Location plane) const;       
        MuonVal::MuonTesterTree m_tree{"MsTrackValidTest", "MuonTrackTester"};

        Gaudi::Property<bool> m_isMC{this, "isMC", false};


        using TruthHitCol = std::unordered_set<const xAOD::MuonSimHit*>;

        using SegmentKey_t = SG::ReadHandleKey<xAOD::MuonSegmentContainer>;
        /** @brief Segment from the truth hits */
        SegmentKey_t m_truthSegmentKey{this, "TruthSegmentKey", "TruthSegmentsR4"};
        /** @brief Primary segment container */
        SegmentKey_t m_recoSegmentKey{this, "SegmentKey", "MuonSegmentsFromR4"};
        /** @brief Key to the truth particle collection */
        SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthKey{this, "TruthKey", "MuonTruthParticles"};
        /** @brief Decoration dependency to the MS truth track links */
        SG::ReadDecorHandleKeyArray<SG::AuxVectorBase> m_trkTruthLinks{this, "TruthTrackLinks", {}};
        /** @brief Temporary container write handle to push the seeds to store gate for later efficiency analysis */
        SG::ReadHandleKey<MuonR4::MsTrackSeedContainer> m_msTrkSeedKey{this, "MsTrkSeedKey", "MsTrackSeeds"};

        using ParticleBranchPtr_t = std::shared_ptr<MuonVal::IParticleFourMomBranch>;
        ParticleBranchPtr_t m_truthTrks{};

        using SegmentBranchPtr_t = std::shared_ptr<MuonPRDTest::SegmentVariables>;
        SegmentBranchPtr_t m_truthSegs{};
        SegmentBranchPtr_t m_recoSegs{};

        /** @brief Simple seed information */
        MuonVal::ThreeVectorBranch m_seedPos{m_tree, "MsTrkSeed_position"};
        /** @brief Is the seed in the encap or in the barrel chambers */
        MuonVal::VectorBranch<char>& m_seedType{m_tree.newVector<char>("MstTrkSeed_type")};
        /** @brief Maximum separation between the segments on the reference plane */
        MuonVal::VectorBranch<float>& m_seedLength{m_tree.newVector<float>("MsTrkSeed_length")};
        /** @brief Maximum angular difference between the segments part of the seed */
        MuonVal::VectorBranch<float>& m_seedThetaCone{m_tree.newVector<float>("MsTrkSeed_thetaCone")};

        /** @brief Link of the track seed to the building segment  */
        MuonVal::MatrixBranch<unsigned short>& m_seedRecoSegMatch{m_tree.newMatrix<unsigned short>("MsTrkSeed_segmentLinks")};
        /** @brief Link of the truth segments to the matchin reco segments */
        MuonVal::MatrixBranch<unsigned short>& m_truthSegToRecoLink{m_tree.newMatrix<unsigned short>("TruthSegments_recoSegLinks",-1)};
        /** @brief Links to all MsTrkSeeds that could be matched to the truthMuon, i.e. >= 1 segment*/
        MuonVal::MatrixBranch<unsigned short>& m_truthMuToSeedIdx{m_tree.newMatrix<unsigned short>("TruthMuons_seedLinks", -1)};
        /** @brief Corresponding matching counter of reconstructed segments */
        MuonVal::MatrixBranch<unsigned short>& m_truthMuToSeedCounter{m_tree.newMatrix<unsigned short>("TruthMuons_seedNSeg")};
        /** @brief Links from the truth muon to the segments  */
        MuonVal::MatrixBranch<unsigned short>& m_truthMuTruthSegLinks{m_tree.newMatrix<unsigned short>("TruthMuons_truthSegLinks")};
        /** @brief Links from the truth muon to the segments  */
        MuonVal::MatrixBranch<unsigned short>& m_truthMuRecoSegLinks{m_tree.newMatrix<unsigned short>("TruthMuons_recoSegLinks")};

        /** @brief Segment selection tool to pick the good quality segments */
        ToolHandle<MuonR4::ISegmentSelectionTool> m_segSelector{this, "SegmentSelectionTool" , "" };
        /** @brief Radius of the barrel reference cylinder onto which all segments are projected */
        double m_refBarrelR{7.*Gaudi::Units::m};
        /** @brief Position along the beam axis of the referece disc onto which all endcap segments are projected */
        double m_refEndcapDiscZ{15.*Gaudi::Units::m};
        /** @brief Radius of the reference disc. */
        double m_refEndcapDiscR{12.*Gaudi::Units::m};

    };
}


#endif