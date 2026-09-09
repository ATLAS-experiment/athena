/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONR4_MUONFASTRECOALGS_MUONGLOBALPATTERNFINDINGALG__H
#define MUONR4_MUONFASTRECOALGS_MUONGLOBALPATTERNFINDINGALG__H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/WriteHandleKey.h"

#include "MuonFastRecoHelpers/GlobalPatternFinder.h"

namespace MuonR4 {
    /// @brief Algorithm performing global pattern recognition. 
    /// 
    /// This algorithm performs global pattern recognition as the first step
    /// of the Phase-2 fast reconstruction stage. It builds global patterns
    /// of precision and non-precision hits using space-points created in 
    /// upstream algorithms. It first builds patterns in eta and then adds
    /// compatible phi-only hits to the patterns. The resulting patterns are
    /// written into store gate. 

    class MuonGlobalPatternFindingAlg : public AthReentrantAlgorithm {
        public:

            using AthReentrantAlgorithm::AthReentrantAlgorithm;
            virtual ~MuonGlobalPatternFindingAlg() = default;

            virtual StatusCode initialize() override;
            virtual StatusCode execute(const EventContext& ctx) const override;

        private:
            /** @brief Geometry context key */              
            ActsTrk::GeoContextReadKey_t m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};
            /** @brief Keys of SpacePoint containers to read */
            SG::ReadHandleKeyArray<SpacePointContainer> m_inSpacePoints{this, "InSpacePoints", {"MuonSpacePoints"}, "List of space point containers to read"};
            /** @brief Write handle key for the output global patterns */ 
            SG::WriteHandleKey<GlobalPatternContainer> m_outPatterns{this, "OutPatterns", "MuonR4GlobalPatterns"};

            /** @brief Toggle the utilization of MDT hits to build patterns */
            BooleanProperty m_useMdtHits {this, "UseMdtHits", true, "Activate the utilization of MDT hits to build patterns"};
            /** @brief Toggle the seeding from MDT hits */
            BooleanProperty m_seedFromMdt {this, "SeedFromMdt", false, "Activate the seeding from MDT hits"};
            /** @brief Activate the seeding from Inner station */
            BooleanProperty m_seedFromInner {this, "SeedFromInner", false, "Activate the seeding from Inner station"};
            /** @brief Size of theta window [rad] to search for compatible hits with a seed, tailored to the target pt cutoff */
            DoubleProperty m_thetaSearchWindow {this, "ThetaWindowSearch", 0.06, "Size of the search window in theta to link hits to a pattern"};
            /** @brief Number of standard deviations to consider for residual acceptance */
            DoubleProperty m_nResidualSigma {this, "NResidualSigma", 3., "Number of standard deviations to consider for residual acceptance"};
            /** @brief Residual uncertainty to consider the hit as low confidence */
            DoubleProperty m_lowConfidenceResSigma {this, "LowConfidenceResSigma", 50.0, "Residual uncertainty to consider the hit as low confidence"};
            /** @brief Number of standard deviations to consider for phi compatibility veto. The residual will be used to determine the acceptance. */
            DoubleProperty m_nPhiSigma {this, "NPhiSigma", 5., "Number of standard deviations to consider for phi acceptance"};
            /** @brief Requirement on trigger layers in the bending direction to accept a pattern  */
            UnsignedIntegerProperty m_minTriggerLayers {this, "MinBendingTriggerLayers", 2, "Minimum number of trigger layers in the bending direction required to accept a pattern"};
            /** @brief Requirement on precision layers in the bending direction to accept a pattern  */
            UnsignedIntegerProperty m_minPrecisionLayers {this, "MinBendingPrecisionLayers", 8, "Minimum number of precision layers in the bending direction required to accept a pattern"};
            /** @brief Minimum number of phi layers required to accept a pattern */
            UnsignedIntegerProperty m_minPhiLayers {this, "MinPhiLayers", 1, "Minimum number of phi layers required to accept a pattern"};
            /** @brief Minimum number of layers in a station to be considered a good station */
            UnsignedIntegerProperty m_minStationLayers {this, "MinStationLayers", 4, "Minimum number of layers in a station to be considered a good station"};
            /** @brief Quality cut on pattern'mean squared normalized residual. Set to a large value to disable the cut, e.g. 10. */
            DoubleProperty m_meanNormRes2Cut {this, "meanNormRes2Cut", 3.5, "Quality cut on pattern'mean squared normalized residual"};
            /** @brief Maximum number of attempts to build a pattern from hits already used in existing patterns */
            UnsignedIntegerProperty m_maxSeedAttempts {this, "MaxSeedAttempts", 3, " Maximum number of attempts to build a pattern from hits already used in existing patterns"};
            /** @brief Maximum number of missed candidate hits in different measurement layers during pattern building allowed for a pattern branch before it is discarded */
            UnsignedIntegerProperty m_maxMissLayersInStation {this, "MaxMissedLayerHits", 3, "Maximum number of missed candidate hits in different measurement layers during pattern building allowed for a pattern branch before it is discarded"};
            /** @brief Minimum distance [mm] between two hits for being used to compute a reliable pattern line. Use the beamspot otherwise. */
            DoubleProperty m_minHitDistance4Line {this, "MinHitDistance4Line", 200, "Minimum distance (in mm) between two hits for being used to compute a reliable pattern line. Use the beamspot otherwise."};
            /** @brief Beam spot radius */
            DoubleProperty m_beamSpotRadius{this, "BeamSpotRadius", 30.*Gaudi::Units::cm};
            /** @brief Beam spot length */
            DoubleProperty m_beamSpotLength{this, "BeamSpotLength", 2.*Gaudi::Units::m};

            /** @brief Handle to the visualization tool for global patterns */
            ToolHandle<MuonValR4::IFastRecoVisualizationTool> m_patVisionTool{this, "VisualizationTool", ""};
            /** @brief Handle to the MuonIdHelper service */        
            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

            /** @brief Pointer to the actual global pattern finder */
            std::unique_ptr<FastReco::GlobalPatternFinder> m_globPatFinder{};
    };
}

#endif