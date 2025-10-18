/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONVALR4_MuonHoughTransformTester_H
#define MUONVALR4_MuonHoughTransformTester_H

// Framework includes
#include "AthenaBaseComps/AthHistogramAlgorithm.h"

#include "StoreGate/ReadHandleKeyArray.h"

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
    virtual StatusCode execute() override;
    virtual StatusCode finalize() override;
    
    using TruthHitCol = std::unordered_set<const xAOD::MuonSimHit*>;

    struct ObjectMatching{
      /** @brief Associated chamber */
      const MuonGMR4::SpectrometerSector* chamber{nullptr};
      /** @brief Truth segment for reference */
      const xAOD::MuonSegment* truthSegment{nullptr};
      /// @brief All segments matched to this object
      std::vector<const MuonR4::Segment*> matchedSegments;
      /// @brief All seeds matched to this object
      std::vector<const MuonR4::SegmentSeed*> matchedSeeds; 
      std::vector<char> matchedSeedFoundSegment{0};
    };

  private:
    std::vector<ObjectMatching> matchWithTruth(const ActsGeometryContext& gctx,
                                               const xAOD::MuonSegmentContainer* truthSegments,
                                               const MuonR4::SegmentSeedContainer* seedContainer,
                                               const MuonR4::SegmentContainer* segmentContainer) const;
    /** @brief Calculates how many measurements from the segment fit have the same drift sign
     *          as when evaluated with the truth parameters
     *  @param gctx: Geometry context to fetch the alignment constants
     *  @param truthSeg: Reference to the truth segment
     *  @param recoSeg: Reference to the reco segment. */
    unsigned int countOnSameSide(const ActsGeometryContext& gctx,
                                 const xAOD::MuonSegment& truthSeg,
                                 const MuonR4::Segment& recoSeg) const;
    
   /** @brief Fill the current chamber info into the output
    *  @param chamber: Pointer to the reference sector to which the view belongs to */ 
   void fillChamberInfo(const MuonGMR4::SpectrometerSector* chamber);
    /** @brief Fill the associated truth information into the tree
    *  @param truthSegment: Pointer to the truth parameters in form of a segment
    *  @param gctx: Geometry context for the alignment of the spectrometer sector */
    void fillTruthInfo(const ActsGeometryContext& gctx,
                       const xAOD::MuonSegment* truthSegment);
    /** @brief Fill the hit summary info of the associated bucket */
    void fillBucketInfo(const MuonR4::SpacePointBucket& bucket);
    
    /** @brief Fill the info associated to the seed 
     *  @param obj: Pointer to the matching object connecting the seeds & segment & truth */
    void fillSeedInfo(const ObjectMatching& obj);
    /** @brief Fill the info assciated to the segment 
      * @param obj: Pointer to the matching object connecting the seeds & segment & truth */
    void fillSegmentInfo(const ActsGeometryContext& gctx, const ObjectMatching& obj);  
                         

      
    // // output tree - allows to compare the sim and fast-digitised hits
    MuonVal::MuonTesterTree m_tree{"MuonEtaHoughTest","MuonEtaHoughTransformTest"}; 

    // MDT sim hits in xAOD format 
    SG::ReadHandleKey<xAOD::MuonSegmentContainer> m_truthSegmentKey {this, "TruthSegmentKey","MuonTruthSegments", "truth segment container"};
                                                          
    SG::ReadHandleKeyArray<MuonR4::SegmentSeedContainer> m_inHoughSegmentSeedKeys{this, "SegmentSeedKeys", {"MuonHoughStationSegmentSeeds"}};
    SG::ReadHandleKeyArray<MuonR4::SegmentContainer> m_inSegmentKeys{this, "SegmentKeys", {"R4MuonSegments"}};


    SG::ReadHandleKey<MuonR4::SpacePointContainer> m_spKey{this, "SpacePointKey", "MuonSpacePoints"};
    SG::ReadHandleKey<ActsGeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};

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

    MuonVal::ScalarBranch<float>& m_out_bucketStart{m_tree.newScalar<float>("bucketStart", 1)};
    MuonVal::ScalarBranch<float>& m_out_bucketEnd{m_tree.newScalar<float>("bucketEnd", -1)};

    /// @brief Branch dumping all the space points from the difference buckets
    std::shared_ptr<SpacePointTesterModule> m_spTester{};
    /// @brief Branch indicating which space points in the tree are associated to the i-th pattern
    MuonVal::MatrixBranch<unsigned char>& m_spMatchedToPattern{m_tree.newMatrix<unsigned char>("seedMatchedSp")};
    /// @brief Branch indicating which space points in the tree are associated to the i-th segment
    MuonVal::MatrixBranch<unsigned char>& m_spMatchedToSegment{m_tree.newMatrix<unsigned char>("segmentMatchedSp")};

    /// @brief Number of all space points in the bucket
    MuonVal::ScalarBranch<unsigned char>& m_out_nSpacePoints{m_tree.newScalar<unsigned char>("bucketNHits",0)};
    /// @brief Number of precision hits in the bucket
    MuonVal::ScalarBranch<unsigned char>& m_out_nPrecSpacePoints{m_tree.newScalar<unsigned char>("bucketNPrecMeas",0)};
    /// @brief Number of phi hits in the bucket
    MuonVal::ScalarBranch<unsigned char>& m_out_nPhiSpacePoints{m_tree.newScalar<unsigned char>("bucketNPhiMeass",0)};
    
    /// @brief Number of all space points in the bucket
    MuonVal::ScalarBranch<unsigned char>& m_out_nTrueSpacePoints{m_tree.newScalar<unsigned char>("bucketNTrueMeas",0)};
    /// @brief Number of precision hits in the bucket
    MuonVal::ScalarBranch<unsigned char>& m_out_nTruePrecSpacePoints{m_tree.newScalar<unsigned char>("bucketNTruePrecMeas",0)};
    /// @brief Number of phi hits in the bucket
    MuonVal::ScalarBranch<unsigned char>& m_out_nTruePhiSpacePoints{m_tree.newScalar<unsigned char>("bucketNTruePhiMeass",0)};
    
    /// ======= Truth block: Filled if we have a truth match. ============ 

    /// existence of a truth match 
    MuonVal::ScalarBranch<bool> & m_out_hasTruth{m_tree.newScalar<bool>("hasTruth",false)};
      
    /** @brief global particle properties */
    MuonVal::ScalarBranch<float>& m_out_gen_Eta{m_tree.newScalar<float>("genEta",-10.)};
    MuonVal::ScalarBranch<float>& m_out_gen_Phi{m_tree.newScalar<float>("genPhi",-10.)};
    MuonVal::ScalarBranch<float>& m_out_gen_Pt{m_tree.newScalar<float>("genPt",-10.)};    
    MuonVal::ScalarBranch<short>& m_out_gen_Q{m_tree.newScalar<short>("genQ", 0)};
    /** @brief Truth - segment parameters  */
    MuonVal::ScalarBranch<float>& m_out_gen_y0{m_tree.newScalar<float>("genY0", 0.0)}; 
    MuonVal::ScalarBranch<float>& m_out_gen_tantheta{m_tree.newScalar<float>("genTanTheta", 0.0)}; 
    MuonVal::ScalarBranch<float>& m_out_gen_tanphi{m_tree.newScalar<float>("genTanPhi", 0.0)}; 
    MuonVal::ScalarBranch<float>& m_out_gen_x0{m_tree.newScalar<float>("genX0", 0.0)}; 
    MuonVal::ScalarBranch<float>& m_out_gen_time{m_tree.newScalar<float>("genTime", 0.0)}; 
    /** @brief Truth - hit count summary */
    MuonVal::ScalarBranch<unsigned short>& m_out_gen_nHits{m_tree.newScalar<unsigned short>("genNHits",0)};
    MuonVal::ScalarBranch<unsigned short>& m_out_gen_nRPCHits{m_tree.newScalar<unsigned short>("genNRpcHits",0)};
    MuonVal::ScalarBranch<unsigned short>& m_out_gen_nMDTHits{m_tree.newScalar<unsigned short>("genNMdtHits",0)};
    MuonVal::ScalarBranch<unsigned short>& m_out_gen_nTGCHits{m_tree.newScalar<unsigned short>("genNTgcHits",0)};
    MuonVal::ScalarBranch<unsigned short>& m_out_gen_nNswHits{m_tree.newScalar<unsigned short>("genNNswHits",0)};
    // truth segment size in the y direction
    MuonVal::ScalarBranch<float>& m_out_gen_minYhit{m_tree.newScalar<float>("genMinYhit", 1.0)}; 
    MuonVal::ScalarBranch<float>& m_out_gen_maxYhit{m_tree.newScalar<float>("genMaxYhit", -1.0)};

    /// ========== Seed block: Filled when we have one or multiple seeds ============= 
    /// seed count
    MuonVal::ScalarBranch<unsigned>& m_out_seed_n{m_tree.newScalar<unsigned>("nSeeds", 0)};
    // the following are filled with one entry per seed 

    // does the seed have a phi-extension? 
    MuonVal::VectorBranch<unsigned short>&  m_out_seed_hasPhiExtension{m_tree.newVector<unsigned short>("seedHasPhiExtension", false)}; 
    // fraction of the hits on the seed matched to truth 
    MuonVal::VectorBranch<unsigned short>& m_out_seed_nMatchedHits{m_tree.newVector<unsigned short>("seedTruthMatchedHits", false)}; 

    // parameters of the seed 
    MuonVal::VectorBranch<float>& m_out_seed_y0{m_tree.newVector<float>("seedY0", 0.0)}; 
    MuonVal::VectorBranch<float>& m_out_seed_x0{m_tree.newVector<float>("seedX0", 0.0)}; 
    MuonVal::VectorBranch<float>& m_out_seed_tantheta{m_tree.newVector<float>("seedTanTheta", 0.0)}; 
    MuonVal::VectorBranch<float>& m_out_seed_tanphi{m_tree.newVector<float>("seedTanPhi", 0.0)};

      // seed size in the y direction
    MuonVal::VectorBranch<float>& m_out_seed_minYhit{m_tree.newVector<float>("seedMinYhit", 1.0)}; 
    MuonVal::VectorBranch<float>& m_out_seed_maxYhit{m_tree.newVector<float>("seedMaxYhit", -1.0)}; 
    // hit counts on the seed
    MuonVal::VectorBranch<unsigned short>& m_out_seed_nHits{m_tree.newVector<unsigned short>("seedNHits", 0)}; 
    MuonVal::VectorBranch<unsigned short>& m_out_seed_nPrecHits{m_tree.newVector<unsigned short>("seedNPrecHits", 0)};
    MuonVal::VectorBranch<unsigned short>& m_out_seed_nEtaHits{m_tree.newVector<unsigned short>("seedNEtaHits", 0)}; 
    MuonVal::VectorBranch<unsigned short>& m_out_seed_nPhiHits{m_tree.newVector<unsigned short>("seedNPhiHits", 0)};

    MuonVal::VectorBranch<unsigned short>& m_out_seed_nMdt{m_tree.newVector<unsigned short>("seedNMdtHits", 0)}; 
    MuonVal::VectorBranch<unsigned short>& m_out_seed_nRpc{m_tree.newVector<unsigned short>("seedNRpcHits", 0)}; 
    MuonVal::VectorBranch<unsigned short>& m_out_seed_nTgc{m_tree.newVector<unsigned short>("seedNTgcHits", 0)}; 
    MuonVal::VectorBranch<unsigned short>& m_out_seed_nsTgc{m_tree.newVector<unsigned short>("seedNsTgcHits", 0)};
    MuonVal::VectorBranch<unsigned short>& m_out_seed_nMm{m_tree.newVector<unsigned short>("seedNMmHits", 0)};

    /// @brief Labelled hits from the pattern visualization tool
    MuonVal::VectorBranch<unsigned short>& m_out_seed_nTrueHits{m_tree.newVector<unsigned short>("seedNTrueHits", 0)};
    MuonVal::VectorBranch<unsigned short>& m_out_seed_nTruePrecHits{m_tree.newVector<unsigned short>("seedNTruePrecHits", 0)};
    MuonVal::VectorBranch<unsigned short>& m_out_seed_nTruePhiHits{m_tree.newVector<unsigned short>("seedNTruePhiHits", 0)};
    MuonVal::VectorBranch<unsigned short>& m_out_seed_nTrueEtaHits{m_tree.newVector<unsigned short>("seedNTrueEtaHits", 0)};

    MuonVal::VectorBranch<unsigned char> & m_out_seed_ledToSegment{m_tree.newVector<unsigned char>("seedConvertedToSegment",false)}; 
      
    /// ========== Segment block: Filled when we have one or multiple segments ============= 

    // count of segments 
    MuonVal::ScalarBranch<unsigned>&  m_out_segment_n{m_tree.newScalar<unsigned>("nSegments", 0)}; 
    // the following are filled with one entry per segment

    // fit metrics 
    MuonVal::VectorBranch<float>& m_out_segment_chi2{m_tree.newVector<float>("segmentChi2", -1.)};
    MuonVal::VectorBranch<uint16_t>& m_out_segment_nDoF{m_tree.newVector<uint16_t>("segmentNdoF", 0)};
    MuonVal::VectorBranch<bool>&  m_out_segment_hasTimeFit {m_tree.newVector<bool>("segmentHasTimeFit", false)}; 
    MuonVal::VectorBranch<uint16_t>&  m_out_segment_fitIter {m_tree.newVector<uint16_t>("segmentFitIterations", 0)};

    // presence of a phi extension
    MuonVal::VectorBranch<bool>&  m_out_segment_hasPhi {m_tree.newVector<bool>("segmentHasPhiHits", false)}; 
      
    // segment parameters
    MuonVal::VectorBranch<float>& m_out_segment_y0{m_tree.newVector<float>("segmentY0", 0.0)}; 
    MuonVal::VectorBranch<float>& m_out_segment_x0{m_tree.newVector<float>("segmentX0", 0.0)}; 
    MuonVal::VectorBranch<float>& m_out_segment_tantheta{m_tree.newVector<float>("segmentTanTheta", 0.0)}; 
    MuonVal::VectorBranch<float>& m_out_segment_tanphi{m_tree.newVector<float>("segmentTanPhi", 0.0)};
    MuonVal::VectorBranch<float>& m_out_segment_time{m_tree.newVector<float>("segmentTime", 0.)};
      
    // segment uncertainties
    MuonVal::VectorBranch<float>& m_out_segment_err_y0{m_tree.newVector<float>("segmentErrY0", -1.0)}; 
    MuonVal::VectorBranch<float>& m_out_segment_err_x0{m_tree.newVector<float>("segmentErrX0", -1.0)}; 
    MuonVal::VectorBranch<float>& m_out_segment_err_tantheta{m_tree.newVector<float>("segmentErrTanTheta", -1.0)}; 
    MuonVal::VectorBranch<float>& m_out_segment_err_tanphi{m_tree.newVector<float>("segmentErrTanPhi", -1.0)}; 
    MuonVal::VectorBranch<float>& m_out_segment_err_time{m_tree.newVector<float>("segmentErrTime", -1.0)};

    // hit counts on segment 
    MuonVal::VectorBranch<unsigned short>& m_out_segment_truthMatchedHits{m_tree.newVector<unsigned short>("segmentTruthMatchedHits", 0)};
    MuonVal::VectorBranch<unsigned short>& m_out_segment_nMdtHits{m_tree.newVector<unsigned short>("segmentNMdtHits", 0)};
    MuonVal::VectorBranch<unsigned short>& m_out_segment_nRpcEtaHits{m_tree.newVector<unsigned short>("segmentNRpcEtaHits", 0)};
    MuonVal::VectorBranch<unsigned short>& m_out_segment_nRpcPhiHits{m_tree.newVector<unsigned short>("segmentNRpcPhiHits", 0)};
    MuonVal::VectorBranch<unsigned short>& m_out_segment_nTgcEtaHits{m_tree.newVector<unsigned short>("segmentNTgcEtaHits", 0)};
    MuonVal::VectorBranch<unsigned short>& m_out_segment_nTgcPhiHits{m_tree.newVector<unsigned short>("segmentNTgcPhiHits", 0)};
    // hit coutns on segment NSW
    MuonVal::VectorBranch<unsigned short>& m_out_segment_nMmEtaHits{m_tree.newVector<unsigned short>("segmentNMmEtaHits", 0)};
    MuonVal::VectorBranch<unsigned short>& m_out_segment_nMmStereoHits{m_tree.newVector<unsigned short>("segmentNMmStereoHits", 0)};
    MuonVal::VectorBranch<unsigned short>& m_out_segment_nsTgcStripHits{m_tree.newVector<unsigned short>("segmentNsTgcStripHits", 0)};
    MuonVal::VectorBranch<unsigned short>& m_out_segment_nsTgcWireHits{m_tree.newVector<unsigned short>("segmentNsTgcWireHits", 0)};
    MuonVal::VectorBranch<unsigned short>& m_out_segment_nsTgcPadpHits{m_tree.newVector<unsigned short>("segmentNsTgcPadHits", 0)};
   /// @brief Labelled hits from the pattern visualization tool
    MuonVal::VectorBranch<unsigned short>& m_out_segment_nTrueHits{m_tree.newVector<unsigned short>("segmentNTrueHits", 0)};
    MuonVal::VectorBranch<unsigned short>& m_out_segment_nTruePrecHits{m_tree.newVector<unsigned short>("segmentNTruePrecHits", 0)};
    MuonVal::VectorBranch<unsigned short>& m_out_segment_nTruePhiHits{m_tree.newVector<unsigned short>("segmentNTruePhiHits", 0)};
    MuonVal::VectorBranch<unsigned short>& m_out_segment_nTrueEtaHits{m_tree.newVector<unsigned short>("segmentNTrueEtaHits", 0)};
    
    // segment size in the y direction
    MuonVal::VectorBranch<float>& m_out_segment_minYhit{m_tree.newVector<float>("segmentMinYhit", 1.0)}; 
    MuonVal::VectorBranch<float>& m_out_segment_maxYhit{m_tree.newVector<float>("segmentMaxYhit", -1.0)}; 

    /// Pattern visualization tool
    ToolHandle<MuonValR4::IPatternVisualizationTool> m_visionTool{this, "VisualizationTool", ""};
 
    const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};

  };
}

#endif // MUONFASTDIGITEST_MUONVALR4_MuonHoughTransformTester_H
