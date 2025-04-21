/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONR4_MUONPATTERNRECOGNTIONALGS_SEGMENTFITTINGALG__H
#define MUONR4_MUONPATTERNRECOGNTIONALGS_SEGMENTFITTINGALG__H

#include "MuonPatternEvent/MuonPatternContainer.h"

#include "MuonSpacePoint/CalibratedSpacePoint.h"
#include "MuonRecToolInterfacesR4/ISpacePointCalibrator.h"
#include "MuonRecToolInterfacesR4/IPatternVisualizationTool.h"

#include "xAODMeasurementBase/UncalibratedMeasurementContainer.h"
#include "MuonPatternEvent/SegmentFitterEventData.h"
#include "MuonPatternEvent/MuonHoughDefs.h"
#include "MuonPatternHelpers/SegmentAmbiSolver.h"

#include "xAODMuon/MuonSegmentContainer.h"

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadDecorHandleKeyArray.h"

#include <set>


namespace MuonR4{
    /// @brief Algorithm to handle segment fits  
    /// 
    /// This is currently a placeholder to test ideas! 
    class SegmentFittingAlg: public AthReentrantAlgorithm{
        public:
            using AthReentrantAlgorithm::AthReentrantAlgorithm;
            virtual ~SegmentFittingAlg();
            virtual StatusCode initialize() override;
            virtual StatusCode execute(const EventContext& ctx) const override;

            using HitVec = SegmentFitResult::HitVec;

        private:
            using Parameters = SegmentFit::Parameters;
            /** @brief Executes the segment fit with start parameters. The returned fit result
             *         indicates whether the fit was a success and all relevant output parameters
             *  @brief ctx: Event context needed to access the calibration constants of the hits
             *  @brief gctx: Geometry context needed to place the segment globally within ATLAS to
             *               calculate the nominal time of arrival
             *  @brief startPars: Segment parameters preestimated from the SegmentSeed 
             *                    (either  hough pattern or two drift circle seed)
             *  @brief calibHits: Vector of strip & mdt hits to consider for the fit */
            SegmentFitResult fitSegmentHits(const EventContext& ctx,
                                            const ActsGeometryContext& gctx,
                                            const Parameters& startPars,
                                            SegmentFitResult::HitVec&& calibHits) const;


            std::vector<std::unique_ptr<Segment>> fitSegmentSeed(const EventContext& ctx,
                                                                 const ActsGeometryContext& gctx,
                                                                 const SegmentSeed* seed) const;             
            /** @brief Spot hits with large discrepancy from the estimated parameters and remove them
             *         from the list.
             *  @param ctx: EventContext needed for hit calibration
             *  @param gctx: Geometry context needed if the beamspot constaint will be packed onto the segment
             *  @param fitResult: Data structure carrying the fit parameters & calibrated hits */
            bool removeOutliers(const EventContext& ctx,
                                const ActsGeometryContext& gctx,
                                const SegmentSeed& seed,
                                SegmentFitResult& fitResult) const;

            /** @brief Recovery of missed hits. Hits in the space point bucket  that are maximally
             *         <RecoveryPull> away from the fitted segment are put onto the segment candidate
             *         and the candidate is refitted. If the refitted candidate has a chi2/nDoF < <OutlierRemoval>
             *         the canidate is automatically choosen otherwise, its chi needs to be better. 
             *  @param ctx: EventContext needed for calibration of the hits
             *  @param gctx: Geometry context needed if the beamspot constaint will be packed onto the segment
             *  @param seed: Segment seed from the pattern recognition to access the underlying bucket
             *  @param toRecover: Fit result with parameters & hits to recover.  */
            bool plugHoles(const EventContext& ctx,
                           const ActsGeometryContext& gctx,
                           const SegmentSeed& seed,
                           SegmentFitResult& toRecover) const;
            /** @brief Removes all hits from the segment which are obvious outliers. E.g. tubes 
             *         which cannot be crossed by the segment. 
             *  @param candidate: Reference of the segment candidate to prune. */
            void eraseWrongHits(SegmentFitResult& candidate) const;            
            /** @brief Converts the fit result into a segment object
             *  @param locToGlobTrf: Local to global transform to translate the segment parameters into
             *                       global parameters
             *  @param parentSeed: Segment seed from which the segment was built
             *  @param toConvert: Fitted segment that needs conversion */
            static std::unique_ptr<Segment> convertToSegment(const Amg::Transform3D& locToGlobTrf, 
                                                             const SegmentSeed* parentSeed,
                                                             SegmentFitResult&& toConvert);
           
            void resolveAmbiguities(const ActsGeometryContext& gctx,
                                    std::vector<std::unique_ptr<Segment>>& segmentCandidates) const;

            /// ReadHandle of the seeds
            SG::ReadHandleKey<SegmentSeedContainer> m_seedKey{this, "ReadKey", "MuonHoughStationSegmentSeeds"};
            // write handle key for the output segment seeds 
            SG::WriteHandleKey<SegmentContainer> m_outSegments{this, "MuonSegmentContainer", "R4MuonSegments"};
            // access to the ACTS geometry context 
            SG::ReadHandleKey<ActsGeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};
            /// IdHelperSvc
            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
            /// Handle to the space point calibrator
            ToolHandle<ISpacePointCalibrator> m_calibTool{this, "Calibrator", "" };
            /// Pattern visualization tool
            ToolHandle<MuonValR4::IPatternVisualizationTool> m_visionTool{this, "VisualizationTool", ""};

            Gaudi::Property<bool> m_doT0Fit{this, "fitSegmentT0", true};
            Gaudi::Property<bool> m_recalibInFit{this, "recalibInFit" , false};
            /// Add beamline constraint
            Gaudi::Property<bool> m_doBeamspotConstraint{this, "doBeamspotConstraint", false};
            Gaudi::Property<double> m_beamSpotR{this, "BeamSpotRadius", 30.* Gaudi::Units::cm};
            Gaudi::Property<double> m_beamSpotL{this, "BeamSpotLength", 20. * Gaudi::Units::m};

            
            /** @brief Two mdt seeds are the same if their defining parameters match wihin */
            Gaudi::Property<double> m_seedHitChi2{this, "ResoSeedHitAssoc", 5. };
            /** @brief Toggle seed recalibration. The two seed circles are recalibrated using 
             *         the initial seed */
            Gaudi::Property<bool> m_recalibSeed{this, "SeedRecalibrate", false};
            /** @brief Toggle seed refit. The segment seed is fastly refitted 
             *         using the collected seed drift circles */
            Gaudi::Property<bool> m_refineSeed{this, "SeedRefine", false};
            /** Cut on the segment chi2 / nDoF to launch the outlier removal */
            Gaudi::Property<double> m_outlierRemovalCut{this, "OutlierRemoval", 5.};
            Gaudi::Property<double> m_recoveryPull{this, "RecoveryPull", 5.};
            /** @brief Minimum number of precision hits to accept the segment */
            Gaudi::Property<unsigned> m_precHitCut{this, "PrecHitCut" , 3};

            std::unique_ptr<SegmentAmbiSolver> m_ambiSolver{};

    };
}


#endif
