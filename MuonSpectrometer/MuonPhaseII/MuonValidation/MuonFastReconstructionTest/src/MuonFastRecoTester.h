/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONFASTRECONSTRUCTIONTEST_MUONFASTRECOTESTER_H
#define MUONFASTRECONSTRUCTIONTEST_MUONFASTRECOTESTER_H

// Framework includes
#include "AthenaBaseComps/AthHistogramAlgorithm.h"

// EDM includes 
#include "MuonFastRecoEvent/GlobalPattern.h"
#include "xAODMuon/MuonSegmentContainer.h"
#include <xAODTruth/TruthParticle.h>
#include <xAODMuonSimHit/MuonSimHit.h>

// muon includes
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonPRDTestR4/SpacePointTesterModule.h"

namespace MuonValR4{

  class MuonFastRecoTester : public AthHistogramAlgorithm {
  public:
    using AthHistogramAlgorithm::AthHistogramAlgorithm;
    virtual ~MuonFastRecoTester()  = default;

    virtual StatusCode initialize() override;
    virtual StatusCode execute() override;
    virtual StatusCode finalize() override;

  private:
    enum class eHitType : std::uint8_t {
        ePrec = 0,
        eTriggerEta = 1,
        ePhi = 2,
        nTypes = 3
    };
    using HitCounts = std::array<unsigned int, Acts::toUnderlying(eHitType::nTypes)>;
    struct PatternHitCount {
        HitCounts hitCounts{};
        HitCounts trueHitCounts{};
        HitCounts pileupHitCounts{};
        HitCounts allHitCounts{};
    };
    using simHitSet = std::unordered_set<const xAOD::MuonSimHit*>;
    using TruthParticleMap = std::map<const xAOD::TruthParticle*, std::vector<simHitSet>>;
    /** @brief Fill the truth particle map
     *  @param truthSegments: Pointer to the truth segments
     *  @return Map of truth particle hits */
    TruthParticleMap fillTruthMap(const xAOD::MuonSegmentContainer* truthSegments) const;
    /** @brief Fill the space point information into the tree
     *  @param spc: Pointer to the space point container
     *  @param patternCont: Pointer to the global pattern container, needed to match spacepoints to patterns
     *  @param truthHits: Map of truth particle hits
     *  @param spTester: Space point braches
     *  @param spTypeBranch: Branch for space point types
     *  @param spMatchedToPatternBranch: Branch for space point matches to patterns
     *  @param spMatchedToTruthBranch: Branch for space point matches to truth */
    void fillSpacePointInfo(const MuonR4::SpacePointContainer* spc,
                            const MuonR4::GlobalPatternContainer* patternCont,
                            const TruthParticleMap& truthHits,
                            SpacePointTesterModule& spTester,
                            MuonVal::VectorBranch<unsigned char>& spTypeBranch,
                            MuonVal::MatrixBranch<unsigned char>& spMatchedToPatternBranch,
                            MuonVal::MatrixBranch<unsigned char>& spMatchedToTruthBranch) const;  
    /** @brief Fill the info associated to the global patterns into the tree
     *  @param patternCont: Pointer to the global pattern container
     *  @param truthHits: Map of truth particle hits, needed to match patterns to truth
     *  @param spContainers: Vector of pointers to space point containers, needed to compute pattern hit counts */
    void fillGlobPatternInfo(const MuonR4::GlobalPatternContainer* patternCont,
                             const TruthParticleMap& truthHits,
                             const std::vector<const MuonR4::SpacePointContainer*>& spContainers);
    /** @brief Fill the truth particleinformation into the tree
    *  @param truthHits: Map of truth particle hits
    *  @param truthSegments: Truth segments needed for truth hit counts */
    void fillTruthInfo(const TruthParticleMap& truthHits,
                       const xAOD::MuonSegmentContainer* truthSegments);
                         
    // // output tree 
    MuonVal::MuonTesterTree m_tree{"MuonFastRecoTest","FastRecoTester"}; 

    // Space points
    SG::ReadHandleKey<MuonR4::SpacePointContainer> m_spKey{this, "SpacePointKey", "MuonSpacePoints"};
    SG::ReadHandleKey<MuonR4::SpacePointContainer> m_NSWspKey{this, "NswSpacePointKey", "NswSpacePoints"};

    // Global patterns
    SG::ReadHandleKey<MuonR4::GlobalPatternContainer> m_patternKey{this, "PatternKey", "R4MuonGlobalPatterns", "global pattern container"};
    
    SG::ReadHandleKey<xAOD::MuonSegmentContainer> m_truthSegmentKey {this, "TruthSegmentKey","MuonTruthSegments", "truth segment container"};
    
    SG::ReadHandleKey<ActsTrk::GeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
    
    Gaudi::Property<bool> m_isMC{this, "isMC", false, "Toggle whether the job is ran on MC or not"};

    Gaudi::Property<bool> m_writeSpacePoints{this, "writeSpacePoints", false,
                                             "Toggle whether the particular space poitns shall be written"};
                                            
  /// ====== Spacepoint block  =========== 
    /// @brief Branch dumping all the space points from the difference buckets
    std::shared_ptr<SpacePointTesterModule> m_spTester{};
    std::shared_ptr<SpacePointTesterModule> m_NSWspTester{};
    /// @brief Type of spacepoints: 1 for trigger eta, 2 for precision, 3 for only-phi
    MuonVal::VectorBranch<unsigned char>& m_spType{m_tree.newVector<unsigned char>(m_spKey.key()+"_type")};
    MuonVal::VectorBranch<unsigned char>& m_NSWspType{m_tree.newVector<unsigned char>(m_NSWspKey.key()+"_type")};
    /// @brief Branch indicating which space points in the tree are associated to the i-th pattern
    MuonVal::MatrixBranch<unsigned char>& m_spMatchedToPattern{m_tree.newMatrix<unsigned char>(m_spKey.key()+"_patternMatched")};
    MuonVal::MatrixBranch<unsigned char>& m_NSWspMatchedToPattern{m_tree.newMatrix<unsigned char>(m_NSWspKey.key()+"_patternMatched")};
    /// @brief Branch indicating which space points in the tree are associated to the i-th truth particle
    MuonVal::MatrixBranch<unsigned char>& m_spMatchedToTruth{m_tree.newMatrix<unsigned char>(m_spKey.key()+"_truthMatched")};
    MuonVal::MatrixBranch<unsigned char>& m_NSWspMatchedToTruth{m_tree.newMatrix<unsigned char>(m_NSWspKey.key()+"_truthMatched")};

  /// ====== Truth particle block  =========== 
    /// properties of truth particles
    MuonVal::VectorBranch<short>& m_gen_Q{m_tree.newVector<short>("gen_Q", 0)};
    MuonVal::VectorBranch<float>& m_gen_Eta{m_tree.newVector<float>("gen_Eta",-10.)};
    MuonVal::VectorBranch<float>& m_gen_Phi{m_tree.newVector<float>("gen_Phi",-10.)};
    MuonVal::VectorBranch<float>& m_gen_Pt{m_tree.newVector<float>("gen_Pt",-10.)}; 

    /// @brief Number of trigger eta measurements per station
    MuonVal::MatrixBranch<unsigned char>& m_gen_nNonPrecSpacePointsPerStation{m_tree.newMatrix<unsigned char>("gen_NNonPrecMeasPerStation")};
    /// @brief Number of precision measurements per station
    MuonVal::MatrixBranch<unsigned char>& m_gen_nPrecSpacePointsPerStation{m_tree.newMatrix<unsigned char>("gen_NPrecMeasPerStation")};
    /// @brief Number of phi measurements per station
    MuonVal::MatrixBranch<unsigned char>& m_gen_nPhiSpacePointsPerStation{m_tree.newMatrix<unsigned char>("gen_NPhiMeasPerStation")};

    
  /// ====== Global Pattern block  =========== 
    /// pattern count
    MuonVal::ScalarBranch<unsigned>& m_pat_n{m_tree.newScalar<unsigned>("pat_nPatterns", 0)};
    /// pattern average theta & phi    
    MuonVal::VectorBranch<float>& m_pat_Eta{m_tree.newVector<float>("pat_Eta", 0.0)};
    MuonVal::VectorBranch<float>& m_pat_phi{m_tree.newVector<float>("pat_Phi", 0.0)};
    /// pattern primary & secondary sectors (different if the pattern is in the sector overlap)  
    MuonVal::VectorBranch<uint16_t>& m_pat_sector1{m_tree.newVector<uint16_t>("pat_Sector1", 0)};
    MuonVal::VectorBranch<uint16_t>& m_pat_sector2{m_tree.newVector<uint16_t>("pat_Sector2", 0)};
    /// pattern residual
    MuonVal::VectorBranch<float>& m_pat_residual{m_tree.newVector<float>("pat_Residual", 0.0)};
    /// pattern normalized residual
    MuonVal::VectorBranch<float>& m_pat_normalizedResidual{m_tree.newVector<float>("pat_NormalizedResidual", 0.0)};
    /// +1 for A-, -1 of C-side 
    MuonVal::VectorBranch<short>& m_pat_side{m_tree.newVector<short>("pat_Side", 0)};
    /// Number of stations
    MuonVal::VectorBranch<unsigned char>& m_pat_nStations{m_tree.newVector<unsigned char>("pat_NStations", 0)};    

    /// @brief Number of trigger eta space points in the pattern
    MuonVal::VectorBranch<unsigned char>& m_pat_nNonPrecSpacePoints{m_tree.newVector<unsigned char>("pat_NNonPrecMeas",0)};
    /// @brief Number of precision measurements in the pattern
    MuonVal::VectorBranch<unsigned char>& m_pat_nPrecSpacePoints{m_tree.newVector<unsigned char>("pat_NPrecMeas",0)};
    /// @brief Number of phi measurements in the pattern
    MuonVal::VectorBranch<unsigned char>& m_pat_nPhiSpacePoints{m_tree.newVector<unsigned char>("pat_NPhiMeas",0)};
    
    /// @brief Number of truth trigger eta space points in the pattern
    MuonVal::VectorBranch<unsigned char>& m_pat_nTrueNonPrecSpacePoints{m_tree.newVector<unsigned char>("pat_NTrueNonPrecMeas",0)};
    /// @brief Number of truth precision space points in the pattern
    MuonVal::VectorBranch<unsigned char>& m_pat_nTruePrecSpacePoints{m_tree.newVector<unsigned char>("pat_NTruePrecMeas",0)};
    /// @brief Number of truth phi space points in the pattern
    MuonVal::VectorBranch<unsigned char>& m_pat_nTruePhiSpacePoints{m_tree.newVector<unsigned char>("pat_NTruePhiMeas",0)};

    /// @brief Number of pileup trigger eta space points in the pattern
    MuonVal::VectorBranch<unsigned char>& m_pat_nPileupNonPrecSpacePoints{m_tree.newVector<unsigned char>("pat_NPileupNonPrecMeas",0)};
    /// @brief Number of pileup precision space points in the pattern
    MuonVal::VectorBranch<unsigned char>& m_pat_nPileupPrecSpacePoints{m_tree.newVector<unsigned char>("pat_NPileupPrecMeas",0)};
    /// @brief Number of pileup phi space points in the pattern
    MuonVal::VectorBranch<unsigned char>& m_pat_nPileupPhiSpacePoints{m_tree.newVector<unsigned char>("pat_NPileupPhiMeas",0)};

    /// @brief Number of trigger eta space points in the buckets crossed by the pattern
    MuonVal::VectorBranch<unsigned char>& m_pat_nAllNonPrecSpacePoints{m_tree.newVector<unsigned char>("pat_NAllNonPrecMeas",0)};
    /// @brief Number of precision space points in the buckets crossed by the pattern
    MuonVal::VectorBranch<unsigned char>& m_pat_nAllPrecSpacePoints{m_tree.newVector<unsigned char>("pat_NAllPrecMeas",0)};
    /// @brief Number of phi space points in the buckets crossed by the pattern
    MuonVal::VectorBranch<unsigned char>& m_pat_nAllPhiSpacePoints{m_tree.newVector<unsigned char>("pat_NAllPhiMeas",0)};

    /// @brief Branch indicating which truth particles in the tree are associated to the i-th pattern
    MuonVal::MatrixBranch<unsigned char>& m_pat_MatchedToTruth{m_tree.newMatrix<unsigned char>("pat_truthMatched")};
    /// number of matched truth particles 
    MuonVal::VectorBranch<unsigned char> & m_pat_nTruthparticles{m_tree.newVector<unsigned char>("pat_NTruthParticles", 0)};

  };
}

#endif // MUONFASTDIGITEST_MUONVALR4_MuonHoughTransformTester_H
