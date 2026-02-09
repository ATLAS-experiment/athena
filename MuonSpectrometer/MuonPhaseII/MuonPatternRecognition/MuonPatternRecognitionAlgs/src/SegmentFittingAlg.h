/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
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
#include "MuonPatternHelpers/MdtSegmentSeedGenerator.h"


namespace MuonR4 {
    /**  @brief The SegmentFittingAlg fits straight lines to the 
     *          Mdt/Rpc/Tgc hits associated with the SegmentSeedPatterns.
     *          The pattern parameters in the bending direction are refined 
     *          by constructing the tangent to two Mdt measurements and then
     *          by associating other measurements to the seed. If the procedure
     *          succeeds, the hits are passed through the straight line procedure.
     *          At the end of the fit, ambiguities amongst the segment candidates 
     *          are removed based on the degrees of freedom and the chi2 fit quality */
    class SegmentFittingAlg: public AthReentrantAlgorithm {
        public:
            using AthReentrantAlgorithm::AthReentrantAlgorithm;
            virtual ~SegmentFittingAlg();
            virtual StatusCode initialize() override;
            virtual StatusCode execute(const EventContext& ctx) const override;
        private:
            using SegmentVec_t = std::vector<std::unique_ptr<Segment>>;

            using Parameters = SegmentFit::Parameters;
            /** @brief Fit the hits from the pattern seed to segment candidates. Tangent lines
             *         to a pair of drift circles are constructed. The remaining hits are associated
             *         based on their chi2 compability. Good seeds are then fitted to segments including
             *         an outlier rejection and hole recovery procedure
             * @param ctx: Event context to access the conditions data needed for calibration
             * @param gctx: Geometry context to access the transforms of the particular reference
             *              surfaces.
             * @param seed: The segment seed from which the parameters in non-bending direction &
             *              the associated hits are taken */
            SegmentVec_t fitSegmentSeed(const EventContext& ctx,
                                        const ActsTrk::GeometryContext& gctx,
                                        const SegmentSeed* seed) const;             
            /** @brief Resolve the ambiguity amongst the segment candidates within a spectrometer
                       sector. Segments are rejected if they share hits with other segments and if 
                       they have poorer quality.
              @param gctx: GeometryContext to compare the local parameters
              @param segmentCandidates: The list of segments to be resolved */
            void resolveAmbiguities(const ActsTrk::GeometryContext& gctx,
                                    SegmentVec_t& segmentCandidates) const;

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
            /** @brief Cut on the number of hits per layer to use the layer for seeding */
            Gaudi::Property<unsigned> m_busyLayerLimit{this, "busyLayerLimit",  2};
            /** @brief Pointer to the ambiguity reosolution */
            std::unique_ptr<SegmentFit::SegmentAmbiSolver> m_ambiSolver{};
            /** @brief Pointer to the actual segment fitter */
            std::unique_ptr<SegmentFit::SegmentLineFitter> m_fitter{};
            /** @brief Pointer to the L-R segment seeder */
            std::unique_ptr<SegmentFit::MdtSegmentSeedGenerator> m_seeder{};
            /** @brief Pointer to the L-R segment seeder used for the BEE
             *         chambers -> increased hit occupancy */
            std::unique_ptr<SegmentFit::MdtSegmentSeedGenerator> m_seederBEE{};
    };
}


#endif
