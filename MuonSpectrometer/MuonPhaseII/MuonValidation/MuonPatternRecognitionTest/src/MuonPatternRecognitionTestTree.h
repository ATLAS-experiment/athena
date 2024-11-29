/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONVALR4_MuonPatternRecognitionTestTree__H
#define MUONVALR4_MuonPatternRecognitionTestTree__H


#include "AthenaBaseComps/AthHistogramAlgorithm.h"
#include "MuonTesterTree/MuonTesterTree.h"
#include "MuonTesterTree/ThreeVectorBranch.h"
#include "MuonTesterTree/VectorBranch.h"
#include "MuonTesterTree/IdentifierBranch.h"
#include "MuonPRDTestR4/SpacePointTesterModule.h"
#include "MuonPRDTestR4/SimHitTester.h"
#include "xAODMuonSimHit/MuonSimHitContainer.h"
#include "xAODMuon/MuonSegmentContainer.h"
#include <MuonPatternEvent/MuonPatternContainer.h>


namespace MuonValR4{

    /// helper struct to associate truth to reco segments and hough seeds   
    struct ObjectMatching{
        /** @brief Associated chamber */
        const MuonGMR4::SpectrometerSector* chamber{nullptr};
        /** @brief Truth segment for reference */
        const xAOD::MuonSegment* truthSegment{nullptr};
        /// @brief All segments matched to this object
        std::vector<const MuonR4::Segment*> matchedSegments;
        /// @brief Shared sim hit counts for segments matched to this object
        std::vector<unsigned> matchedSegHits{0};
        /// @brief All seeds matched to this object
        std::vector<const MuonR4::SegmentSeed*> matchedSeeds; 
        /// @brief Shared sim hit counts for seeds matched to this object
        std::vector<unsigned> matchedSeedHits{0};
        std::vector<char> matchedSeedFoundSegment{0};
    };

    class MuonPatternRecognitionTestTree: public MuonTesterTree{
        public:
        /// constructor delegates to the parent class
        using MuonTesterTree::MuonTesterTree;

        /// initialisation - internally calls the MuonTesterTree::init method 
        StatusCode initialize(AthHistogramAlgorithm* parent); 
        void fillChamberInfo(const MuonGMR4::SpectrometerSector* chamber);

        void fillTruthInfo(const xAOD::MuonSegment* truthSegment, const MuonGMR4::MuonDetectorManager* detMgr, const ActsGeometryContext& gctx);
        
        void fillSeedInfo(const ObjectMatching& obj);            
        void fillSegmentInfo(const ActsGeometryContext& gctx, const ObjectMatching& obj);  

        protected: 
        

        /// ====== Common block: Filled for all entries =========== 

        /// chamber index field 
        MuonVal::ScalarBranch<int>& m_out_chamberIndex{newScalar<int>("chamberIndex")};
        /// +1 for A-, -1 of C-side 
        MuonVal::ScalarBranch<short>& m_out_stationSide{newScalar<short>("stationSide")};
        /// phi index of the station
        MuonVal::ScalarBranch<int>& m_out_stationPhi{newScalar<int>("stationPhi")};
        MuonVal::ScalarBranch<float>& m_out_bucketStart{newScalar<float>("bucketStart", 1)};
        MuonVal::ScalarBranch<float>& m_out_bucketEnd{newScalar<float>("bucketEnd", -1)};

        /// ======= Truth block: Filled if we have a truth match. ============ 

        /// existence of a truth match 
        MuonVal::ScalarBranch<bool> & m_out_hasTruth{newScalar<bool>("hasTruth",false)};
        
        /** @brief global particle properties */
        MuonVal::ScalarBranch<float>& m_out_gen_Eta{newScalar<float>("genEta",-10.)};
        MuonVal::ScalarBranch<float>& m_out_gen_Phi{newScalar<float>("genPhi",-10.)};
        MuonVal::ScalarBranch<float>& m_out_gen_Pt{newScalar<float>("genPt",-10.)};    
        MuonVal::ScalarBranch<short>& m_out_gen_Q{newScalar<short>("genQ", 0)};
        /** @brief Truth - segment parameters  */
        MuonVal::ScalarBranch<float>& m_out_gen_y0{newScalar<float>("genY0", 0.0)}; 
        MuonVal::ScalarBranch<float>& m_out_gen_tantheta{newScalar<float>("genTanTheta", 0.0)}; 
        MuonVal::ScalarBranch<float>& m_out_gen_tanphi{newScalar<float>("genTanPhi", 0.0)}; 
        MuonVal::ScalarBranch<float>& m_out_gen_x0{newScalar<float>("genX0", 0.0)}; 
        MuonVal::ScalarBranch<float>& m_out_gen_time{newScalar<float>("genTime", 0.0)}; 
        /** @brief Truth - hit count summary */
        MuonVal::ScalarBranch<unsigned short>& m_out_gen_nHits{newScalar<unsigned short>("genNHits",0)};
        MuonVal::ScalarBranch<unsigned short>& m_out_gen_nRPCHits{newScalar<unsigned short>("genNRpcHits",0)};
        MuonVal::ScalarBranch<unsigned short>& m_out_gen_nMDTHits{newScalar<unsigned short>("genNMdtHits",0)};
        MuonVal::ScalarBranch<unsigned short>& m_out_gen_nTGCHits{newScalar<unsigned short>("genNTgcHits",0)};
        MuonVal::ScalarBranch<unsigned short>& m_out_gen_nNswHits{newScalar<unsigned short>("genNNswHits",0)};
        // truth segment size in the y direction
        MuonVal::ScalarBranch<float>& m_out_gen_minYhit{newScalar<float>("genMinYhit", 1.0)}; 
        MuonVal::ScalarBranch<float>& m_out_gen_maxYhit{newScalar<float>("genMaxYhit", -1.0)}; 

        /// ========== Seed block: Filled when we have one or multiple seeds ============= 
        /// seed count
        MuonVal::ScalarBranch<unsigned>& m_out_seed_n{newScalar<unsigned>("nSeeds", 0)};
        // the following are filled with one entry per seed 

        // does the seed have a phi-extension? 
        MuonVal::VectorBranch<bool>&  m_out_seed_hasPhiExtension {newVector<bool>("seedHasPhiExtension", false)}; 
        // fraction of the hits on the seed matched to truth 
        MuonVal::VectorBranch<float>& m_out_seed_nMatchedHits {newVector<float>("seedTruthMatchedHits", false)}; 

        // parameters of the seed 
        MuonVal::VectorBranch<float>& m_out_seed_y0{newVector<float>("seedY0", 0.0)}; 
        MuonVal::VectorBranch<float>& m_out_seed_x0{newVector<float>("seedX0", 0.0)}; 
        MuonVal::VectorBranch<float>& m_out_seed_tantheta{newVector<float>("seedTanTheta", 0.0)}; 
        MuonVal::VectorBranch<float>& m_out_seed_tanphi{newVector<float>("seedTanPhi", 0.0)};

        // seed size in the y direction
        MuonVal::VectorBranch<float>& m_out_seed_minYhit{newVector<float>("seedMinYhit", 1.0)}; 
        MuonVal::VectorBranch<float>& m_out_seed_maxYhit{newVector<float>("seedMaxYhit", -1.0)}; 
        // hit counts on the seed
        MuonVal::VectorBranch<unsigned short>& m_out_seed_nHits{newVector<unsigned short>("seedNHits", 0)}; 
        MuonVal::VectorBranch<unsigned short>& m_out_seed_nEtaHits{newVector<unsigned short>("seedNEtaHits", 0)}; 
        MuonVal::VectorBranch<unsigned short>& m_out_seed_nPhiHits{newVector<unsigned short>("seedNPhiHits", 0)}; 
        MuonVal::VectorBranch<unsigned short>& m_out_seed_nMdt{newVector<unsigned short>("seedNMdtHits", 0)}; 
        MuonVal::VectorBranch<unsigned short>& m_out_seed_nRpc{newVector<unsigned short>("seedNRpcHits", 0)}; 
        MuonVal::VectorBranch<unsigned short>& m_out_seed_nTgc{newVector<unsigned short>("seedNTgcHits", 0)}; 
        MuonVal::VectorBranch<unsigned short>& m_out_seed_nsTgc{newVector<unsigned short>("seedNsTgcHits", 0)};
        MuonVal::VectorBranch<unsigned short>& m_out_seed_nMm{newVector<unsigned short>("seedNMmHits", 0)};

        MuonVal::VectorBranch<bool> & m_out_seed_ledToSegment{newVector<bool>("seedConvertedToSegment",false)}; 
        
        /// ========== Segment block: Filled when we have one or multiple segments ============= 

        // count of segments 
        MuonVal::ScalarBranch<unsigned>&  m_out_segment_n{newScalar<unsigned>("nSegments", 0)}; 
        // the following are filled with one entry per segment

        // fit metrics 
        MuonVal::VectorBranch<float>& m_out_segment_chi2{newVector<float>("segmentChi2", -1.)};
        MuonVal::VectorBranch<uint16_t>& m_out_segment_nDoF{newVector<uint16_t>("segmentNdoF", 0)};
        MuonVal::VectorBranch<bool>&  m_out_segment_hasTimeFit {newVector<bool>("segmentHasTimeFit", false)}; 
        MuonVal::VectorBranch<uint16_t>&  m_out_segment_fitIter {newVector<uint16_t>("segmentFitIterations", 0)};

        // presence of a phi extension
        MuonVal::VectorBranch<bool>&  m_out_segment_hasPhi {newVector<bool>("segmentHasPhiHits", false)}; 
        
        // segment parameters
        MuonVal::VectorBranch<float>& m_out_segment_y0{newVector<float>("segmentY0", 0.0)}; 
        MuonVal::VectorBranch<float>& m_out_segment_x0{newVector<float>("segmentX0", 0.0)}; 
        MuonVal::VectorBranch<float>& m_out_segment_tantheta{newVector<float>("segmentTanTheta", 0.0)}; 
        MuonVal::VectorBranch<float>& m_out_segment_tanphi{newVector<float>("segmentTanPhi", 0.0)};
        MuonVal::VectorBranch<float>& m_out_segment_time{newVector<float>("segmentTime", 0.)};
        
        // segment uncertainties
        MuonVal::VectorBranch<float>& m_out_segment_err_y0{newVector<float>("segmentErrY0", -1.0)}; 
        MuonVal::VectorBranch<float>& m_out_segment_err_x0{newVector<float>("segmentErrX0", -1.0)}; 
        MuonVal::VectorBranch<float>& m_out_segment_err_tantheta{newVector<float>("segmentErrTanTheta", -1.0)}; 
        MuonVal::VectorBranch<float>& m_out_segment_err_tanphi{newVector<float>("segmentErrTanPhi", -1.0)}; 
        MuonVal::VectorBranch<float>& m_out_segment_err_time{newVector<float>("segmentErrTime", -1.0)};

        // hit counts on segment 
        MuonVal::VectorBranch<unsigned short>& m_out_segment_truthMatchedHits{newVector<unsigned short>("segmentTruthMatchedHits", 0)};
        MuonVal::VectorBranch<unsigned short>& m_out_segment_nMdtHits{newVector<unsigned short>("segmentNMdtHits", 0)};
        MuonVal::VectorBranch<unsigned short>& m_out_segment_nRpcEtaHits{newVector<unsigned short>("segmentNRpcEtaHits", 0)};
        MuonVal::VectorBranch<unsigned short>& m_out_segment_nRpcPhiHits{newVector<unsigned short>("segmentNRpcPhiHits", 0)};
        MuonVal::VectorBranch<unsigned short>& m_out_segment_nTgcEtaHits{newVector<unsigned short>("segmentNTgcEtaHits", 0)};
        MuonVal::VectorBranch<unsigned short>& m_out_segment_nTgcPhiHits{newVector<unsigned short>("segmentNTgcPhiHits", 0)};
        
        // segment size in the y direction
        MuonVal::VectorBranch<float>& m_out_segment_minYhit{newVector<float>("segmentMinYhit", 1.0)}; 
        MuonVal::VectorBranch<float>& m_out_segment_maxYhit{newVector<float>("segmentMaxYhit", -1.0)}; 


    };


} // namespace MuonValR4

#endif //  MUONVALR4_MuonPatternRecognitionTestTree__H
