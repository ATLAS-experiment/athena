/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONVALR4_MuonRecoChainTester_H
#define MUONVALR4_MuonRecoChainTester_H

// Framework includes
#include "AthenaBaseComps/AthHistogramAlgorithm.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "MuonSpacePoint/SpacePointContainer.h"
#include "MuonTesterTree/MuonTesterTreeDict.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadDecorHandleKeyArray.h"

namespace MuonValR4{

  class MuonRecoChainTester : public AthHistogramAlgorithm {
    	public:
            using AthHistogramAlgorithm::AthHistogramAlgorithm;
            virtual ~MuonRecoChainTester() = default;

            virtual StatusCode initialize() override;
            virtual StatusCode execute() override;
            virtual StatusCode finalize() override;

  private:
        using StIdx = Muon::MuonStationIndex::StIndex;
        /** @brief Counts how many buckets are in a particular station
         *  @param spContainer: Space point collection
         *  @param station: Station index to consider
         *  @param outBranch: Output branch to save the information to  */
        void fillBucketsPerStation(const MuonR4::SpacePointContainer& spContainer,
                                   const StIdx station,
                                   MuonVal::ScalarBranch<uint16_t>& outBranch) const;

        // output tree - allows to compare the sim and fast-digitised hits
        MuonVal::MuonTesterTree m_tree{"MuonRecoObjTest", "MuonEtaHoughTransformTest"};

        /** @brief  number of buckets in the entire MS */
        MuonVal::ScalarBranch<uint16_t>& m_nBucket{m_tree.newScalar<uint16_t>("nBucket")};
        /** @brief number of buckets in the BI */
        MuonVal::ScalarBranch<uint16_t>& m_nBucketBI{m_tree.newScalar<uint16_t>("nBucketBI")};
        /** @brief number of buckets in the BM */
        MuonVal::ScalarBranch<uint16_t>& m_nBucketBM{m_tree.newScalar<uint16_t>("nBucketBM")};
        /** @brief number of buckets in the BO */
        MuonVal::ScalarBranch<uint16_t>& m_nBucketBO{m_tree.newScalar<uint16_t>("nBucketBO")};
        /** @brief number of buckets in the BE */
        MuonVal::ScalarBranch<uint16_t>& m_nBucketBE{m_tree.newScalar<uint16_t>("nBucketBE")};
        /** @brief number of buckets in the BI */
        MuonVal::ScalarBranch<uint16_t>& m_nBucketEI{m_tree.newScalar<uint16_t>("nBucketEI")};
        /** @brief number of buckets in the BM */
        MuonVal::ScalarBranch<uint16_t>& m_nBucketEM{m_tree.newScalar<uint16_t>("nBucketEM")};
        /** @brief number of buckets in the BO */
        MuonVal::ScalarBranch<uint16_t>& m_nBucketEO{m_tree.newScalar<uint16_t>("nBucketEO")};
        /** @brief number of buckets in the EE */
        MuonVal::ScalarBranch<uint16_t>& m_nBucketEE{m_tree.newScalar<uint16_t>("nBucketEE")};

        Gaudi::Property<bool> m_isMC{this, "isMC", false};

        /** @brief Keys to the segment collections */
        
        /** @brief Segment made from the full legacy chain */
        Gaudi::Property<std::string> m_legacySegmentKey{this, "LegacySegmentKey", "LegacyChainSegments"};
        /** @brief Segments seeded from the R4 pattern but made with the legacy segment maker */
        Gaudi::Property<std::string> m_r4PatternSegmentKey{this, "SegmentFromR4HoughKey", "MuonSegmentsFromHoughR4"};
        /** @brief Segments made from the R4 segment maker */
        Gaudi::Property<std::string> m_segmentKeyR4{this, "R4SegmentKey", "MuonSegmentsFromR4"};
        /** @brief Segment from the truth hits */
        Gaudi::Property<std::string> m_truthSegmentKey{this, "TruthSegmentKey", "TruthSegmentsR4"};
        /** @brief Key to the track collections */
        SG::ReadHandleKey<xAOD::TrackParticleContainer> m_legacyTrackKey{this,"LegacyTrackKey", "MuonSpectrometerTrackParticles"};
        SG::ReadHandleKey<xAOD::TrackParticleContainer> m_TrackKeyHoughR4{this, "TrackKeyHoughR4", "MuonSpectrometerTrackParticlesFromHoughR4"};
        SG::ReadHandleKey<xAOD::TrackParticleContainer> m_TrackKeyR4{this, "TrackKeyR4", "MuonSpectrometerTrackParticlesR4"};

        /** @brief Key to the truth particle collection */
        SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthKey{this, "TruthKey", "MuonTruthParticles"};
        /** @brief Decoration dependency to the MS truth track links */
        SG::ReadDecorHandleKeyArray<xAOD::TrackParticleContainer> m_trkTruthLinks{this, "TruthTrackLinks", {}};
        /** @brief Key to the space point container */
        SG::ReadHandleKey<MuonR4::SpacePointContainer> m_spacePointKey{this, "SpacePointContainer", "MuonSpacePoints"};
  
        std::shared_ptr<MuonVal::IParticleFourMomBranch> m_legacyTrks{};
        std::shared_ptr<MuonVal::IParticleFourMomBranch> m_TrksHoughR4{};
        std::shared_ptr<MuonVal::IParticleFourMomBranch> m_TrksSegmentR4{};
        
        std::shared_ptr<MuonVal::IParticleFourMomBranch> m_truthTrks{};
  
  };
}

#endif 