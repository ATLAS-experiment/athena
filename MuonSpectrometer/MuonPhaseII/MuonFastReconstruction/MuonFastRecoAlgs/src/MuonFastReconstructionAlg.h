/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONR4_MUONFASTRECOALGS_MUONFASTRECONSTRUCTIONALG__H
#define MUONR4_MUONFASTRECOALGS_MUONFASTRECONSTRUCTIONALG__H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/WriteHandleKey.h"

#include <MuonRecToolInterfacesR4/IPatternVisualizationTool.h>
#include <MuonRecToolInterfacesR4/IFastRecoVisualizationTool.h>
#include <MuonRecToolInterfacesR4/ITrackSeedingTool.h>

#include "MuonFastRecoHelpers/GlobalPatternFinder.h"
#include "MuonFastRecoHelpers/FastMuonSABuilder.h"
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
            /** @brief Write handle key for the output Fast Reco SA muons */
            SG::WriteHandleKey<xAOD::MuonContainer> m_outMuons{this, "OutMuons", "R4FastRecoSAMuons", "List of fast muon containers to write"};
            /** @brief Geometry context key */              
            ActsTrk::GeoContextReadKey_t m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};
            /** @brief Handle to the MuonIdHelper service */        
            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
            /** @brief Handle to the space point calibrator tool */
            ToolHandle<ISpacePointCalibrator> m_calibTool{this, "Calibrator", "" };
            /** @brief Handle to the visualization tool for global patterns */
            ToolHandle<MuonValR4::IFastRecoVisualizationTool> m_patVisionTool{this, "VisualizationTool", ""};
            /** @brief Handle to the visualization tool for segments */
            ToolHandle<MuonValR4::IPatternVisualizationTool> m_segVisionTool{this, "SegmentVisualizationTool", ""};
            /** @brief The track seeding tool to construct the seed candidates and to estimate the initial parameters */
            ToolHandle<ITrackSeedingTool> m_seedingTool{this, "SeedingTool", ""};

            /** ----------------- Configuration options for the global pattern finder ----------------- */
            /** @brief Toggle the utilization of MDT hits to build patterns */
            BooleanProperty m_useMdtHits {this, "UseMdtHits", true, "Activate the utilization of MDT hits to build patterns"};
            /** @brief Toggle the seeding from MDT hits */
            BooleanProperty m_seedFromMdt {this, "SeedFromMdt", false, "Activate the seeding from MDT hits"};
            /** @brief Activate the seeding from Inner station */
            BooleanProperty m_seedFromInner {this, "SeedFromInner", false, "Activate the seeding from Inner station"};
            /** @brief Size of theta window [rad] to search for compatible hits with a seed, tailored to the target pt cutoff */
            DoubleProperty m_thetaSearchWindow {this, "ThetaWindowSearch", 0.06, "Size of the search window in theta to link hits to a pattern"};
            /** @brief Effective isotropic position uncertainty [mm], including detector resolution and unmodelled effects */
            DoubleProperty m_baseResidualSigma  {this, "BaseResidualSigma", 35, "Effective isotropic position uncertainty [mm], including detector resolution and unmodelled effects"};
            /** @brief Maximum phi difference [rad] allowed between two hits belonging to the same pattern */
            DoubleProperty m_phiTolerance {this, "PhiTolerance", 0.05, "Maximum allowed phi difference between two hits to be considered compatible"};
            /** @brief Requirement on trigger layers in the bending direction to accept a pattern  */
            UnsignedIntegerProperty m_minTriggerLayers {this, "MinBendingTriggerLayers", 2, "Minimum number of trigger layers in the bending direction required to accept a pattern"};
            /** @brief Requirement on precision layers in the bending direction to accept a pattern  */
            UnsignedIntegerProperty m_minPrecisionLayers {this, "MinBendingPrecisionLayers", 8, "Minimum number of precision layers in the bending direction required to accept a pattern"};
            /** @brief Minimum number of phi layers required to accept a pattern */
            UnsignedIntegerProperty m_minPhiLayers {this, "MinPhiLayers", 1, "Minimum number of phi layers required to accept a pattern"};
            /** @brief Minimum number of layers in a station to be considered a good station */
            UnsignedIntegerProperty m_minStationLayers {this, "MinStationLayers", 5, "Minimum number of layers in a station to be considered a good station"};
            /** @brief Quality cut on pattern'mean squared normalized residual. Set to a large value to disable the cut, e.g. 10. */
            DoubleProperty m_meanNormRes2Cut {this, "meanNormRes2Cut", 0.2, "Quality cut on pattern'mean squared normalized residual"};
            /** @brief Maximum number of attempts to build a pattern from hits already used in existing patterns */
            UnsignedIntegerProperty m_maxSeedAttempts {this, "MaxSeedAttempts", 3, " Maximum number of attempts to build a pattern from hits already used in existing patterns"};
            /** @brief Maximum number of missed candidate hits in different measurement layers during pattern building allowed for a pattern branch before it is discarded */
            UnsignedIntegerProperty m_maxMissLayersInStation {this, "MaxMissedLayerHits", 3, "Maximum number of missed candidate hits in different measurement layers during pattern building allowed for a pattern branch before it is discarded"};
            /** @brief Minimum distance [mm] between two hits for being used to compute a reliable pattern line. Use the beamspot otherwise. */
            DoubleProperty m_minHitDistance4Line {this, "MinHitDistance4Line",40, "Minimum distance (in mm) between two hits for being used to compute a reliable pattern line. Use the beamspot otherwise."};
            
            /** -------------------------- Configuration options for segment fitter ------------------------- */
            /** @brief Toggle the recalibration of hits during the segment fit */
            BooleanProperty m_recalibInFit{this, "recalibInFit" , false};
            /// @brief Use the expliciit Hessian in the residual calculation
            BooleanProperty m_hessianResidual{this, "useHessianResidual", false};
            /** @brief Two mdt seeds are the same if their defining parameters match wihin */
            DoubleProperty m_seedHitChi2{this, "ResoSeedHitAssoc", 5. };
            /** @brief Toggle seed recalibration. The two seed circles are recalibrated using 
             *         the initial seed */
            BooleanProperty m_recalibSeed{this, "SeedRecalibrate", false};
            /** @brief Reduced chi2 defining a good segment, stopping the fit of other segments in the same station */
            DoubleProperty m_goodSegmentCut{this, "GoodSegmentCut", 2.};
            /** @brief Cut on the segment chi2 / nDoF to launch the outlier removal */
            DoubleProperty m_outlierRemovalCut{this, "OutlierRemoval", 3.};
            /** @brief Pull value for hit recovery */
            DoubleProperty m_recoveryPull{this, "RecoveryPull", 3.};
            /** @brief Minimum number of precision hits to accept the segment */
            UnsignedIntegerProperty m_precHitCut{this, "PrecHitCut" , 3};
            /** @brief Use the fast Mdt fitter where possible */
            BooleanProperty m_useFastFitter{this, "useFastFitter", false};
            /** @brief The fast fitter is treated as a pre fitter */
            BooleanProperty m_fastPreFitter{this, "useFastPreFitter", false};
            /** @brief Switch to try the full fit when the fast pre-fitter fails */
            BooleanProperty m_ignoreFailedPreFit{this, "ignoreFailedPreFit", false};
            /** @brief Tune the number of iterations */
            UnsignedIntegerProperty m_maxIter{this, "maxIterations", 50};



            /** @brief Pointer to the actual global pattern finder */
            std::unique_ptr<FastReco::GlobalPatternFinder> m_globPatFinder{};
            /** @brief Pointer to the FastMuonSABuilder */
            std::unique_ptr<FastReco::FastMuonSABuilder> m_saBuilder{};

            
            
    };
}


#endif
