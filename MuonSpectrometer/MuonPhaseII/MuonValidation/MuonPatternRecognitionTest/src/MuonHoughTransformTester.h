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

#include "MuonPatternRecognitionTestTree.h"

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
    
    
    // MDT sim hits in xAOD format 
    SG::ReadHandleKey<xAOD::MuonSegmentContainer> m_truthSegmentKey {this, "TruthSegmentKey","TruthSegmentsR4", "truth segment container"};
                                                          
    SG::ReadHandleKey<MuonR4::SegmentSeedContainer> m_inHoughSegmentSeedKey{this, "SegmentSeedKey", "MuonHoughStationSegmentSeeds"};
    SG::ReadHandleKey<MuonR4::SegmentContainer> m_inSegmentKey{this, "SegmentKey", "R4MuonSegments"};
    SG::ReadHandleKey<MuonR4::SpacePointContainer> m_spKey{this, "SpacePointKey", "MuonSpacePoints"};

    SG::ReadHandleKey<ActsGeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};

    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
   
    Gaudi::Property<bool> m_writeSpacePoints{this, "writeSpacePoints", false,
                                             "Toggle whether the particular space poitns shall be written"};
    
    /// @brief Branch dumping all the space points from the difference buckets
    std::shared_ptr<SpacePointTesterModule> m_spTester{};
    /// @brief Branch indicating to which bucket the i-th pattern is associated
    std::shared_ptr<MuonVal::VectorBranch<uint16_t>> m_patternBucket{};
    /// @brief Branch indicatin to which bucket the i-th segment is associated
    std::shared_ptr<MuonVal::VectorBranch<uint16_t>> m_segmentBucket{};
    /// @brief Branch indicating which space points in the tree are associated to the i-th pattern
    std::shared_ptr<MuonVal::MatrixBranch<unsigned char>> m_spMatchedToPattern{};
    /// @brief Branch indicating which space points in the tree are associated to the i-th segment
    std::shared_ptr<MuonVal::MatrixBranch<unsigned char>> m_spMatchedToSegment{};

    /// Pattern visualization tool
    ToolHandle<MuonValR4::IPatternVisualizationTool> m_visionTool{this, "VisualizationTool", ""};
 
    const MuonGMR4::MuonDetectorManager* m_r4DetMgr{nullptr};

    // // output tree - allows to compare the sim and fast-digitised hits
    MuonValR4::MuonPatternRecognitionTestTree m_tree{"MuonEtaHoughTest","MuonEtaHoughTransformTest"}; 
  };
}

#endif // MUONFASTDIGITEST_MUONVALR4_MuonHoughTransformTester_H
