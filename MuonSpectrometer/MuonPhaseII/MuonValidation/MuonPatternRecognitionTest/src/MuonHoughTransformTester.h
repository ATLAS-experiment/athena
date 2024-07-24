/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONVALR4_MuonHoughTransformTester_H
#define MUONVALR4_MuonHoughTransformTester_H

// Framework includes
#include "AthenaBaseComps/AthHistogramAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadHandleKeyArray.h"
#include "StoreGate/ReadCondHandleKey.h"

// EDM includes 
#include "xAODMuonSimHit/MuonSimHitContainer.h"

#include <MuonPatternEvent/MuonPatternContainer.h>
#include <MuonPatternEvent/MuonSegment.h>

#include <MuonReadoutGeometryR4/MuonDetectorManager.h>

// muon includes
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
    MuonHoughTransformTester(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~MuonHoughTransformTester()  = default;

    virtual StatusCode initialize() override;
    virtual StatusCode execute() override;
    virtual StatusCode finalize() override;

  private:
    
    /// Helper method to fetch data from StoreGate. If the key is empty, a nullptr is assigned to the container ptr
    /// Failure is returned in cases, of non-empty keys and failed retrieval
    template <class ContainerType> StatusCode retrieveContainer(const EventContext& ctx,
                                                                const SG::ReadHandleKey<ContainerType>& key,
                                                                const ContainerType* & contToPush) const;

    Amg::Transform3D toChamberTrf(const ActsGeometryContext& gctx,
                                  const Identifier& hitId) const;

    StatusCode drawEventDisplay(const EventContext& ctx,
                                const std::vector<const xAOD::MuonSimHit*>& simHits,
                                const MuonR4::SegmentSeed* foundMax) const;

    StatusCode drawChi2(        const EventContext& ctx,
                                const std::vector<const xAOD::MuonSimHit*>& simHits,
                                const MuonR4::SegmentSeed* foundMax,
                                const MuonR4::MuonSegment* foundSegment,
                                const std::string & label, const ActsGeometryContext & gctx) const;

    struct chamberLevelObjects { 
        struct SeedMatchQuantites {
            /** @brief Best matched truth particle */
            HepMC::ConstGenParticlePtr truthParticle{};
            /** @brief Probability of which the segment is matched to it */
            double matchProb{0.};
            /** @brief Associated segment */
            const MuonR4::MuonSegment* segment{nullptr};
        };
        using SeedMatchMap = std::map<const MuonR4::SegmentSeed*, SeedMatchQuantites>;
        SeedMatchMap seedMatching{};

        /** @brief Collection of the truth particle trajectory */
        struct TruthMatchQuantities{
            std::vector<const xAOD::MuonSimHit*> detectorHits{};
            std::vector<const MuonR4::SegmentSeed*> assocSeeds{};
        };

        std::map<HepMC::ConstGenParticlePtr, TruthMatchQuantities> truthMatching{}; 
    };

    void matchSeedToTruth(const MuonR4::SegmentSeed* seed, chamberLevelObjects & objs ) const;                          
    std::pair<HepMC::ConstGenParticlePtr, double> matchSegmentToTruth(const MuonR4::MuonSegment* seed, chamberLevelObjects & objs ) const;                          
    void matchSeedsToTruth(chamberLevelObjects & objs) const;          
    void matchSegmentsToTruth(chamberLevelObjects & objs) const;          
    void fillChamberInfo(const MuonGMR4::MuonChamber* chamber);                
    void fillTruthInfo(const HepMC::ConstGenParticlePtr genParticlePtr, const std::vector<const xAOD::MuonSimHit*> & simHits, const ActsGeometryContext & gctx);     
    void fillSeedInfo(const MuonR4::SegmentSeed* segmentSeed, double matchProb);            
    void fillSegmentInfo(const MuonR4::MuonSegment* segmentSeed, double matchProb);            
    
    // MDT sim hits in xAOD format 
    SG::ReadHandleKeyArray<xAOD::MuonSimHitContainer> m_inSimHitKeys {this, "SimHitKeys",{}, "xAOD  SimHit collections"};
                                                          
    SG::ReadHandleKey<MuonR4::SegmentSeedContainer> m_inHoughSegmentSeedKey{this, "SegmentSeedKey", "MuonHoughStationSegmentSeeds"};
    SG::ReadHandleKey<MuonR4::MuonSegmentContainer> m_inSegmentKey{this, "SegmentKey", "R4MuonSegments"};
    SG::ReadHandleKey<MuonR4::SpacePointContainer> m_spacePointKey{this, "SpacePointKey", "MuonSpacePoints"};
    
    SG::ReadHandleKey<ActsGeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};

    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
    const MuonGMR4::MuonDetectorManager* m_r4DetMgr{nullptr};

    // // output tree - allows to compare the sim and fast-digitised hits
    MuonVal::MuonTesterTree m_tree{"MuonEtaHoughTest","MuonEtaHoughTransformTest"}; 
    
    MuonVal::ScalarBranch<int>&            m_out_stationName{m_tree.newScalar<int>("stationName")};
    MuonVal::ScalarBranch<int>&            m_out_stationEta{m_tree.newScalar<int>( "stationEta")};
    MuonVal::ScalarBranch<int>&            m_out_stationPhi{m_tree.newScalar<int>( "stationPhi")};

    MuonVal::ScalarBranch<bool> &          m_out_hasTruth{m_tree.newScalar<bool>("hasTruth",false)}; 

    MuonVal::ScalarBranch<float>&          m_out_gen_Eta{m_tree.newScalar<float>("genEta",-10.)};
    MuonVal::ScalarBranch<float>&          m_out_gen_Phi{m_tree.newScalar<float>("genPhi",-10.)};
    MuonVal::ScalarBranch<float>&          m_out_gen_Pt{m_tree.newScalar<float>("genPt",-10.)};
    
    MuonVal::ScalarBranch<float>&           m_out_gen_tantheta{m_tree.newScalar<float>("genTanTheta", 0.0)}; 
    MuonVal::ScalarBranch<float>&           m_out_gen_y0{m_tree.newScalar<float>("genY0", 0.0)}; 
    MuonVal::ScalarBranch<float>&           m_out_gen_tanphi{m_tree.newScalar<float>("genTanPhi", 0.0)}; 
    MuonVal::ScalarBranch<float>&           m_out_gen_x0{m_tree.newScalar<float>("genX0", 0.0)}; 

    MuonVal::ScalarBranch<unsigned int>&   m_out_gen_nHits{m_tree.newScalar<unsigned int>("genNHits",0)};
    MuonVal::ScalarBranch<unsigned int>&   m_out_gen_nRPCHits{m_tree.newScalar<unsigned int>("genNRpcHits",0)};
    MuonVal::ScalarBranch<unsigned int>&   m_out_gen_nMDTHits{m_tree.newScalar<unsigned int>("genNMdtHits",0)};
    MuonVal::ScalarBranch<unsigned int>&   m_out_gen_nTGCHits{m_tree.newScalar<unsigned int>("genNTgcHits",0)};
    MuonVal::ScalarBranch<unsigned int>&   m_out_gen_nsTGCHits{m_tree.newScalar<unsigned int>("genNsTgcHits",0)};
    MuonVal::ScalarBranch<unsigned int>&   m_out_gen_nMMits{m_tree.newScalar<unsigned int>("genNMmHits",0)};
    MuonVal::ScalarBranch<bool>&  m_out_hasMax {m_tree.newScalar<bool>("hasMax", false)}; 
    MuonVal::ScalarBranch<bool>&  m_out_max_hasPhiExtension {m_tree.newScalar<bool>("maxHasPhiExtension", false)}; 
    MuonVal::ScalarBranch<float>&  m_out_max_matchFraction {m_tree.newScalar<float>("maxMatchFraction", false)}; 

    MuonVal::ScalarBranch<float>& m_out_max_y0{m_tree.newScalar<float>("maxY0", 0.0)}; 
    MuonVal::ScalarBranch<float>& m_out_max_x0{m_tree.newScalar<float>("maxX0", 0.0)}; 
    MuonVal::ScalarBranch<float>& m_out_max_tantheta{m_tree.newScalar<float>("maxTanTheta", 0.0)}; 
    MuonVal::ScalarBranch<float>& m_out_max_tanphi{m_tree.newScalar<float>("maxTanPhi", 0.0)}; 
    
    /** space point teste module */
    std::shared_ptr<MuonValR4::SpacePointTesterModule>      m_out_SP{nullptr}; 
    MuonVal::VectorBranch<unsigned char>& m_sacePointOnSeed{m_tree.newVector<unsigned char>("spacePoint_onSegmentSeed")};
    MuonVal::VectorBranch<unsigned char>& m_sacePointOnSegment{m_tree.newVector<unsigned char>("spacePoint_onSegment")};
    
    MuonVal::ScalarBranch<unsigned int>& m_out_max_nHits{m_tree.newScalar<unsigned int>("maxNHits", 0)}; 
    MuonVal::ScalarBranch<unsigned int>& m_out_max_nEtaHits{m_tree.newScalar<unsigned int>("maxNEtaHits", 0)}; 
    MuonVal::ScalarBranch<unsigned int>& m_out_max_nPhiHits{m_tree.newScalar<unsigned int>("maxNPhiHits", 0)}; 
    MuonVal::ScalarBranch<unsigned int>& m_out_max_nMdt{m_tree.newScalar<unsigned int>("maxNMdtHits", 0)}; 
    MuonVal::ScalarBranch<unsigned int>& m_out_max_nRpc{m_tree.newScalar<unsigned int>("maxNRpcHits", 0)}; 
    MuonVal::ScalarBranch<unsigned int>& m_out_max_nTgc{m_tree.newScalar<unsigned int>("maxNTgcHits", 0)}; 
    MuonVal::ScalarBranch<unsigned int>& m_out_max_nsTgc{m_tree.newScalar<unsigned int>("maxNsTgcHits", 0)};
    MuonVal::ScalarBranch<unsigned int>& m_out_max_nMm{m_tree.newScalar<unsigned int>("maxNMmHits", 0)};

    
    MuonVal::ScalarBranch<bool>&  m_out_hasSegment {m_tree.newScalar<bool>("hasSegment", false)}; 
    MuonVal::ScalarBranch<float>&  m_out_segment_matchFraction {m_tree.newScalar<float>("segmentMatchFraction", false)}; 
    MuonVal::ScalarBranch<float>& m_out_segment_chi2{m_tree.newScalar<float>("segmentChi2", 0.0)}; 
    MuonVal::ScalarBranch<float>& m_out_segment_tantheta{m_tree.newScalar<float>("segmentTanTheta", 0.0)}; 
    MuonVal::ScalarBranch<float>& m_out_segment_z0{m_tree.newScalar<float>("segmentZ0", 0.0)}; 
    MuonVal::ScalarBranch<float>& m_out_segment_tanphi{m_tree.newScalar<float>("segmentTanPhi", 0.0)}; 
    MuonVal::ScalarBranch<float>& m_out_segment_x0{m_tree.newScalar<float>("segmentX0", 0.0)}; 
    MuonVal::VectorBranch<double>& m_out_segment_chi2_measurement{m_tree.newVector<double>("segmentChi2Measurements", 0.0)}; 
    
    /// Draw the event display for the cases where the hough transform did not find any hough maximum
    Gaudi::Property<bool> m_drawEvtDisplayFailure{this, "drawDisplayFailed", false};
    /// Draw the event dispalty for the successful cases
    Gaudi::Property<bool> m_drawEvtDisplaySuccess{this, "drawDisplaySuccss", false};
    /// Add beamline constraint
    Gaudi::Property<bool> m_doBeamspotConstraint{this, "doBeamspotConstraint", false};
    
    std::unique_ptr<TCanvas> m_allCan{};
    Gaudi::Property<std::string> m_allCanName{this, "AllCanvasName", "AllHoughiDiPuffDisplays.pdf"};

  };
}

#endif // MUONFASTDIGITEST_MUONVALR4_MuonHoughTransformTester_H
