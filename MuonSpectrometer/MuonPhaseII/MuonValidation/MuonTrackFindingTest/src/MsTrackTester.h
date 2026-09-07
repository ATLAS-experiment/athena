/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTRACKFINDINGTEST_MSTRACKTESTER_H
#define MUONTRACKFINDINGTEST_MSTRACKTESTER_H

#include "AthenaBaseComps/AthHistogramAlgorithm.h"
#include "MuonTesterTree/MuonTesterTreeDict.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ReadDecorHandleKeyArray.h"

#include "MuonRecToolInterfacesR4/ITrackSummaryTool.h"
#include "MuonRecToolInterfacesR4/ISegmentSelectionTool.h"
#include "MuonRecToolInterfacesR4/ITrackSeedingDiagnosticsTool.h"
#include "MagFieldConditions/AtlasFieldCacheCondObj.h"

#include "ActsEvent/TrackContainer.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODMuonSimHit/MuonSimHitContainer.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODMuon/MuonSegmentContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "MuonPRDTest/SegmentVariables.h"
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"
#include "MuonPRDTestR4/TrackSummaryModule.h"

#include "MuonPatternEvent/MuonPatternContainer.h"
#include "MuonTrackEvent/MsTrackSeed.h"
#include "MuonTrackEvent/MuonTag.h"

#include "ActsEvent/ContextUtility.h"
#include "ActsGeometryInterfaces/IExtrapolationTool.h"


namespace MuonValR4{
  class MsTrackTester : public AthHistogramAlgorithm {
      public:
          using AthHistogramAlgorithm::AthHistogramAlgorithm;

          StatusCode initialize() override final;
          StatusCode execute(const EventContext& ctx) override final;
          StatusCode finalize() override final;
      private:
        using Location = MuonR4::MsTrackSeed::Location;
        using SectorProjector = MuonR4::ExpandedSector::SectorProjector;
        /** @brief Construct MS track seed from the truth associated segments. A nullopt is returned
          *        if either no segment is matched to the particle or no valid seed could be constructed
          *  @param gctx: The geometry context to align the segment within ATLAS
          *  @param truthMuon: Reference to the truth muon for which a seed should be constructed */
        std::optional<MuonR4::MsTrackSeed> makeSeedFromTruth(const Acts::GeometryContext& tgContext,
                                                             const xAOD::TruthParticle& truthMuon) const;
        /** @brief Calculate the length of the seed and the theta deflection angle
         *         The length is defined as the spread of the seed's segments in the
         *         cylinder coordinate. The deflection angle is calculates as the spread
         *         of the theta angles of the individual segments
         *  @param gctx: The geometry context to align the segment within ATLAS
         *  @param seed: The seed with the contributing segments */
        std::pair<double, double>  calcSeedLength(const Acts::GeometryContext& tgContext,
                                                  const MuonR4::MsTrackSeed& seed) const;

        
        /** @brief Returns the matched truth particle that is dumped in the tree. This method
         *         only works downstream the dumpTruthContent call
         *  @param part: Reference to the particle of interest */
        const xAOD::TruthParticle* truthTreeParticle(const xAOD::IParticle& part) const;
        /** @brief Returns the collection of reconstructed segments that can be associated to the id track
         *         If the track is connected with a truth muon, the list of reconstructed segments matched
         *         to the truth muon is returned. Otherwise, the list of segments associated with the
         *         MuTagIMO tag is returned.
         *  @param idTrack: The inner detector track of interest
         *  @param ctx: The current event context to access the segment tag collection from store gate */
        std::vector<const xAOD::MuonSegment*> getAssociatedSegments(const xAOD::TrackParticle& idTrack,
                                                                    const EventContext& ctx) const;

        /** @brief Searches the Id track in the basline collection of all ID tracks selected for the
         *         combined muon reconstructiion (STACO/MuidCo/MuTagIMO/Calo) 
          * @param idTrack: The inner detector track of interest
          * @param ctx: The current event context to access the segment tag collection from store gate */
        const MuonR4::MuonTag* findBaseIdTag(const xAOD::TrackParticle& idTrack,
                                            const EventContext& ctx) const;

        const MuonR4::MuonTag* findMuTagIMO(const xAOD::TrackParticle& idTrack,
                                            const EventContext& ctx) const;


        const MuonGMR4::SpectrometerSector* getEnvelope(const xAOD::MuonSegment& segment) const;

                                            
        /** @brief Searched the MuTagIMO tag  */
        /** @brief Associated sement tag variables */
        struct SegmentTagVariables{
            /** @brief  Indices of the matched segments in the tree*/
            std::vector<std::uint8_t> recoSegs{};
            /** @brief chi2 scores of the matching procedure */
            std::vector<float> matchScores{};
            /** @brief Difference in theta between ID and segment parameters */
            std::vector<float> deltaTheta{};
            /** @brief Difference in phi between ID and segment parameters */
            std::vector<float> deltaPhi{};
            /** @brief Difference in y0 between ID and segment parameters */
            std::vector<float> deltaY0{};
            /** @brief Difference in x0 between ID and segment parameters */
            std::vector<float> deltaX0{};
            /** @brief Is the extrapolation to the surface good */
            std::vector<std::uint8_t> goodExtp{};
            /** @brief Flags indicating whether the segment made it onto MuTagIMO */
            std::vector<std::uint8_t> taggedSeg{};

        };
      
        SegmentTagVariables calcSegTagVariables(const xAOD::TrackParticle& idTrack,
                                              const EventContext& ctx); 

        /** @brief Dumps the legacy containers to the TTree */
        StatusCode dumpLegacyTracks(const EventContext& ctx);
        /** @brief Dump truth information */
        StatusCode dumpTruthContent(const EventContext& ctx);
        /** @brief Dump the reconstructed information */
        StatusCode dumpRecoContent(const EventContext& ctx);

        /** @brief The output tree object  */
        MuonVal::MuonTesterTree m_tree{"MsTrackValidTest", "MuonTrackTester"};
        /** @brief The detector manager to fetch the segment surfaces */
        const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};
        /** @brief Toggle whether the job runs on MC or not */
        Gaudi::Property<bool> m_isMC{this, "isMC", false};
        /** @brief Toggle whether to process ID tracks or not */
        Gaudi::Property<bool> m_storeID{this, "storeIdTrks", true};

        using TruthHitCol = std::unordered_set<const xAOD::MuonSimHit*>;
        /** @brief Abrivate the ReadHandleKey_t for the segment container */
        using SegmentKey_t = SG::ReadHandleKey<xAOD::MuonSegmentContainer>;
        /** @brief Abrivate the key type for the track particle container */
        using TrackKey_t = SG::ReadHandleKey<xAOD::TrackParticleContainer>;
        /** @brief Abrivate the key type for the muon container */
        using MuonKey_t = SG::ReadHandleKey<xAOD::MuonContainer>;
        /** @brief Abrivate the muon tag container */
        using MuonTagKey_t = SG::ReadHandleKey<MuonR4::MuonTagContainer>;
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

        /** @brief Context provider for geometry, magnetic field and calibration contexts */
        ActsTrk::ContextUtility m_ctxProvider{this};
        /** @brief Dependency on the R4 muon container */
        MuonKey_t m_muonKey{this, "MuonKey", "MuonsR4"};
        /** @brief Hit summary tool */
        ToolHandle<MuonR4::ITrackSummaryTool> m_summaryTool{this, "SummaryTool" ,""};
        /** @brief The track seeding tool to construct the seed candidates and to estimate the initial parameters */
        ToolHandle<MuonR4::ITrackSeedingDiagnosticsTool> m_seedingTool{this, "SeedingTool", ""};
        /** @brief Selection tool to quantify the segment candidate quality */
        ToolHandle<MuonR4::ISegmentSelectionTool> m_segSelector{this, "SegmentSelectionTool" , "" };
        /** @brief Track extrapolation tool */
        ToolHandle<ActsTrk::IExtrapolationTool> m_extrapolationTool{this, "ExtrapolationTool" ,"" };
        /** @brief Legacy track reconstruction chain */
        TrackKey_t m_legacyTrackKey{this,"LegacyTrackKey", "MuonSpectrometerTrackParticles"};
        /** @brief  The collection of ID tracks associated with the truth particle*/
        TrackKey_t m_idTrackKey{this, "IdTrackKey", "InDetTrackParticles"};
        /** @brief The collection of all ID tracks that are selected for combined reconstruction */
        MuonTagKey_t m_idTagKey{this, "IdTagKey", "MuonInDetCandidates"};
        /** @brief The collection of the original segment tags */
        MuonTagKey_t m_segTagKey{this, "SegTagKey", "SegmentTags"};

        /** @brief Extra tolerance applied on the non-bending intercept when calculating
         *         the matching score */
        Gaudi::Property<double> m_toleranceX0{this, "toleranceX0", 20.*Gaudi::Units::cm};
        /** @brief Extra tolerance applied on the bending intercept when calculating
         *         the matching score */
        Gaudi::Property<double> m_toleranceY0{this, "toleranceY0", 5.*Gaudi::Units::cm};
        /** @brief Extra tolerance applied on the bending direction when calculating the
         *         matching score */
        Gaudi::Property<double> m_toleranceTheta{this, "toleranceTheta", 1.*Gaudi::Units::deg};
        /** @brief Extra tolerance applied on the bending direction when calculating the
         *         matching score */
        Gaudi::Property<double> m_tolerancePhi{this, "tolerancePhi", 2.*Gaudi::Units::deg};

        /** @brief Legacy muons  */
        MuonKey_t m_legacyMuonKey{this,"LegacyMuonKey", "Muons"};

        /** @brief Instance to the Acts logger */
        std::unique_ptr<const Acts::Logger> m_logger{};
        /** @brief Return the reference to the Acts logger */
        const Acts::Logger& logger() const { return *m_logger; }

        
        using ParticleBranchPtr_t = std::shared_ptr<MuonVal::IParticleFourMomBranch>;
        ParticleBranchPtr_t m_truthTrks{};
        /** @brief Stored muon information from the Acts muon reco chain */
        ParticleBranchPtr_t m_muonTrks{};
        /** @brief Stored ID track information */
        ParticleBranchPtr_t m_idTracks{};

        /** @brief Abrivation of the branches containing sement information */
        using SegmentBranchPtr_t = std::shared_ptr<MuonPRDTest::SegmentVariables>;
        SegmentBranchPtr_t m_truthSegs{};
        SegmentBranchPtr_t m_recoSegs{};
        SegmentBranchPtr_t m_legacyRecoSegs{};
        /** @brief The number of ID tracks in the event */
        MuonVal::ScalarBranch<std::uint16_t>& m_nIdTracks{m_tree.newScalar<std::uint16_t>("nIdTracks", 0)};
        /** @brief The number of selected ID tracks in the event */
        MuonVal::ScalarBranch<std::uint16_t>& m_nIdTags{m_tree.newScalar<std::uint16_t>("nIdTags", 0)};
        /** @brief Simple seed information */
        MuonVal::ThreeVectorBranch m_seedPos{m_tree, "TrkSeed_position"};
        /** @brief Seed direction vector */
        MuonVal::UnitThreeVectorBranch m_seedDir{m_tree, "TrkSeed_direction"};
        /** @brief Is the seed in the encap or in the barrel chambers */
        MuonVal::VectorBranch<char>& m_seedType{m_tree.newVector<char>("TrkSeed_type")};
        /** @brief Sector of the seed, even center, odd overlap regions, for details see:  */
        MuonVal::VectorBranch<int>& m_seedSector{m_tree.newVector<int>("TrkSeed_sector")};
        /** @brief Maximum separation between the segments on the reference plane */
        MuonVal::VectorBranch<float>& m_seedLength{m_tree.newVector<float>("TrkSeed_length")};
        /** @brief Maximum angular difference between the segments part of the seed */
        MuonVal::VectorBranch<float>& m_seedThetaCone{m_tree.newVector<float>("TrkSeed_thetaCone")};
        /** @brief Does the seeding tool construct valid parameters from the seed */
        MuonVal::VectorBranch<char>& m_seedGood{m_tree.newVector<char>("TrkSeed_goodSeed")};
        /** @brief Estimated momentum times charge from the track seed */
        MuonVal::VectorBranch<float>& m_seedQP{m_tree.newVector<float>("TrkSeed_qTimesP")};
        /** @brief Link to the truth muon */
        MuonVal::VectorBranch<unsigned short>& m_seedTruthLink{m_tree.newVector<unsigned short>("TrkSeed_truthLink", -1)};
        /** @brief Hit summary on the track seed */
        std::shared_ptr<TrackSummaryModule> m_seedSummary{};
        /** @brief Link of the track seed to the building segment  */
        MuonVal::MatrixBranch<unsigned short>& m_seedRecoSegMatch{m_tree.newMatrix<unsigned short>("TrkSeed_segmentLinks")};
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
