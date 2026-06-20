/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTRACKFINDINGTEST_MSTRACKTESTER_H
#define MUONTRACKFINDINGTEST_MSTRACKTESTER_H

#include "AthenaBaseComps/AthHistogramAlgorithm.h"
#include "MuonTesterTree/MuonTesterTreeDict.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadDecorHandleKeyArray.h"


#include "MuonRecToolInterfacesR4/ISegmentSelectionTool.h"
#include "MuonRecToolInterfacesR4/ITrackSummaryTool.h"

#include "ActsEvent/TrackContainer.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODMuonSimHit/MuonSimHitContainer.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODMuon/MuonSegmentContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "MuonPRDTest/SegmentVariables.h"
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"
#include "MuonTrackFindingTools/MsTrackSeeder.h"
#include "MuonPRDTestR4/TrackSummaryModule.h"

#include "MuonPatternEvent/MuonPatternContainer.h"
#include "MuonTrackEvent/MsTrackSeed.h"

namespace MuonValR4{
  class MsTrackTester : public AthHistogramAlgorithm {
      public:
          using AthHistogramAlgorithm::AthHistogramAlgorithm;

          StatusCode initialize() override final;
          StatusCode execute(const EventContext& ctx) override final;
          StatusCode finalize() override final;
      private:
        using Location = MuonR4::MsTrackSeed::Location;
        using SectorProjector = MuonR4::MsTrackSeeder::SectorProjector;
        /** @brief Construct MS track seed from the truth associated segments. A nullopt is returned
         *         if either no segment is matched to the particle or no valid seed could be constructed
        *  @param gctx: Geometry context to project the segments onto the sector centers
        *  @param truthMuon: Reference to the truth muon for which a seed should be constructed */
        std::optional<MuonR4::MsTrackSeed> makeSeedFromTruth(const ActsTrk::GeometryContext& gctx,
                                                             const xAOD::TruthParticle& truthMuon) const;
        /** @brief Calculate the length of the seed and the theta deflection angle
         *         The length is defined as the spread of the seed's segments in the
        *         cylinder coordinate. The deflection angle is calculates as the spread
        *         of the theta angles of the individual segments
        *  @param gctx: Geometry context to retrieve the reference positions
        *  @param seed: The seed with the contributing segments */
        std::pair<double, double>  calcSeedLength(const ActsTrk::GeometryContext& gctx, 
                                                  const MuonR4::MsTrackSeed& seed) const;

        /** @brief Dumps the legacy containers to the TTree */
        StatusCode dumpLegacyTracks(const EventContext& ctx);
        /** @brief Dump truth information */
        StatusCode dumpTruthContent(const EventContext& ctx);
        /** @brief Dump the reconstructed information */
        StatusCode dumpRecoContent(const EventContext& ctx);

        /** @brief  */
        MuonVal::MuonTesterTree m_tree{"MsTrackValidTest", "MuonTrackTester"};
        /** @brief */
        const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};

        Gaudi::Property<bool> m_isMC{this, "isMC", false};


        using TruthHitCol = std::unordered_set<const xAOD::MuonSimHit*>;
        /** @brief Abrivate the ReadHandleKey_t for the segment container */
        using SegmentKey_t = SG::ReadHandleKey<xAOD::MuonSegmentContainer>;
        /** @brief Abrivate the key type for the track particle container */
        using TrackKey_t = SG::ReadHandleKey<xAOD::TrackParticleContainer>;
        /** @brief Abrivate the key type for the muon container */
        using MuonKey_t = SG::ReadHandleKey<xAOD::MuonContainer>;
        /** @brief Segment from the truth hits */
        SegmentKey_t m_truthSegmentKey{this, "TruthSegmentKey", "MuonTruthSegments"};
        /** @brief Primary segment container */
        SegmentKey_t m_recoSegmentKey{this, "SegmentKey", "MuonSegmentsFromR4"};
        /** @brief Legacy segment container */
        SegmentKey_t m_legacySegmentKey{this, "LegacySegmentKey", "MuonSegments"};
        /** @brief Key to the truth particle collection */
        SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthKey{this, "TruthKey", "MuonTruthParticles"};
        /** @brief Decoration dependency to the MS truth track links */
        SG::ReadDecorHandleKeyArray<SG::AuxVectorBase> m_trkTruthLinks{this, "TruthTrackLinks", {}};
        /** @brief Temporary container write handle to push the seeds to store gate for later efficiency analysis */
        SG::ReadHandleKey<MuonR4::MsTrackSeedContainer> m_msTrkSeedKey{this, "MsTrkSeedKey", "MsTrackSeeds"};
        /** @brief Dependency on the geometry alignment */
        ActsTrk::GeoContextReadKey_t m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};
        /** @brief Dependency on the magnetic field */
        SG::ReadCondHandleKey<AtlasFieldCacheCondObj> m_fieldCacheKey{this, "MagFieldKey", "fieldCondObj", "Name of the Magnetic Field conditions object key"};
        /** @brief Segment selection tool to pick the good quality segments */
        ToolHandle<MuonR4::ISegmentSelectionTool> m_segSelector{this, "SegmentSelectionTool" , "" };
        /** @brief Dependency on the R4 muon container */
        MuonKey_t m_muonKey{this, "MuonKey", "MuonsR4"};
        /** @brief Hit summary tool */
        ToolHandle<MuonR4::ITrackSummaryTool> m_summaryTool{this, "SummaryTool" ,""};
        /** @brief Legacy track reconstruction chain */
        TrackKey_t m_legacyTrackKey{this,"LegacyTrackKey", "MuonSpectrometerTrackParticles"};

        /** @brief Legacy muons  */
        MuonKey_t m_legacyMuonKey{this,"LegacyMuonKey", "Muons"};

        std::unique_ptr<MuonR4::MsTrackSeeder> m_seeder{};
        
        using ParticleBranchPtr_t = std::shared_ptr<MuonVal::IParticleFourMomBranch>;
        ParticleBranchPtr_t m_truthTrks{};
        /** @brief Stored muon information from the Acts muon reco chain */
        ParticleBranchPtr_t m_muonTrks{};

        using SegmentBranchPtr_t = std::shared_ptr<MuonPRDTest::SegmentVariables>;
        SegmentBranchPtr_t m_truthSegs{};
        SegmentBranchPtr_t m_recoSegs{};
        SegmentBranchPtr_t m_legacyRecoSegs{};

        /** @brief Simple seed information */
        MuonVal::ThreeVectorBranch m_seedPos{m_tree, "MsTrkSeed_position"};
        /** @brief Is the seed in the encap or in the barrel chambers */
        MuonVal::VectorBranch<char>& m_seedType{m_tree.newVector<char>("MsTrkSeed_type")};
        /** @brief Sector of the seed, even center, odd overlap regions, for details see:  */
        MuonVal::VectorBranch<int>& m_seedSector{m_tree.newVector<int>("MsTrkSeed_sector")};
        /** @brief Maximum separation between the segments on the reference plane */
        MuonVal::VectorBranch<float>& m_seedLength{m_tree.newVector<float>("MsTrkSeed_length")};
        /** @brief Maximum angular difference between the segments part of the seed */
        MuonVal::VectorBranch<float>& m_seedThetaCone{m_tree.newVector<float>("MsTrkSeed_thetaCone")};
        /** @brief Estimated momentum times charge from the track seed */
        MuonVal::VectorBranch<float>& m_seedQP{m_tree.newVector<float>("MsTrkSeed_qTimesP")};
        /** @brief Link to the truth muon */
        MuonVal::VectorBranch<unsigned short>& m_seedTruthLink{m_tree.newVector<unsigned short>("MsTrkSeed_truthLink", -1)};
        /** @brief Hit summary on the track seed */
        std::shared_ptr<TrackSummaryModule> m_seedSummary{};
        /** @brief Link of the track seed to the building segment  */
        MuonVal::MatrixBranch<unsigned short>& m_seedRecoSegMatch{m_tree.newMatrix<unsigned short>("MsTrkSeed_segmentLinks")};
        /** @brief Link of the truth segments to the matchin reco segments */
        MuonVal::MatrixBranch<unsigned short>& m_truthSegToRecoLink{m_tree.newMatrix<unsigned short>("TruthSegments_recoSegLinks",-1)};

        /** @brief Link of the legacy track to the legacy segment */
        MuonVal::VectorBranch<unsigned short>& m_legacySegToTrkLinks{m_tree.newVector<unsigned short>("LegacyRecoSegments_trkLinks", -1)};

        /** @brief Links to all MsTrkSeeds that could be matched to the truthMuon, i.e. >= 1 segment*/
        MuonVal::MatrixBranch<unsigned short>& m_truthMuToSeedIdx{m_tree.newMatrix<unsigned short>("TruthMuons_seedLinks", -1)};
        /** @brief Number of matched segments in the seed */
        MuonVal::MatrixBranch<unsigned short>& m_truthMuToSeedCounter{m_tree.newMatrix<unsigned short>("TruthMuons_seedNSeg")};
        /** @brief Links from the truth muon to the segments  */
        MuonVal::MatrixBranch<unsigned short>& m_truthMuRecoSegLinks{m_tree.newMatrix<unsigned short>("TruthMuons_recoSegLinks")};
 
        /** @brief Output branches of the legacy MS tracks */
        ParticleBranchPtr_t m_legacyTrks{};
    };
}


#endif
