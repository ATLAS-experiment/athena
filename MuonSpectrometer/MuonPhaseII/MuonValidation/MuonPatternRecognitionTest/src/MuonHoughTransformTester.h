/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONVALR4_MuonHoughTransformTester_H
#define MUONVALR4_MuonHoughTransformTester_H

// Framework includes
#include "AthenaBaseComps/AthHistogramAlgorithm.h"

#include "StoreGate/ReadHandleKeyArray.h"
#include "StoreGate/ReadDecorHandleKeyArray.h"

// EDM includes 
#include "xAODMuonSimHit/MuonSimHitContainer.h"
#include "xAODMuon/MuonSegmentContainer.h"

#include <MuonPatternEvent/MuonPatternContainer.h>
#include <MuonReadoutGeometryR4/MuonDetectorManager.h>

// muon includes
#include <MuonRecToolInterfacesR4/IPatternVisualizationTool.h>

#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonTesterTree/ThreeVectorBranch.h"
#include "MuonTesterTree/IdentifierBranch.h"
#include "MuonPRDTestR4/SpacePointTesterModule.h"
#include "MuonPRDTestR4/SimHitTester.h"


///  @brief Lightweight algorithm to read xAOD MDT sim hits and 
///  (fast-digitised) drift circles from SG and fill a 
///  validation NTuple with identifier and drift circle info.


namespace MuonValR4{

  class MuonHoughTransformTester : public AthHistogramAlgorithm {
  public:
    using AthHistogramAlgorithm::AthHistogramAlgorithm;
    virtual ~MuonHoughTransformTester()  = default;

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) override;
    virtual StatusCode finalize() override;
    
    using TruthHitCol = std::unordered_set<const xAOD::MuonSimHit*>;

    struct ObjectMatching{
      /** @brief Associated chamber */
      const MuonGMR4::SpectrometerSector* chamber{nullptr};
      /** @brief Truth segment for reference */
      const xAOD::MuonSegment* truthSegment{nullptr};
      /// @brief All segments matched to this object
      std::vector<const xAOD::MuonSegment*> matchedSegments;
      /// @brief All seeds matched to this object
      std::vector<const MuonR4::SegmentSeed*> matchedSeeds; 
      std::vector<char> matchedSeedFoundSegment;
    };

  private:
    std::vector<ObjectMatching> matchWithTruth(const MuonR4::SegmentSeedContainer& seedContainer,
                                               const xAOD::MuonSegmentContainer& segmentContainer,
                                               const xAOD::MuonSegmentContainer* truthSegments) const;
    /** @brief Calculates how many measurements from the segment fit have the same drift sign
     *          as when evaluated with the truth parameters
     *  @param truthSeg: Reference to the truth segment
     *  @param recoSeg: Reference to the reco segment. */
    unsigned int countOnSameSide(const xAOD::MuonSegment& truthSeg,
                                 const xAOD::MuonSegment& recoSeg) const;
    
   /** @brief Fill the current chamber info into the output
    *  @param chamber: Pointer to the reference sector to which the view belongs to */ 
   void fillChamberInfo(const MuonGMR4::SpectrometerSector* chamber);
    /** @brief Fill the associated truth information into the tree
    *  @param truthSegment: Pointer to the truth parameters in form of a segment
    *  @param gctx: Geometry context for the alignment of the spectrometer sector */
    void fillTruthInfo(const ActsTrk::GeometryContext& gctx,
                       const xAOD::MuonSegment* truthSegment);
    /** @brief Fill the hit summary info of the associated bucket */
    void fillBucketInfo(const MuonR4::SpacePointBucket& bucket);
    
    /** @brief Fill the info associated to the seed 
     *  @param obj: Pointer to the matching object connecting the seeds & segment & truth */
    void fillSeedInfo(const ObjectMatching& obj);
    /** @brief Fill the info assciated to the segment 
      * @param obj: Pointer to the matching object connecting the seeds & segment & truth */
    void fillSegmentInfo(const ObjectMatching& obj);  
                         

    void fillRecoSummary(const xAOD::MuonSegment& recoSegment);
    void fillTruthSummary(const xAOD::MuonSegment& recoSegment);
      
    // // output tree - allows to compare the sim and fast-digitised hits
    MuonVal::MuonTesterTree m_tree{"MuonEtaHoughTest","MuonEtaHoughTransformTest"}; 

    /** @brief Key to the truth segment */
    SG::ReadHandleKey<xAOD::MuonSegmentContainer> m_truthSegmentKey {this, "TruthSegmentKey","MuonTruthSegments", "truth segment container"};
    /** @brief Declare the dependencies on the decorations */
    SG::ReadDecorHandleKeyArray<xAOD::MuonSegmentContainer> m_truthSegLinkKeys{this, "TruthSegLinkKeys", {}};
    /** @brief Name of the decorations for the truth segment */
    Gaudi::Property<std::vector<std::string>> m_truthLinks{this, "TruthSegLinks", {"simHitLinks", "truthParticleLink"}};
    /** @brief Key to the xAOD::MuonSegment container */
    SG::ReadHandleKey<xAOD::MuonSegmentContainer> m_recoSegKey{this, "SegmentKey", "MuonSegmentsFromR4"};
    /** @brief name of the truth link decorations for the reco segment container */
    Gaudi::Property<std::vector<std::string>> m_recoSegLinks{this, "RecoSegLinks", {"truthSegmentLink", "truthParticleLink"}};
    /** @brief List of the two segment seed containers from which the segments are buiit (Complets the pattern finding step) */
    SG::ReadHandleKeyArray<MuonR4::SegmentSeedContainer> m_patternSeedKeys{this, "SegmentSeedKeys", {"MuonHoughStationSegmentSeeds"}};
    /** @brief List of the space point containers in the event legacy + NSW containers */
    SG::ReadHandleKeyArray<MuonR4::SpacePointContainer> m_spKeys{this, "SpacePointKeys", {"MuonSpacePoints"}};
    /** @brief Tracking geometry context */
    ActsTrk::GeoContextReadKey_t m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};

    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
    
    
    Gaudi::Property<bool> m_isMC{this, "isMC", false, "Toggle whether the job is ran on MC or not"};

    Gaudi::Property<bool> m_writeSpacePoints{this, "writeSpacePoints", false,
                                             "Toggle whether the particular space poitns shall be written"};
    
  /// ====== Common block: Filled for all entries =========== 

    /// chamber index field 
    MuonVal::ScalarBranch<int>& m_out_chamberIndex{m_tree.newScalar<int>("chamberIndex")};
    /// +1 for A-, -1 of C-side 
    MuonVal::ScalarBranch<short>& m_out_stationSide{m_tree.newScalar<short>("stationSide")};
    /// phi index of the station
    MuonVal::ScalarBranch<int>& m_out_stationPhi{m_tree.newScalar<int>("stationPhi")};

    MuonVal::ScalarBranch<float>& m_out_bucketStart{m_tree.newScalar<float>("bucket_start", 1)};
    MuonVal::ScalarBranch<float>& m_out_bucketEnd{m_tree.newScalar<float>("bucket_end", -1)};
    MuonVal::ScalarBranch<float>& m_out_bucketEtaHitGap{m_tree.newScalar<float>("bucket_etaHitGap", 0.)};

    /// @brief Branch dumping all the space points from the difference buckets
    std::shared_ptr<SpacePointTesterModule> m_spTester{};
    /// @brief Branch indicating which space points in the tree are associated to the i-th pattern
    MuonVal::MatrixBranch<unsigned char>& m_spMatchedToPattern{m_tree.newMatrix<unsigned char>("seed_matchedSpacPoins")};
    /// @brief Branch indicating which space points in the tree are associated to the i-th segment
    MuonVal::MatrixBranch<unsigned char>& m_spMatchedToSegment{m_tree.newMatrix<unsigned char>("segment_matchedSpacePoints")};

    /// @brief Number of all space points in the bucket
    MuonVal::ScalarBranch<unsigned char>& m_out_nSpacePoints{m_tree.newScalar<unsigned char>("bucket_nHits",0)};
    /// @brief Number of precision hits in the bucket
    MuonVal::ScalarBranch<unsigned char>& m_out_nPrecSpacePoints{m_tree.newScalar<unsigned char>("bucket_nPrecMeas",0)};
    /// @brief Number of phi hits in the bucket
    MuonVal::ScalarBranch<unsigned char>& m_out_nPhiSpacePoints{m_tree.newScalar<unsigned char>("bucket_nPhiMeass",0)};
    
    /// @brief Number of all space points in the bucket
    MuonVal::ScalarBranch<unsigned char>& m_out_nTrueSpacePoints{m_tree.newScalar<unsigned char>("bucket_nTrueMeas",0)};
    /// @brief Number of precision hits in the bucket
    MuonVal::ScalarBranch<unsigned char>& m_out_nTruePrecSpacePoints{m_tree.newScalar<unsigned char>("bucket_nTruePrecMeas",0)};
    /// @brief Number of phi hits in the bucket
    MuonVal::ScalarBranch<unsigned char>& m_out_nTruePhiSpacePoints{m_tree.newScalar<unsigned char>("bucket_nTruePhiMeass",0)};
    
    /// ======= Truth block: Filled if we have a truth match. ============ 

    /// existence of a truth match 
    MuonVal::ScalarBranch<char> & m_out_hasTruth{m_tree.newScalar<char>("gen_exists",false)};
      
    /** @brief global particle properties */
    MuonVal::ScalarBranch<float>& m_out_gen_Eta{m_tree.newScalar<float>("gen_eta",-10.)};
    MuonVal::ScalarBranch<float>& m_out_gen_Phi{m_tree.newScalar<float>("gen_phi",-10.)};
    MuonVal::ScalarBranch<float>& m_out_gen_Pt{m_tree.newScalar<float>("gen_pt",-10.)};    
    MuonVal::ScalarBranch<short>& m_out_gen_Q{m_tree.newScalar<short>("gen_q", 0)};
    /** @brief Truth - segment parameters  */
    MuonVal::ScalarBranch<float>& m_out_gen_y0{m_tree.newScalar<float>("gen_y0", 0.0)}; 
    MuonVal::ScalarBranch<float>& m_out_gen_tanbeta{m_tree.newScalar<float>("gen_tanBeta", 0.0)}; 
    MuonVal::ScalarBranch<float>& m_out_gen_tanalpha{m_tree.newScalar<float>("gen_tanAlpha", 0.0)}; 
    MuonVal::ScalarBranch<float>& m_out_gen_x0{m_tree.newScalar<float>("gen_x0", 0.0)}; 
    MuonVal::ScalarBranch<float>& m_out_gen_time{m_tree.newScalar<float>("gen_time", 0.0)};

    MuonVal::ScalarBranch<int>& m_out_gen_truthOrigin{m_tree.newScalar<int>("gen_origin", -1)};
    MuonVal::ScalarBranch<int>& m_out_gen_truthType{m_tree.newScalar<int>("gen_type", -1)};
    MuonVal::ScalarBranch<float>& m_out_gen_truthBeta{m_tree.newScalar<float>("gen_beta", -1)};
    MuonVal::ScalarBranch<int>& m_out_gen_truthPdgId{m_tree.newScalar<int>("gen_pdgId", 0)};
    
    /** @brief Truth - hit count summary */
    MuonVal::ScalarBranch<unsigned short>& m_out_gen_nPrecHits{m_tree.newScalar<unsigned short>("gen_nPrecHits",0)};
    MuonVal::ScalarBranch<unsigned short>& m_out_gen_nTrigEtaHits{m_tree.newScalar<unsigned short>("gen_nTrigEtaHits",0)};
    MuonVal::ScalarBranch<unsigned short>& m_out_gen_nTrigPhiHits{m_tree.newScalar<unsigned short>("gen_nTrigPhiHits",0)};
    MuonVal::ScalarBranch<unsigned short>& m_out_gen_nMmEtaHits{m_tree.newScalar<unsigned short>("gen_nMmEtaHits",0)};
    MuonVal::ScalarBranch<unsigned short>& m_out_gen_nMmStereoHits{m_tree.newScalar<unsigned short>("gen_nMmStereoHits",0)};
    MuonVal::ScalarBranch<unsigned short>& m_out_gen_nStgcHits{m_tree.newScalar<unsigned short>("gen_nStgcHits",0)};
    
    
    // truth segment size in the y direction
    MuonVal::ScalarBranch<float>& m_out_gen_minYhit{m_tree.newScalar<float>("gen_hitMinY0", 1.0)}; 
    MuonVal::ScalarBranch<float>& m_out_gen_maxYhit{m_tree.newScalar<float>("gen_hitMaxY0", -1.0)};

    /// ========== Seed block: Filled when we have one or multiple seeds ============= 
    /// seed count
    MuonVal::ScalarBranch<unsigned>& m_out_seed_n{m_tree.newScalar<unsigned>("nSeeds", 0)};
    // the following are filled with one entry per seed 

    // does the seed have a phi-extension? 
    MuonVal::VectorBranch<unsigned short>&  m_out_seed_hasPhiExtension{m_tree.newVector<unsigned short>("seed_hasPhiExtension", false)}; 
    // parameters of the seed 
    MuonVal::VectorBranch<float>& m_out_seed_y0{m_tree.newVector<float>("seed_y0", 0.0)}; 
    MuonVal::VectorBranch<float>& m_out_seed_x0{m_tree.newVector<float>("seed_x0", 0.0)}; 
    MuonVal::VectorBranch<float>& m_out_seed_tanbeta{m_tree.newVector<float>("seed_tanBeta", 0.0)}; 
    MuonVal::VectorBranch<float>& m_out_seed_tanalpha{m_tree.newVector<float>("seed_tanAlpha", 0.0)};

      // seed size in the y direction
    MuonVal::VectorBranch<float>& m_out_seed_minYhit{m_tree.newVector<float>("seed_hitMinY0", 1.0)}; 
    MuonVal::VectorBranch<float>& m_out_seed_maxYhit{m_tree.newVector<float>("seed_hitMaxY0", -1.0)}; 
    // hit counts on the seed
    MuonVal::VectorBranch<unsigned short>& m_out_seed_nPrecHits{m_tree.newVector<unsigned short>("seed_nPrecHits", 0)};
    MuonVal::VectorBranch<unsigned short>& m_out_seed_nEtaHits{m_tree.newVector<unsigned short>("seed_nTrigEtaHits", 0)}; 
    MuonVal::VectorBranch<unsigned short>& m_out_seed_nPhiHits{m_tree.newVector<unsigned short>("seed_nTrigPhiHits", 0)};

    MuonVal::VectorBranch<unsigned short>& m_out_seed_nTruePrecHits{m_tree.newVector<unsigned short>("seed_nTruePrecHits", 0)};
    MuonVal::VectorBranch<unsigned short>& m_out_seed_nTrueEtaHits{m_tree.newVector<unsigned short>("seed_nTrueTrigEtaHits", 0)}; 
    MuonVal::VectorBranch<unsigned short>& m_out_seed_nTruePhiHits{m_tree.newVector<unsigned short>("seed_nTrueTrigPhiHits", 0)};
    
    MuonVal::VectorBranch<unsigned short>& m_out_seed_nMmEtaHits{m_tree.newVector<unsigned short>("seed_nMmEtaHits", 0)};
    MuonVal::VectorBranch<unsigned short>& m_out_seed_nMmStereoHits{m_tree.newVector<unsigned short>("seed_nMmStereoHits", 0)};

    MuonVal::VectorBranch<unsigned short>& m_out_seed_nTrueMmEtaHits{m_tree.newVector<unsigned short>("seed_nMmTrueEtaHits", 0)};
    MuonVal::VectorBranch<unsigned short>& m_out_seed_nTrueMmStereoHits{m_tree.newVector<unsigned short>("seed_nMmTrueStereoHits", 0)};
    
    MuonVal::VectorBranch<unsigned short>& m_out_seed_nsTgcStripHits{m_tree.newVector<unsigned short>("seed_nStgcStripHits", 0)};
    MuonVal::VectorBranch<unsigned short>& m_out_seed_nsTgcWireHits{m_tree.newVector<unsigned short>("seed_nStgcWireHits", 0)};
    MuonVal::VectorBranch<unsigned short>& m_out_seed_nsTgcPadHits{m_tree.newVector<unsigned short>("seed_nStgcPadHits", 0)};

    MuonVal::VectorBranch<unsigned short>& m_out_seed_nTruesTgcStripHits{m_tree.newVector<unsigned short>("seed_nStgcTrueStripHits", 0)};
    MuonVal::VectorBranch<unsigned short>& m_out_seed_nTruesTgcWireHits{m_tree.newVector<unsigned short>("seed_nStgcTrueWireHits", 0)};
    MuonVal::VectorBranch<unsigned short>& m_out_seed_nTruesTgcPadHits{m_tree.newVector<unsigned short>("seed_nStgcTruePadHits", 0)};

    MuonVal::VectorBranch<unsigned char> & m_out_seed_ledToSegment{m_tree.newVector<unsigned char>("seed_becameSegment",false)}; 
      
    /// ========== Segment block: Filled when we have one or multiple segments ============= 

    // count of segments 
    MuonVal::ScalarBranch<unsigned>&  m_out_segment_n{m_tree.newScalar<unsigned>("nSegments", 0)}; 
    // the following are filled with one entry per segment

    // fit metrics 
    MuonVal::VectorBranch<float>& m_out_segment_chi2{m_tree.newVector<float>("segment_chi2", -1.)};
    MuonVal::VectorBranch<uint16_t>& m_out_segment_nDoF{m_tree.newVector<uint16_t>("segment_nDoF", 0)};
    MuonVal::VectorBranch<char>&  m_out_segment_hasTimeFit {m_tree.newVector<char>("segment_hasTimeFit", false)}; 
    MuonVal::VectorBranch<uint16_t>&  m_out_segment_fitIter {m_tree.newVector<uint16_t>("segment_nIter", 0)};

    // segment parameters
    MuonVal::VectorBranch<float>& m_out_segment_y0{m_tree.newVector<float>("segment_y0", 0.)}; 
    MuonVal::VectorBranch<float>& m_out_segment_x0{m_tree.newVector<float>("segment_x0", 0.)}; 
    MuonVal::VectorBranch<float>& m_out_segment_theta{m_tree.newVector<float>("segment_theta", 0.)}; 
    MuonVal::VectorBranch<float>& m_out_segment_phi{m_tree.newVector<float>("segment_phi", 0.)};
    MuonVal::VectorBranch<float>& m_out_segment_time{m_tree.newVector<float>("segment_t0", 0.)};
    // segment covariance    
    using FloatVecBrPtr_t = std::shared_ptr<MuonVal::VectorBranch<float>>; 
    std::array<FloatVecBrPtr_t, Acts::sumUpToN(Acts::toUnderlying(MuonR4::SegmentFit::ParamDefs::nPars))> m_segmentCov{};
    
    /** Hit counts on segment */
    MuonVal::VectorBranch<unsigned short>& m_out_segment_nPrecHits{m_tree.newVector<unsigned short>("segment_nPrecHits", 0)};
    MuonVal::VectorBranch<unsigned short>& m_out_segment_nTrigEtaHits{m_tree.newVector<unsigned short>("segment_nTrigEtaHits", 0)};
    MuonVal::VectorBranch<unsigned short>& m_out_segment_nTrigPhiHits{m_tree.newVector<unsigned short>("segment_nTrigPhiHits", 0)};
   
    MuonVal::VectorBranch<unsigned short>& m_out_segment_nPrecOutliers{m_tree.newVector<unsigned short>("segment_nPrecOutliers", 0)};
    MuonVal::VectorBranch<unsigned short>& m_out_segment_nTrigEtaOutliers{m_tree.newVector<unsigned short>("segment_nTrigEtaOutliers", 0)};
    MuonVal::VectorBranch<unsigned short>& m_out_segment_nTrigPhiOutliers{m_tree.newVector<unsigned short>("segment_nTrigPhiOutliers", 0)};

    MuonVal::VectorBranch<unsigned short>& m_out_segment_nPrecHoles{m_tree.newVector<unsigned short>("segment_nPrecHoles", 0)};
    MuonVal::VectorBranch<unsigned short>& m_out_segment_nTrigEtaHoles{m_tree.newVector<unsigned short>("segment_nTrigEtaHoles", 0)};
    MuonVal::VectorBranch<unsigned short>& m_out_segment_nTrigPhiHoles{m_tree.newVector<unsigned short>("segment_nTrigPhiHoles", 0)};
    /** True matched hit counters */
    MuonVal::VectorBranch<unsigned short>& m_out_segment_nTruePrecHits{m_tree.newVector<unsigned short>("segment_nTruePrecHits", 0)};
    MuonVal::VectorBranch<unsigned short>& m_out_segment_nTrueTrigEtaHits{m_tree.newVector<unsigned short>("segment_nTrueTrigEtaHits", 0)};
    MuonVal::VectorBranch<unsigned short>& m_out_segment_nTrueTrigPhiHits{m_tree.newVector<unsigned short>("segment_nTrueTrigPhiHits", 0)};
   
    MuonVal::VectorBranch<unsigned short>& m_out_segment_nTruePrecOutliers{m_tree.newVector<unsigned short>("segment_nTruePrecOutliers", 0)};
    MuonVal::VectorBranch<unsigned short>& m_out_segment_nTrueTrigEtaOutliers{m_tree.newVector<unsigned short>("segment_nTrueTrigEtaOutliers", 0)};
    MuonVal::VectorBranch<unsigned short>& m_out_segment_nTrueTrigPhiOutliers{m_tree.newVector<unsigned short>("segment_nTrueTrigPhiOutliers", 0)};
    
    /** NSW hit counters */
    MuonVal::VectorBranch<unsigned short>& m_out_segment_nMmEtaHits{m_tree.newVector<unsigned short>("segment_nMmEtaHits", 0)};
    MuonVal::VectorBranch<unsigned short>& m_out_segment_nMmStereoHits{m_tree.newVector<unsigned short>("segment_nMmStereoHits", 0)};
    MuonVal::VectorBranch<unsigned short>& m_out_segment_nSTgcStripHits{m_tree.newVector<unsigned short>("segment_nStgcStripHits", 0)};
    MuonVal::VectorBranch<unsigned short>& m_out_segment_nSTgcWireHits{m_tree.newVector<unsigned short>("segment_nStgcWireHits", 0)};
    MuonVal::VectorBranch<unsigned short>& m_out_segment_nSTgcPadHits{m_tree.newVector<unsigned short>("segment_nStgcPadHits", 0)};
  
    MuonVal::VectorBranch<unsigned short>& m_out_segment_nMmTrueEtaHits{m_tree.newVector<unsigned short>("segment_nMmTrueEtaHits", 0)};
    MuonVal::VectorBranch<unsigned short>& m_out_segment_nMmTrueStereoHits{m_tree.newVector<unsigned short>("segment_nMmTrueStereoHits", 0)};
    MuonVal::VectorBranch<unsigned short>& m_out_segment_nSTgcTrueStripHits{m_tree.newVector<unsigned short>("segment_nStgcTrueStripHits", 0)};
    MuonVal::VectorBranch<unsigned short>& m_out_segment_nSTgcTrueWireHits{m_tree.newVector<unsigned short>("segment_nStgcTrueWireHits", 0)};
    MuonVal::VectorBranch<unsigned short>& m_out_segment_nSTgcTruePadHits{m_tree.newVector<unsigned short>("segment_nStgcTruePadHits", 0)};
  
    MuonVal::VectorBranch<unsigned short>& m_out_segment_nMmEtaOutliers{m_tree.newVector<unsigned short>("segment_nMmEtaOutliers", 0)};
    MuonVal::VectorBranch<unsigned short>& m_out_segment_nMmStereoOutliers{m_tree.newVector<unsigned short>("segment_nMmStereoOutliers", 0)};
    MuonVal::VectorBranch<unsigned short>& m_out_segment_nSTgcStripOutliers{m_tree.newVector<unsigned short>("segment_nStgcStripOutliers", 0)};
    MuonVal::VectorBranch<unsigned short>& m_out_segment_nSTgcWireOutliers{m_tree.newVector<unsigned short>("segment_nStgcWireOutliers", 0)};
    MuonVal::VectorBranch<unsigned short>& m_out_segment_nSTgcPadOutliers{m_tree.newVector<unsigned short>("segment_nStgcPadOutliers", 0)};
  
    MuonVal::VectorBranch<unsigned short>& m_out_segment_nMmTrueEtaOutliers{m_tree.newVector<unsigned short>("segment_nMmTrueEtaOutliers", 0)};
    MuonVal::VectorBranch<unsigned short>& m_out_segment_nMmTrueStereoOutliers{m_tree.newVector<unsigned short>("segment_nMmTrueStereoOutliers", 0)};
    MuonVal::VectorBranch<unsigned short>& m_out_segment_nSTgcTrueStripOutliers{m_tree.newVector<unsigned short>("segment_nStgcTrueStripOutliers", 0)};
    MuonVal::VectorBranch<unsigned short>& m_out_segment_nSTgcTrueWireOutliers{m_tree.newVector<unsigned short>("segment_nStgcTrueWireOutliers", 0)};
    MuonVal::VectorBranch<unsigned short>& m_out_segment_nSTgcTruePadOutliers{m_tree.newVector<unsigned short>("segment_nStgcTruePadOutliers", 0)};

    // segment size in the y direction
    MuonVal::VectorBranch<float>& m_out_segment_minYhit{m_tree.newVector<float>("segment_hitMinY0", 1.0)}; 
    MuonVal::VectorBranch<float>& m_out_segment_maxYhit{m_tree.newVector<float>("segment_hitMinY0", -1.0)}; 
    MuonVal::VectorBranch<float>& m_out_segment_minTrueYhit{m_tree.newVector<float>("segment_trueHitMinY0", 1.0)}; 
    MuonVal::VectorBranch<float>& m_out_segment_maxTrueYhit{m_tree.newVector<float>("segment_trueHitMaxY0", -1.0)}; 

    /// Pattern visualization tool
    ToolHandle<MuonValR4::IPatternVisualizationTool> m_visionTool{this, "VisualizationTool", ""};
 
    const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};

  };
}

#endif // MUONFASTDIGITEST_MUONVALR4_MuonHoughTransformTester_H
