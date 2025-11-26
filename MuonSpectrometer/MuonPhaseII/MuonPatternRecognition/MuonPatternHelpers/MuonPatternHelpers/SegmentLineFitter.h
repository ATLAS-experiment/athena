/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONPATTERNHELPERS_MDTSEGMENTFITTER_H
#define MUONPATTERNHELPERS_MDTSEGMENTFITTER_H

#include <GeoPrimitives/GeoPrimitives.h>
///
#include <AthenaBaseComps/AthMessaging.h>

#include <MuonSpacePoint/CalibratedSpacePoint.h>
#include <MuonRecToolInterfacesR4/ISpacePointCalibrator.h>
#include <MuonRecToolInterfacesR4/IPatternVisualizationTool.h>


#include <MuonPatternEvent/Segment.h>
#include <MuonPatternEvent/SegmentSeed.h>


#include <Acts/Seeding/CompositeSpacePointLineFitter.hpp>

namespace MuonR4::SegmentFit {
    /** @brief The SegmentLineFitter is a standalone module to fit a straight line to calibrated 
     *          muon space points. The `CompositeSpacePointLineFitter` from the ACTS toolkit is 
     *          used to perform the actual fit to the measurements. The SegmentLineFitter is a wrapper
     *          class taking care of relaunching fits of poor quality but with cleaned measurements and 
     *          also to put back meaurements on the line that have been missed by the initial line fit. */
    class SegmentLineFitter: public AthMessaging {
        public:
            /** @brief Abrivation of the actual line fitter */
            using Fitter_t = Acts::Experimental::CompositeSpacePointLineFitter;
            /** @brief Abrivation of the fitted line parameters */
            using LinePar_t = Fitter_t::ParamVec_t; 
            /** @brief Abrivation of the space point type to use */
            using Hit_t = std::unique_ptr<CalibratedSpacePoint>;
            /** @brief Collection of space points */
            using HitVec_t = std::vector<Hit_t>;
            /** @brief Abrivation of the fit parameters  */
            using FitPars_t = Fitter_t::FitParameters;
            /** @brief Abrivation of the fit result */
            using Result_t = Fitter_t::FitResult<HitVec_t>;
            /** @brief Abrivation of the fit options */
            using FitOpts_t = Fitter_t::FitOptions<HitVec_t, ISpacePointCalibrator>;
            /** @brief Abrivation of the hit selector to choose valid hits */
            using Selector_t = Fitter_t::Selector_t<CalibratedSpacePoint>;
            /** @brief Abrivation of the fit state flag */
            using HitState = CalibratedSpacePoint::State;
            /** @brief Configuration object of the ATLAS implementation */
            struct ConfigSwitches{
                /** @brief Pointer to the calibrator */
                const ISpacePointCalibrator* calibrator{nullptr};
                /** @brief Pointer to the visualization tool */
                const MuonValR4::IPatternVisualizationTool* visionTool{nullptr};
                /** @brief Pointer to the idHelperSvc */
                const Muon::IMuonIdHelperSvc* idHelperSvc{nullptr};
                /** @brief Switch to insert a beamspot constraint if possible */
                bool doBeamSpot{true};
                /** @brief Parameters of the beamspot measurement */
                double beamSpotRadius{30.*Gaudi::Units::cm};
                double beamSpotLength{2.*Gaudi::Units::m};
                /** @brief Cut on the segment chi2 / nDoF to launch the outlier removal */
                double outlierRemovalCut{5.};
                /** @brief Maximum pull on a measurement to add it back on the line */
                double recoveryPull{5.};
                /** @brief Minimum number of precision hits */
                unsigned nPrecHitCut{3u};
                /** @brief Maximum trials to recover outliers */
                unsigned nRecoveryLoops{10u};
            };
            /** @brief Full configuration object */           
            struct Config : public Fitter_t::Config,
                            public ConfigSwitches {
                /** @brief Function that returns a set of predefined ranges for testing */
                static RangeArray defaultRanges();
                /** @brief Standard constructor */
                Config() {
                    ranges = defaultRanges();
                }

            };
            /** @brief Standard constructor
             *  @param name: Name to be printed in the messaging
             *  @param config: Fit configuration parameters */
            SegmentLineFitter(const std::string& name,
                             Config&& config);

            /** @brief Fit a set of measurements to a straight segment line. Badish
             *         initial fits are cleaned and then holes are put filled back
             *          Returns a nullptr if the fit failed
             * @param ctx: EventContext to access the calibration constants
             * @param parent: Pointer to the seed from which the hits to fit are taken.
             *                The seed gives also access to the parent bucket to recover
             *                lost hits
             * @param startPars: List of parameters serving as an initial guess
             * @param localToGlobal: Transform to align the segment's station inside ATLAS
             *                       (Mainly neede if the time is fit)
             * @param calibHits: List of hits that will be fitted */
            std::unique_ptr<Segment> fitSegment(const EventContext& ctx,
                                                const SegmentSeed* parent,
                                                const LinePar_t& startPars,
                                                const Amg::Transform3D& localToGlobal,
                                                HitVec_t&& calibHits) const;

        private:
            /** @brief Actual implementation of the straight line fit  */
            Fitter_t m_fitter;
            /** @brief Configuration switches of the ATLAS fitter implementation */
            ConfigSwitches m_cfg{};
            /** @brief Selector to identify the valid hits */
            Selector_t m_goodHitSel{};
            /** @brief Moves the segment to the average x0 position, if 
             *         the segment does not contain any measurement. */
            void centerAlongWire(Result_t& fitResult) const;
            /** @brief Calls the underlying line fitter to determine the segment parameters
             *  @param cctx: Calibration context to fetch later the measurement's calib constants
             *               from StoreGate (It's a packed EventContext*)
             * @param startPars: Initial line parameters guess
             * @param localToGlobal: Transform to align the segment's station inside ATLAS
             *                       (Mainly neede if the time is fit)
             * @param calibHits: List of hits that will be fitted */
            Result_t callLineFit(const Acts::CalibrationContext& cctx,
                                 const Parameters& startPars,
                                 const Amg::Transform3D& localToGlobal,
                                 HitVec_t&& calibHits) const;
            /** @brief Cleans the fitted segment from the most outlier hit and then
             *         attempts to refit the segment. The outlier removal is not run
             *         if the segment has already a chi2 / nDoF better than <outlierRemovalCut>.
             *         Returns false if the recovery lead to the destruction of all nDoF
             *  @param cctx: Calibration context to fetch later the measurement's calib constants
             *               from StoreGate (It's a packed EventContext*)
             *  @param seed: Parent seed from which the segment fit is actually triggered
             *               The seed is mainly used for visualization purposes
             *  @param localToGlobal: Transform to align the spectrometer sector within ATLAS
             *                         mainly used for the t0 fit
             *  @param fitResult: Previously achieved fit result to be checked. The measurements
             *                    on the result and the paramters are updated accordingly */
            bool removeOutliers(const Acts::CalibrationContext& cctx,
                                const SegmentSeed& seed,
                                const Amg::Transform3D& localToGlobal,
                                Result_t& fitResult) const;
            /** @brief Recovery of missed hits. Hits in the space point bucket  that are maximally
             *         <RecoveryPull> away from the fitted segment are put onto the segment candidate
             *         and the candidate is refitted. If the refitted candidate has a chi2/nDoF < <OutlierRemoval>
             *         the canidate is automatically choosen otherwise, its chi needs to be better. 
             *  @param cctx: Calibration context to fetch later the measurement's calib constants
             *               from StoreGate (It's a packed EventContext*)
             *  @param seed: Parent seed from which the segment fit is actually triggered
             *               The seed is mainly used for visualization purposes
             *  @param localToGlobal: Transform to align the spectrometer sector within ATLAS
             *                         mainly used for the t0 fit
             *  @param fitResult: Previously achieved fit result to be checked. The measurements
             *                    on the result and the paramters are updated accordingly */
            bool plugHoles(const Acts::CalibrationContext& cctx,
                           const SegmentSeed& seed,
                           const Amg::Transform3D& localToGlobal,
                           Result_t& toRecover) const;
            /** @brief Removes all hits from the segment which are obvious outliers. E.g. tubes 
             *         which cannot be crossed by the segment. 
             *  @param candidate: Reference of the segment candidate to prune. */
            void eraseWrongHits(Result_t& candidate) const;
            /** @brief Marks duplicate hits on a strip layer as outliers to avoid
             *         competing contributions from the same layers in the fit. Hits
             *         on the same layer are sorted by their chi2 and the worse ones
             *         are rejected if they don't provide additional information
             *  @param linePos: Position of the latest segment line 
             *  @param lineDir: Direction of the latest segment line
             *  @param hits: List of hit measurements to clean*/
            void cleanStripLayers(const Amg::Vector3D& linePos,
                                  const Amg::Vector3D& lineDir,
                                  HitVec_t& hits) const;
            /** @brief Converts the fit result into a segment object
             *  @param locToGlobTrf: Local to global transform to translate the segment parameters into
             *                       global parameters
             *  @param parentSeed: Segment seed from which the segment was built
             *  @param toConvert: Fitted segment that needs conversion */
            std::unique_ptr<Segment> convertToSegment(const Amg::Transform3D& locToGlobTrf, 
                                                      const SegmentSeed* parentSeed,
                                                      Result_t&& toConvert) const;

    };  
}


#endif