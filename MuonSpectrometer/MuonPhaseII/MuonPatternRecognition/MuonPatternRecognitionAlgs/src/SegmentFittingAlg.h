/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONR4_MUONPATTERNRECOGNTIONALGS_SEGMENTFITTINGALG__H
#define MUONR4_MUONPATTERNRECOGNTIONALGS_SEGMENTFITTINGALG__H


#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadDecorHandleKeyArray.h"


#include "MuonSpacePoint/CalibratedSpacePoint.h"
#include "MuonRecToolInterfacesR4/ISpacePointCalibrator.h"
#include "MuonRecToolInterfacesR4/IPatternVisualizationTool.h"

#include "MuonPatternEvent/MuonPatternContainer.h"
#include "MuonPatternEvent/SegmentFitterEventData.h"
#include "MuonPatternEvent/MuonHoughDefs.h"

#include "MuonPatternHelpers/SegmentAmbiSolver.h"
#include "MuonPatternHelpers/SegmentLineFitter.h"

#include "xAODMuon/MuonSegmentContainer.h"


#include <set>


namespace MuonR4 {
    /// @brief Algorithm to handle segment fits  
    /// 
    /// This is currently a placeholder to test ideas! 
    class SegmentFittingAlg: public AthReentrantAlgorithm{
        public:
            using AthReentrantAlgorithm::AthReentrantAlgorithm;
            virtual ~SegmentFittingAlg();
            virtual StatusCode initialize() override;
            virtual StatusCode execute(const EventContext& ctx) const override;
        private:
            using Parameters = SegmentFit::Parameters;

            std::vector<std::unique_ptr<Segment>> fitSegmentSeed(const EventContext& ctx,
                                                                 const ActsTrk::GeometryContext& gctx,
                                                                 const SegmentSeed* seed) const;             
           
            void resolveAmbiguities(const ActsTrk::GeometryContext& gctx,
                                    std::vector<std::unique_ptr<Segment>>& segmentCandidates) const;

            /// ReadHandle of the seeds
            SG::ReadHandleKey<SegmentSeedContainer> m_seedKey{this, "ReadKey", "MuonHoughStationSegmentSeeds"};
            // write handle key for the output segment seeds 
            SG::WriteHandleKey<SegmentContainer> m_outSegments{this, "OutSegmentContainer", "R4MuonSegments"};
            // access to the ACTS geometry context 
            SG::ReadHandleKey<ActsTrk::GeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};
            /// IdHelperSvc
            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
            /// Handle to the space point calibrator
            ToolHandle<ISpacePointCalibrator> m_calibTool{this, "Calibrator", "" };
            /// Pattern visualization tool
            ToolHandle<MuonValR4::IPatternVisualizationTool> m_visionTool{this, "VisualizationTool", ""};

            Gaudi::Property<bool> m_doT0Fit{this, "fitSegmentT0", true};
            Gaudi::Property<bool> m_recalibInFit{this, "recalibInFit" , false};
            /// @brief Try first to fit the pattern parameters. Then proceed with the straw line tangents
            Gaudi::Property<bool> m_tryPatternPars{this, "tryPatternPars", false};
            /// @brief Use the expliciit Hessian in the residual calculation
            Gaudi::Property<bool> m_hessianResidual{this, "useHessianResidual", false};
            /// Add beamline constraint
            Gaudi::Property<bool> m_doBeamspotConstraint{this, "doBeamspotConstraint", false};
            Gaudi::Property<double> m_beamSpotR{this, "BeamSpotRadius", 30.* Gaudi::Units::cm};
            Gaudi::Property<double> m_beamSpotL{this, "BeamSpotLength", 2. * Gaudi::Units::m};

            
            /** @brief Two mdt seeds are the same if their defining parameters match wihin */
            Gaudi::Property<double> m_seedHitChi2{this, "ResoSeedHitAssoc", 5. };
            /** @brief Toggle seed recalibration. The two seed circles are recalibrated using 
             *         the initial seed */
            Gaudi::Property<bool> m_recalibSeed{this, "SeedRecalibrate", false};
            /** Cut on the segment chi2 / nDoF to launch the outlier removal */
            Gaudi::Property<double> m_outlierRemovalCut{this, "OutlierRemoval", 5.};
            Gaudi::Property<double> m_recoveryPull{this, "RecoveryPull", 5.};
            /** @brief Minimum number of precision hits to accept the segment */
            Gaudi::Property<unsigned> m_precHitCut{this, "PrecHitCut" , 3};
            /** @brief Use the fast Mdt fitter where possible */
            Gaudi::Property<bool> m_useFastFitter{this, "useFastFitter", true};
            /** @brief The fast fitter is treated as a pre fitter */
            Gaudi::Property<bool> m_fastPreFitter{this, "useFastPreFitter", false};
            /** @brief Tune the number of iterations */
            Gaudi::Property<unsigned> m_maxIter{this, "maxIterations", 50};
            /** @brief Pointer to the ambiguity reosolution */
            std::unique_ptr<SegmentFit::SegmentAmbiSolver> m_ambiSolver{};
            /** @brief Pointer to the actual segment fitter */
            std::unique_ptr<SegmentFit::SegmentLineFitter> m_fitter{};

    };
}


#endif
