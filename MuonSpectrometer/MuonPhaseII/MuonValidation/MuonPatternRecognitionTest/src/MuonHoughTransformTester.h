/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONVALR4_MuonHoughTransformTester_H
#define MUONVALR4_MuonHoughTransformTester_H

// Framework includes
#include "AthenaBaseComps/AthHistogramAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/ReadHandleKeyArray.h"
#include "StoreGate/ReadCondHandleKey.h"

// EDM includes 
#include "xAODMuonSimHit/MuonSimHitContainer.h"
#include "xAODMuon/MuonSegmentContainer.h"

#include <MuonPatternEvent/MuonPatternContainer.h>
#include <MuonReadoutGeometryR4/MuonDetectorManager.h>

// muon includes
#include <MuonRecToolInterfacesR4/IPatternVisualizationTool.h>

#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonTesterTree/MuonTesterTree.h"
#include "MuonTesterTree/ThreeVectorBranch.h"
#include "MuonTesterTree/IdentifierBranch.h"
#include "MuonPRDTestR4/SpacePointTesterModule.h"
#include "MuonPRDTestR4/SimHitTester.h"

#include "TCanvas.h"
#include "TEllipse.h"
#include "TBox.h"
#include "TLatex.h"
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
  private:
    
 
    /// Helper method to fetch data from StoreGate. If the key is empty, a nullptr is assigned to the container ptr
    /// Failure is returned in cases, of non-empty keys and failed retrieval
    template <class ContainerType> StatusCode retrieveContainer(const EventContext& ctx,
                                                                const SG::ReadHandleKey<ContainerType>& key,
                                                                const ContainerType* & contToPush) const;


    struct ObjectMatching{
        /** @brief Associated chamber */
        const MuonGMR4::SpectrometerSector* chamber{nullptr};
        /** @brief Truth segment for reference */
        const xAOD::MuonSegment* truthSegment{nullptr};
        /** @brief Best associated matching seed */
        const MuonR4::SegmentSeed* matchedSeed{nullptr};
        /** @brief Best segment matched to the truth */
        const MuonR4::Segment* matchedSegment{nullptr};
        /** @brief Flag toggling whether that's the best possible match in terms of hits */
        bool bestTruthMatch{false};
        /** @brief Segment is the best ambiguity solution */
        bool bestAmbiMatch{false};
        /** @brief Number of segments matched to the truth segments */
        unsigned nRecoSegFromTruth{0};
        /** @brief Number of truth matched hits on the segment */
        unsigned nMatchedSegHits{0};
        /** @brief Number of truth matched hit on the maximum */
        unsigned nMatchedMaxHits{0};
   };

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
    
    void fillChamberInfo(const MuonGMR4::SpectrometerSector* chamber);

    void fillTruthInfo(const xAOD::MuonSegment* truthSegment);
    
    void fillSeedInfo(const ObjectMatching& obj);            
    void fillSegmentInfo(const ActsGeometryContext& gctx, const ObjectMatching& obj);  
    
    // MDT sim hits in xAOD format 
    SG::ReadHandleKey<xAOD::MuonSegmentContainer> m_truthSegmentKey {this, "TruthSegmentKey","TruthSegmentsR4", "truth segment container"};
                                                          
    SG::ReadHandleKey<MuonR4::SegmentSeedContainer> m_inHoughSegmentSeedKey{this, "SegmentSeedKey", "MuonHoughStationSegmentSeeds"};
    SG::ReadHandleKey<MuonR4::SegmentContainer> m_inSegmentKey{this, "SegmentKey", "R4MuonSegments"};
    SG::ReadHandleKey<MuonR4::SpacePointContainer> m_spacePointKey{this, "SpacePointKey", "MuonSpacePoints"};
    
    SG::ReadHandleKey<ActsGeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};

    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
   
    /// Pattern visualization tool
    ToolHandle<MuonValR4::IPatternVisualizationTool> m_visionTool{this, "VisualizationTool", ""};
 
    const MuonGMR4::MuonDetectorManager* m_r4DetMgr{nullptr};

    // // output tree - allows to compare the sim and fast-digitised hits
    MuonVal::MuonTesterTree m_tree{"MuonEtaHoughTest","MuonEtaHoughTransformTest"}; 
    
    MuonVal::ScalarBranch<int>& m_out_stationName{m_tree.newScalar<int>("stationName")};
    MuonVal::ScalarBranch<int>& m_out_stationEta{m_tree.newScalar<int>("stationEta")};
    MuonVal::ScalarBranch<int>& m_out_stationPhi{m_tree.newScalar<int>("stationPhi")};

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

    /** @brief Is the current segment / maximum / truth combo the best on in terms of hits*/
    MuonVal::ScalarBranch<bool> & m_out_gen_bestMatch{m_tree.newScalar<bool>("bestMatch", true)};
    /** @brief Is the current segment / truth combo also the one that solves the left right ambiguity  */
    MuonVal::ScalarBranch<bool> & m_out_gen_bestAmbiMatch{m_tree.newScalar<bool>("bestAmbiMatch", false)};
    /** @brief How many reconstructed segments are associated to the same truth segment */
    MuonVal::ScalarBranch<unsigned>& m_out_gen_nRecoFromTruth{m_tree.newScalar<unsigned>("genNumRecoSegs", 0)};

    MuonVal::ScalarBranch<bool>&  m_out_hasMax {m_tree.newScalar<bool>("hasMax", false)}; 
    MuonVal::ScalarBranch<bool>&  m_out_max_hasPhiExtension {m_tree.newScalar<bool>("maxHasPhiExtension", false)}; 
    MuonVal::ScalarBranch<float>& m_out_max_matchFraction {m_tree.newScalar<float>("maxMatchFraction", false)}; 

    MuonVal::ScalarBranch<float>& m_out_max_y0{m_tree.newScalar<float>("maxY0", 0.0)}; 
    MuonVal::ScalarBranch<float>& m_out_max_x0{m_tree.newScalar<float>("maxX0", 0.0)}; 
    MuonVal::ScalarBranch<float>& m_out_max_tantheta{m_tree.newScalar<float>("maxTanTheta", 0.0)}; 
    MuonVal::ScalarBranch<float>& m_out_max_tanphi{m_tree.newScalar<float>("maxTanPhi", 0.0)};
    
    /** space point teste module */
    std::shared_ptr<MuonValR4::SpacePointTesterModule>      m_out_SP{nullptr}; 
    MuonVal::VectorBranch<unsigned char>& m_spacePointOnSeed{m_tree.newVector<unsigned char>("spacePoint_onSegmentSeed")};
    MuonVal::VectorBranch<unsigned char>& m_spacePointOnSegment{m_tree.newVector<unsigned char>("spacePoint_onSegment")};
    
    MuonVal::ScalarBranch<unsigned short>& m_out_max_nHits{m_tree.newScalar<unsigned short>("maxNHits", 0)}; 
    MuonVal::ScalarBranch<unsigned short>& m_out_max_nEtaHits{m_tree.newScalar<unsigned short>("maxNEtaHits", 0)}; 
    MuonVal::ScalarBranch<unsigned short>& m_out_max_nPhiHits{m_tree.newScalar<unsigned short>("maxNPhiHits", 0)}; 
    MuonVal::ScalarBranch<unsigned short>& m_out_max_nMdt{m_tree.newScalar<unsigned short>("maxNMdtHits", 0)}; 
    MuonVal::ScalarBranch<unsigned short>& m_out_max_nRpc{m_tree.newScalar<unsigned short>("maxNRpcHits", 0)}; 
    MuonVal::ScalarBranch<unsigned short>& m_out_max_nTgc{m_tree.newScalar<unsigned short>("maxNTgcHits", 0)}; 
    MuonVal::ScalarBranch<unsigned short>& m_out_max_nsTgc{m_tree.newScalar<unsigned short>("maxNsTgcHits", 0)};
    MuonVal::ScalarBranch<unsigned short>& m_out_max_nMm{m_tree.newScalar<unsigned short>("maxNMmHits", 0)};

    
    MuonVal::ScalarBranch<bool>&  m_out_hasSegment{m_tree.newScalar<bool>("hasSegment", false)}; 
    MuonVal::ScalarBranch<float>& m_out_segment_chi2{m_tree.newScalar<float>("segmentChi2", -1.)};
    MuonVal::ScalarBranch<uint16_t>& m_out_segment_nDoF{m_tree.newScalar<uint16_t>("segmentNdoF", 0)};
    MuonVal::ScalarBranch<bool>&  m_out_segment_hasPhi {m_tree.newScalar<bool>("segmentHasPhiHits", false)}; 
    MuonVal::ScalarBranch<bool>&  m_out_segment_hasTimeFit {m_tree.newScalar<bool>("segmentHasTimeFit", false)}; 
    MuonVal::ScalarBranch<uint16_t>&  m_out_segment_fitIter {m_tree.newScalar<uint16_t>("segmentFitIterations", 0)};


     
    MuonVal::ScalarBranch<float>& m_out_segment_y0{m_tree.newScalar<float>("segmentY0", 0.0)}; 
    MuonVal::ScalarBranch<float>& m_out_segment_x0{m_tree.newScalar<float>("segmentX0", 0.0)}; 
    MuonVal::ScalarBranch<float>& m_out_segment_tantheta{m_tree.newScalar<float>("segmentTanTheta", 0.0)}; 
    MuonVal::ScalarBranch<float>& m_out_segment_tanphi{m_tree.newScalar<float>("segmentTanPhi", 0.0)};
    MuonVal::ScalarBranch<float>& m_out_segment_time{m_tree.newScalar<float>("segmentTime", 0.)};
     

    MuonVal::ScalarBranch<float>& m_out_segment_err_y0{m_tree.newScalar<float>("segmentErrY0", -1.0)}; 
    MuonVal::ScalarBranch<float>& m_out_segment_err_x0{m_tree.newScalar<float>("segmentErrX0", -1.0)}; 
    MuonVal::ScalarBranch<float>& m_out_segment_err_tantheta{m_tree.newScalar<float>("segmentErrTanTheta", -1.0)}; 
    MuonVal::ScalarBranch<float>& m_out_segment_err_tanphi{m_tree.newScalar<float>("segmentErrTanPhi", -1.0)}; 
    MuonVal::ScalarBranch<float>& m_out_segment_err_time{m_tree.newScalar<float>("segmentErrTime", -1.0)};

    MuonVal::VectorBranch<float>& m_out_segment_chi2_measurement{m_tree.newVector<float>("segmentChi2Measurements")}; 
    MuonVal::ScalarBranch<unsigned short>& m_out_segment_truthMatchedHits{m_tree.newScalar<unsigned short>("segmentTruthMatchedHits", 0)};
    MuonVal::ScalarBranch<unsigned short>& m_out_segment_nMdtHits{m_tree.newScalar<unsigned short>("segmentNMdtHits", 0)};
    MuonVal::ScalarBranch<unsigned short>& m_out_segment_nRpcEtaHits{m_tree.newScalar<unsigned short>("segmentNRpcEtaHits", 0)};
    MuonVal::ScalarBranch<unsigned short>& m_out_segment_nRpcPhiHits{m_tree.newScalar<unsigned short>("segmentNRpcPhiHits", 0)};
    MuonVal::ScalarBranch<unsigned short>& m_out_segment_nTgcEtaHits{m_tree.newScalar<unsigned short>("segmentNTgcEtaHits", 0)};
    MuonVal::ScalarBranch<unsigned short>& m_out_segment_nTgcPhiHits{m_tree.newScalar<unsigned short>("segmentNTgcPhiHits", 0)};
    
  };
}

#endif // MUONFASTDIGITEST_MUONVALR4_MuonHoughTransformTester_H
