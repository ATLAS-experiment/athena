/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONR4_MUONFASTRECOALGS_MUONFASTRECONSTRUCTIONALG__H
#define MUONR4_MUONFASTRECOALGS_MUONFASTRECONSTRUCTIONALG__H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/WriteHandleKey.h"

#include "MuonFastRecoHelpers/GlobalPatternFinder.h"
#include "MuonSpacePoint/SpacePointContainer.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"

namespace MuonR4{
    
    /// @brief Algorithm executing Phase-2 fast reconstruction 
    /// 
    /// This algorithm performs the Phase-2 fast reconstruction stage 
    /// before the precision tracking in the Phase-2 Event Filter. It
    /// receives as input the collection of space-point containers and
    /// produces """"to be defined""" muon objects, which will be used to
    /// perform trigger decisions and be combined with InnerDetector tracks
    /// in the Combined Fast Reconstruction step. It will optionally write 
    /// the GlobalPattern objects into the event store.
    /// The algorithm builds global patterns through the standalone module 
    /// GlobalPatternFinder, fits segments in each station and use them
    /// to estimate the muon momentum.

    class FastReconstructionAlg: public AthReentrantAlgorithm{
        public:
            using AthReentrantAlgorithm::AthReentrantAlgorithm;
            virtual ~FastReconstructionAlg() = default;

            virtual StatusCode initialize() override;
            virtual StatusCode execute(const EventContext& ctx) const override;

        private:
            /** @brief Abrivation for a vector of space-point containers */
            using SpacePointContainerVec = FastReco::GlobalPatternFinder::SpacePointContainerVec;
            /** @brief Abrivation for a vector of global patterns */
            using PatternVec = FastReco::GlobalPatternFinder::PatternVec;

            /** @brief Keys of SpacePoint containers to read */
            SG::ReadHandleKeyArray<SpacePointContainer> m_inSpacePoints{this, "InSpacePoints", {"MuonSpacePoints"}, "List of space point containers to read"};
            /** @brief Write handle key for the output buckets */
            SG::WriteHandleKeyArray<SpacePointContainer> m_outSpacePoints{this, "OutSpacePoints", {}, "List of space point containers to write"};
            /** @brief Suffix to add to the input space point container names to create the output container names, when not provided*/
            StringProperty m_outSpacePointSuffix{this, "OutSpacePointSuffix", "FastReco", "Suffix to add to input space point container names to create the output ones, when not provided"};
            /** @brief Write handle key for the output global patterns */ 
            SG::WriteHandleKey<GlobalPatternContainer> m_outPatterns{this, "OutPatternContainer", "R4MuonGlobalPatterns"};
            /** @brief Geometry context key */              
            SG::ReadHandleKey<ActsTrk::GeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};
            /** @brief Handle to the MuonIdHelper service */        
            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
            /** @brief Handle to the visualization tool */
            ToolHandle<MuonValR4::IFastRecoVisualizationTool> m_visionTool{this, "VisualizationTool", ""};

            /** ----------------- Configuration options for the global pattern finder ----------------- */
            /** @brief Toggle the utilization of MDT hits to build patterns */
            BooleanProperty m_useMdtHits {this, "UseMdtHits", true, "Activate the utilization of MDT hits to build patterns"};
            /** @brief Toggle the seeding from MDT hits */
            BooleanProperty m_seedFromMdt {this, "SeedFromMdt", false, "Activate the seeding from MDT hits"};
            /** @brief Activate the seeding from Inner station */
            BooleanProperty m_seedFromInner {this, "SeedFromInner", false, "Activate the seeding from Inner station"};
            /** @brief Size of theta window [rad] to search for compatible hits with a seed, tailored to the target pt cutoff */
            DoubleProperty m_thetaSearchWindow {this, "ThetaWindowSearch", 0.045, "Size of the search window in theta to link hits to a pattern"};
            /** @brief Çonstant term [mm] of the acceptance window for the residual between a test hit and the pattern line. Represents the minimum window 
             *         size regardless of the geometry and absorbing unmodelled effects such as muon bending in the magnetic field and scattering */
            DoubleProperty m_baseRWindow {this, "BaseRWindow", 140, "Çonstant term [mm] of the acceptance window for the residual between a test hit and the pattern line"};
            /** @brief Maximum phi difference [rad] allowed between two hits belonging to the same pattern */
            DoubleProperty m_phiTolerance {this, "PhiTolerance", 0.1, "Maximum allowed phi difference between two hits to be considered compatible"};
            /** @brief Requirement on trigger hits in the bending direction to accept a pattern  */
            UnsignedIntegerProperty m_minBendingTriggerHits {this, "MinBendingTriggerHits", 2, "Minimum number of trigger hits in the bending direction required to accept a pattern"};
            /** @brief Requirement on precision hits in the bending direction to accept a pattern  */
            UnsignedIntegerProperty m_minBendingPrecisionHits {this, "MinBendingPrecisionHits", 8, "Minimum number of precision hits in the bending direction required to accept a pattern"};
            /** @brief Minimum number of phi measurements required to accept a pattern */
            UnsignedIntegerProperty m_minPhiHits {this, "MinPhiHits", 1, "Minimum number of phi measurements required to accept a pattern"};
            /** @brief Quality cut on pattern'mean squared normalized residual. Set to a large value to disable the cut, e.g. 10. */
            DoubleProperty m_meanNormRes2Cut {this, "meanNormRes2Cut", 0.2, "Quality cut on pattern'mean squared normalized residual"};
            /** @brief Maximum number of attempts to build a pattern from hits already used in existing patterns */
            UnsignedIntegerProperty m_maxSeedAttempts {this, "MaxSeedAttempts", 6, " Maximum number of attempts to build a pattern from hits already used in existing patterns"};
            /** @brief Maximum number of missed candidate hits in different measurement layers during pattern building allowed for a pattern branch before it is discarded */
            UnsignedIntegerProperty m_maxMissedLayerHits {this, "MaxMissedLayerHits", 3, "Maximum number of missed candidate hits in different measurement layers during pattern building allowed for a pattern branch before it is discarded"};
            /** @brief Minimum separation [mm] between the measurement layers of two hits for being used to compute a reliable pattern line. Use the beamspot otherwise. */
            DoubleProperty m_minLayerSeparation {this, "MinLayerSeparation",40, "Minimum separation (in mm) between the measurement layers of two hits for being used to compute a reliable pattern line. Use the beamspot otherwise."};
            /** @brief Weight of precision hits in the score of a pattern, w.r.t trigger hits */
            DoubleProperty m_precisionWeight {this, "PrecisionWeight", 0.75, "Weight of precision hits in the pattern score of a pattern"};
            /** @brief number of eta hits per station at which the pattern score saturates */
            DoubleProperty m_hitScoreSaturation {this, "HitScoreSaturation", 10.0, "Number of eta hits per station at which the pattern score saturates"};
            /** @brief How strongly to penalize residual — higher = stricter quality requirement */
            DoubleProperty m_residualPenalty {this, "ResidualPenalty", 2.0, "How strongly to penalize residual in the pattern score — higher = stricter quality requirement"};
            /** @brief Saturation for phi bonus — beyond this many phi hits the bonus is maxed */
            DoubleProperty m_phiBonusSaturation {this, "PhiBonusSaturation", 4.0, "Saturation for phi bonus — beyond this many phi hits the bonus is maxed"};
            
            /** @brief Pointer to the actual global pattern finder */
            std::unique_ptr<FastReco::GlobalPatternFinder> m_globPatFinder{};

            /** -------------------------- Configuration options for segment fitter ------------------------- */
            
    };
}


#endif
