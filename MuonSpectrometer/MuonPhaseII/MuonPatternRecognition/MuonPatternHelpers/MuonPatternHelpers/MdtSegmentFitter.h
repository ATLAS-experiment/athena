/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONPATTERNHELPERS_MDTSEGMENTFITTER_H
#define MUONPATTERNHELPERS_MDTSEGMENTFITTER_H

#include <GeoPrimitives/GeoPrimitives.h>
///
#include <AthenaBaseComps/AthMessaging.h>

#include <MuonPatternEvent/HoughEventData.h>
#include <MuonPatternEvent/SegmentSeed.h>
#include <MuonPatternEvent/SegmentFitterEventData.h>

namespace MuonR4{
    class ISpacePointCalibrator;
    class CalibratedSpacePoint;
}

namespace MuonR4::SegmentFit{
    class MdtSegmentFitter: public AthMessaging{
        public:
            using HitType = std::unique_ptr<CalibratedSpacePoint>;
            using HitVec = std::vector<HitType>;

            struct Config{
                /** @brief How many calls shall be executed */
                unsigned int nMaxCalls{100};
                /** @brief Gradient cut off to declare a fit as converged */
                double tolerance{1.e-7};
                /** @brief Cutoff on the angular parameter update to keep
                 *         the calibration on */
                double angularCalibCutOff{1.*Gaudi::Units::mrad};
                /** @brief Cutoff on the spatial paramater update to keep
                 *         calibration on */
                double spatialCalibCuttOff{0.1*Gaudi::Units::mm};
                /** @brief Switch toggling whether the T0 shall be fitted or not*/
                bool doTimeFit{true};
                /** @brief Switch toggling whether the calibrator shall be called at each iteration */
                bool reCalibrate{false};
                /** @brief Switch toggling whether the second order derivative shall be included */
                bool useSecOrderDeriv{false};
                /** @brief Use the fast Straw line fitter if there are only 
                 *         Mdt measurements to fit */
                bool useFastFit{true};
                /** @brief Abort the fit as soon as more than n parameters leave the fit range*/
                unsigned int nParsOutOfBounds{1};
                /** @brief Pointer to the calibrator tool*/
                const ISpacePointCalibrator* calibrator{nullptr};
                /** @brief How many iterations with changes below tolerance */
                unsigned int noMoveIter{2};
                /** @brief Allowed parameter ranges */
                using RangeArray = std::array<std::array<double,2>, Acts::toUnderlying(ParamDefs::nPars)>;
                /** @brief Function that returns a set of predefined ranges for testing */
                static RangeArray defaultRanges();

                RangeArray ranges{defaultRanges()};
            };
            /** @brief Standard constructor
             *  @param name: Name to be printed in the messaging
             *  @param config: Fit configuration parameters */
            MdtSegmentFitter(const std::string& name,
                             Config&& config);
            
            SegmentFitResult fitSegment(const EventContext& ctx,
                                        HitVec&& calibHits,
                                        const Parameters& startPars,
                                        const Amg::Transform3D& localToGlobal) const;
        private:
            Config m_cfg{};
            /** @brief Recalibrate the measurements participating the fit based on the best straight-line knowledge 
             *  @param ctx: EventContext to fetch the calibration constants
             *  @param line: Reference to the best known segment line
             *  @param fitResult: Current fit state which measurements are calibrated */ 
            bool recalibrate(const EventContext& ctx,
                             const Line_t& segmentLine,
                             SegmentFitResult& fitResult) const;
            /** @brief Updates the hit summary from the contributing hits. Returns fals if there're too little valid hits.
             *  @param fitResult: Reference to the container containing all the measurements */
            bool updateHitSummary(SegmentFitResult& fitResult) const;
            /** @brief Moves the segment to the average x0 position, if 
             *         the segment does not contain any measurement. */
            void centerAlongWire(SegmentFitResult& fitResult) const;
            /** @brief Update the signs of the measurement */
            void updateDriftSigns(const Line_t& segmentLine, SegmentFitResult& fitRes)const;
            /** @brief Status update of the parameter update */
            enum class UpdateStatus{
                allOkay = 0,     /// Ordinary parameter update keep calibration switched on
                smallStep = 1,   /// Step size is small enough that calibration can be skipped
                outOfBounds = 2, /// The parameters drifted out of bounds -> Abortion
                noChange = 3,    /// Tiny parameter change
            };
            /** @brief Update step of the segment parameters using the Hessian and the gradient. If the Hessian is definite,
             *         the currentParameters are updated according to
             *                   x_{n+1} = x_{n} - H_{n}^{1} * grad(x_{n})
             *          Otherwise, the method of steepest descent is attempted.
             *  @param currentPars:  Best segment estimator parameters
             *  @param previousPars: Segment estimator parameters from the last iteration
             *  @param currGrad: Gradient of the chi2 from this iteration
             *  @param prevGrad: Gradient of the chi2 from the previous iteration
             *  @param hessian: Hessian estimator
             */            
            template <unsigned int nDim>
                UpdateStatus updateParameters(Parameters& currentPars,
                                              Parameters& previousPars,
                                              Parameters& currGrad,
                                              Parameters& prevGrad,
                                              const AmgSymMatrix(5)& hessian) const;
            
            template <unsigned int nDim>
                void blockCovariance(const AmgSymMatrix(5)& hessian,                                    
                                     SegmentFit::Covariance&  covariance) const;


    };
}


#endif