/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONFASTRECONSTRUCTIONTEST_MUONFASTRECOTESTER_H
#define MUONFASTRECONSTRUCTIONTEST_MUONFASTRECOTESTER_H

// Framework includes
#include "AthenaBaseComps/AthHistogramAlgorithm.h"

// EDM includes 
#include "MuonFastRecoEvent/GlobalPattern.h"
#include <xAODMuon/Muon.h>
#include <xAODMuon/MuonContainer.h>
#include "xAODMuon/MuonSegmentContainer.h"
#include <xAODTruth/TruthParticle.h>
#include <xAODMuonSimHit/MuonSimHit.h>
#include "TrigSteeringEvent/TrigRoiDescriptorCollection.h"

// muon includes
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonPRDTestR4/SpacePointTesterModule.h"
#include "MuonSpacePoint/SpacePointPerLayerSorter.h"

namespace MuonValR4{

  class MuonFastRecoTester : public AthHistogramAlgorithm {
  public:
    using AthHistogramAlgorithm::AthHistogramAlgorithm;
    virtual ~MuonFastRecoTester()  = default;

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) override;
    virtual StatusCode finalize() override;

  private:
    /** Enum for measurement types */
    enum class measType {Prec, NonPrec, Phi, nTypes};
    using simHitSet = std::unordered_set<const xAOD::MuonSimHit*>;
    using TruthParticleMap = std::map<const xAOD::TruthParticle*, std::vector<simHitSet>>;
    /** @brief Fill the truth particle map
     *  @param truthSegments: Pointer to the truth segments
     *  @param roiCollection: Pointer to the RoI collection, needed to match truth particles to RoIs in seeded reco
     *  @return Map of truth particle hits */
    TruthParticleMap fillTruthMap(const xAOD::MuonSegmentContainer* truthSegments,
                                  const TrigRoiDescriptorCollection* roiCollection) const;
    /** @brief Fill the space point information into the tree
     *  @param spc: Pointer to the space point container
     *  @param patternCont: Pointer to the global pattern container, needed to match spacepoints to patterns
     *  @param truthHits: Map of truth particle hits
     *  @param spTester: Space point branches
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
    /** @brief Fill the info associated to fast reco muons
     *  @param muonCont: Pointer to the fast muon container
     *  @param patternCont: Pointer to the global pattern container, needed to match muons to patterns */
    void fillFastRecoMuonInfo(const xAOD::MuonContainer* muonCont,
                              const MuonR4::GlobalPatternContainer* patternCont);
    /** @brief Fill the truth particle information into the tree
    *  @param truthHits: Map of truth particle hits
    *  @param spContainers: Vector of pointers to space point containers, needed for truth hit counts */
    void fillTruthInfo(const TruthParticleMap& truthHits,
                       const std::vector<const MuonR4::SpacePointContainer*>& spContainers);
    /** @brief Fill the RoI information into the tree
    *  @param roiCollection: Pointer to the RoI collection */
    void fillRoIInfo(const TrigRoiDescriptorCollection* roiCollection);

    /** @brief Enum for different types of pattern hit content branches */
    enum class ePatBranchType : std::uint8_t {
        eReco,    // Counts of pattern hits
        eTruth,   // Counts of pattern hits matched to truth
        ePileup,  // Counts of pattern hits matched to pileup truth
        eMismatched, // Counts of pattern hits matched to truth but not to the main truth particle of the pattern
        eAll,     // Counts of all hits in the buckets crossed by the pattern
    };
    /** @brief Update the hit counts for a given pattern branch type
     *  @param type: The pattern branch type
     *  @param patIdx: Index of the pattern
     *  @param hitSt: Station index of the hit
     *  @param sp: Pointer to the space point
     *  @param isSecondaryMatched: if the spacepoint has a secondary measurement and it is matched to truth */
    void updatePatHitInfo(const ePatBranchType type, 
                          const std::size_t patIdx,
                          const Muon::MuonStationIndex::StIndex hitSt,
                          const MuonR4::SpacePoint* sp,
                          const bool isSecondaryMatched = false);
                         
    // // output tree 
    MuonVal::MuonTesterTree m_tree{"MuonFastRecoTest","FastRecoTester"}; 

    // Space points
    SG::ReadHandleKey<MuonR4::SpacePointContainer> m_spKey{this, "SpacePointKey", "MuonSpacePoints"};
    SG::ReadHandleKey<MuonR4::SpacePointContainer> m_NSWspKey{this, "NswSpacePointKey", "NswSpacePoints"};

    // Global patterns
    SG::ReadHandleKey<MuonR4::GlobalPatternContainer> m_patternKey{this, "PatternKey", "MuonR4GlobalPatterns", "global pattern container"};

    // Fast reco muons
    SG::ReadHandleKey<xAOD::MuonContainer> m_fastMuonKey{this, "FastMuonKey", "FastRecoSAMuons", "fast reco muon container"};
    
    // Truth segments
    SG::ReadHandleKey<xAOD::MuonSegmentContainer> m_truthSegmentKey {this, "TruthSegmentKey","MuonTruthSegments", "truth segment container"};

    // HLT seeding RoIs
    SG::ReadHandleKey<TrigRoiDescriptorCollection> m_roiCollectionKey{this, "MuRoIs", "EFMuMSReco_RoI", "Name of the input data from HLTSeeding"};
    
    ActsTrk::GeoContextReadKey_t m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
    
    BooleanProperty m_isMC{this, "isMC", false, "Toggle whether the job is ran on MC or not"};

    BooleanProperty m_isSeededReco{this, "isSeededReco", false, "Toggle whether the job is ran on seeded reconstruction or not"};

    BooleanProperty m_writeSpacePoints{this, "writeSpacePoints", false,
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
    MuonVal::MatrixBranch<unsigned char>& m_gen_nNonPrecMeas{m_tree.newMatrix<unsigned char>("gen_NNonPrecMeas", 0)};
    /// @brief Number of precision measurements per station
    MuonVal::MatrixBranch<unsigned char>& m_gen_nPrecMeas{m_tree.newMatrix<unsigned char>("gen_NPrecMeas", 0)};
    /// @brief Number of phi measurements per station
    MuonVal::MatrixBranch<unsigned char>& m_gen_nPhiMeas{m_tree.newMatrix<unsigned char>("gen_NPhiMeas", 0)};

    
  /// ====== Global Pattern block  =========== 
    /// pattern count
    MuonVal::ScalarBranch<unsigned>& m_pat_n{m_tree.newScalar<unsigned>("pat_nPatterns", 0)};
    /// pattern average theta & phi    
    MuonVal::VectorBranch<float>& m_pat_Eta{m_tree.newVector<float>("pat_Eta", 0.0)};
    MuonVal::VectorBranch<float>& m_pat_phi{m_tree.newVector<float>("pat_Phi", 0.0)};
    /// pattern primary & secondary sectors (different if the pattern is in the sector overlap)  
    MuonVal::VectorBranch<uint16_t>& m_pat_sector1{m_tree.newVector<uint16_t>("pat_Sector1", 0)};
    MuonVal::VectorBranch<uint16_t>& m_pat_sector2{m_tree.newVector<uint16_t>("pat_Sector2", 0)};
    /// mean square normalized pattern residual
    MuonVal::VectorBranch<float>& m_pat_meanNormResidual2{m_tree.newVector<float>("pat_meanNormResidual2", 0.0)};
    /// +1 for A-, -1 of C-side 
    MuonVal::VectorBranch<short>& m_pat_side{m_tree.newVector<short>("pat_Side", 0)};
    /// Number of stations
    MuonVal::VectorBranch<unsigned char>& m_pat_nStations{m_tree.newVector<unsigned char>("pat_NStations", 0)};    

    /// @brief Number of trigger eta measurements per station
    MuonVal::MatrixBranch<unsigned char>& m_pat_nNonPrecMeas{m_tree.newMatrix<unsigned char>("pat_NNonPrecMeas", 0)};
    /// @brief Number of precision measurements per station
    MuonVal::MatrixBranch<unsigned char>& m_pat_nPrecMeas{m_tree.newMatrix<unsigned char>("pat_NPrecMeas", 0)};
    /// @brief Number of phi measurements per station
    MuonVal::MatrixBranch<unsigned char>& m_pat_nPhiMeas{m_tree.newMatrix<unsigned char>("pat_NPhiMeas", 0)};
    
    /// @brief Number of truth trigger eta measurements per station
    MuonVal::MatrixBranch<unsigned char>& m_pat_nTruthNonPrecMeas{m_tree.newMatrix<unsigned char>("pat_NTruthNonPrecMeas", 0)};
    /// @brief Number of truth precision measurements per station
    MuonVal::MatrixBranch<unsigned char>& m_pat_nTruthPrecMeas{m_tree.newMatrix<unsigned char>("pat_NTruthPrecMeas", 0)};
    /// @brief Number of truth phi measurements per station
    MuonVal::MatrixBranch<unsigned char>& m_pat_nTruthPhiMeas{m_tree.newMatrix<unsigned char>("pat_NTruthPhiMeas", 0)};

    /// @brief Number of mismatched truth trigger eta measurements per station
    MuonVal::MatrixBranch<unsigned char>& m_pat_nMisTruthNonPrecMeas{m_tree.newMatrix<unsigned char>("pat_NMisTruthNonPrecMeas", 0)};
    /// @brief Number of mismatched truth precision measurements per station
    MuonVal::MatrixBranch<unsigned char>& m_pat_nMisTruthPrecMeas{m_tree.newMatrix<unsigned char>("pat_NMisTruthPrecMeas", 0)};
    /// @brief Number of mismatched truth phi measurements per station
    MuonVal::MatrixBranch<unsigned char>& m_pat_nMisTruthPhiMeas{m_tree.newMatrix<unsigned char>("pat_NMisTruthPhiMeas", 0)};

    /// @brief Number of pileup trigger eta measurements per station
    MuonVal::MatrixBranch<unsigned char>& m_pat_nPileupNonPrecMeas{m_tree.newMatrix<unsigned char>("pat_NPileupNonPrecMeas", 0)};
    /// @brief Number of pileup precision measurements per station
    MuonVal::MatrixBranch<unsigned char>& m_pat_nPileupPrecMeas{m_tree.newMatrix<unsigned char>("pat_NPileupPrecMeas", 0)};
    /// @brief Number of pileup phi measurements per station
    MuonVal::MatrixBranch<unsigned char>& m_pat_nPileupPhiMeas{m_tree.newMatrix<unsigned char>("pat_NPileupPhiMeas", 0)};

    /// @brief Number of trigger eta measurements in the buckets crossed by the pattern, grouped by station
    MuonVal::MatrixBranch<unsigned char>& m_pat_nAllNonPrecMeas{m_tree.newMatrix<unsigned char>("pat_NAllNonPrecMeas", 0)};
    /// @brief Number of precision measurements in the buckets crossed by the pattern, grouped by station
    MuonVal::MatrixBranch<unsigned char>& m_pat_nAllPrecMeas{m_tree.newMatrix<unsigned char>("pat_NAllPrecMeas", 0)};
    /// @brief Number of phi measurements in the buckets crossed by the pattern, grouped by station
    MuonVal::MatrixBranch<unsigned char>& m_pat_nAllPhiMeas{m_tree.newMatrix<unsigned char>("pat_NAllPhiMeas", 0)};

    /// @brief Branch indicating which truth particles in the tree are associated to the i-th pattern. We can have in principle
    /// multiple truth particles associated to the same pattern.
    MuonVal::MatrixBranch<unsigned char>& m_pat_MatchedToTruth{m_tree.newMatrix<unsigned char>("pat_truthMatched")};

  /// ====== RoI info  =========== 
    MuonVal::VectorBranch<float>& m_roi_EtaMin{m_tree.newVector<float>("roi_EtaMin",-10.)};
    MuonVal::VectorBranch<float>& m_roi_EtaMax{m_tree.newVector<float>("roi_EtaMax",-10.)};
    MuonVal::VectorBranch<float>& m_roi_PhiMin{m_tree.newVector<float>("roi_PhiMin",-10.)};
    MuonVal::VectorBranch<float>& m_roi_PhiMax{m_tree.newVector<float>("roi_PhiMax",-10.)};
    MuonVal::VectorBranch<float>& m_roi_ZMin{m_tree.newVector<float>("roi_ZMin",-10.)}; 
    MuonVal::VectorBranch<float>& m_roi_ZMax{m_tree.newVector<float>("roi_ZMax",-10.)};
    
  /// ====== Fast Reco Muon info  ===========
    MuonVal::VectorBranch<float>& m_muon_Eta{m_tree.newVector<float>("muon_Eta", -10.)};
    MuonVal::VectorBranch<float>& m_muon_Phi{m_tree.newVector<float>("muon_Phi", -10.)};
    MuonVal::VectorBranch<float>& m_muon_Pt{m_tree.newVector<float>("muon_Pt", -10.)};
    MuonVal::VectorBranch<short>& m_muon_Q{m_tree.newVector<short>("muon_Q", 0)};
    MuonVal::VectorBranch<unsigned char>& m_muon_MatchedToPattern{m_tree.newVector<unsigned char>("muon_patMatched")};

    MuonR4::SpacePointPerLayerSorter m_spSorter{};

  };
}

#endif
