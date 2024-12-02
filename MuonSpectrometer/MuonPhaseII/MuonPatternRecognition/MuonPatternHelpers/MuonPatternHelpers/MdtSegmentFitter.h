/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONPATTERNHELPERS_MDTSEGMENTFITTER_H
#define MUONPATTERNHELPERS_MDTSEGMENTFITTER_H

#include <GeoPrimitives/GeoPrimitives.h>

#include <AthenaBaseComps/AthMessaging.h>
#include <MuonSpacePoint/CalibratedSpacePoint.h>
#include <MuonPatternEvent/SegmentSeed.h>
#include <MuonPatternEvent/SegmentFitterEventData.h>
#include <MuonPatternHelpers/MatrixUtils.h>
#include <GaudiKernel/SystemOfUnits.h>


namespace MuonR4{
    class ISpacePointCalibrator;
    class MdtSegmentFitter: public AthMessaging{
        public:
            using ParamDefs = SegmentFit::ParamDefs;
            using Parameters = SegmentFit::Parameters;
            using HitType = std::unique_ptr<CalibratedSpacePoint>;
            using HitVec = std::vector<HitType>;

            struct Config{
                /** @brief How many calls shall be executed */
                unsigned int nMaxCalls{100};
                /** @brief Gradient cut off */
                double tolerance{1.e-7};
                /** @brief Switch toggling whether the T0 shall be fitted or not*/
                bool doTimeFit{true};
                /** @brief Switch toggling whether the calibrator shall be called at each iteration */
                bool reCalibrate{false};
                /** @brief Switch toggling whether the second order derivative shall be included */
                bool useSecOrderDeriv{false};
                /** @brief Abort the fit as soon as more than n parameters leave the fit range*/
                unsigned int nParsOutOfBounds{1};
                /** @brief Pointer to the calibrator tool*/
                const ISpacePointCalibrator* calibrator{nullptr};
                /** @brief How many iterations with changes below tolerance */
                unsigned int noMoveIter{2};
                /** @brief Allowed parameter ranges */
                using RangeArray = std::array<std::array<double,2>, SegmentFit::toInt(ParamDefs::nPars)>;
                /** @brief Function that returns a set of predefined ranges for testing */
                static RangeArray defaultRanges();

                RangeArray ranges{defaultRanges()};
                
            };
            /** @brief Store the partial derivative of the line w.r.t. the fit parameters
             *         at the slots x0,y0 the derivatives of the position are saved
             *         while at the slot theta & phi, the deriviatives of the direction vector are saved */
            struct LineWithPartials{
                /** @brief Free parameters of the line (x0,y0,theta,phi) */
                static constexpr unsigned nPars{4};
                /** @brief segment position */
                Amg::Vector3D pos{Amg::Vector3D::Zero()};
                /** @brief Segment direction  */
                Amg::Vector3D dir{Amg::Vector3D::Zero()};
                /** @brief First order derivatives */
                std::array<Amg::Vector3D, nPars> gradient{make_array<Amg::Vector3D, nPars>(Amg::Vector3D::Zero())};
                /** @brief Second order derivatives */
                std::array<Amg::Vector3D, sumUp(nPars)> hessian{make_array<Amg::Vector3D, sumUp(nPars)>(Amg::Vector3D::Zero())};
            };
            /** @brief Helper struct carrying the residual with its derivatives */
            struct ResidualWithPartials{
                /** @brief Number of parameters */
                static constexpr unsigned nPars{toInt(ParamDefs::nPars)};
                /** @brief Flag whether the the residuals w.r.t phi shall be evaluated */
                bool evalPhiPars{true};
                /** @brief Vector carrying the residual */
                Amg::Vector3D residual{Amg::Vector3D::Zero()};
                /** @brief First order derivatives */
                std::array<Amg::Vector3D, nPars> gradient{make_array<Amg::Vector3D, nPars>(Amg::Vector3D::Zero())};
                /** @brief Second order derivatives */
                std::array<Amg::Vector3D, sumUp(nPars)> hessian{make_array<Amg::Vector3D, sumUp(nPars)>(Amg::Vector3D::Zero())};
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
            /** @brief Recalibrate the measurements participating in the fit & shift them into the
             *         centre-of gravity frame. The parsed centerOfGravity position is updated to be 
             *         the average measurement position weighted by the inverse covariance. Returns true
             *         if there're enough degrees of freedom left to fit
             *  @param ctx: EventContext to fetch the calibration constants
             *  @param fitResult: Current fit state
             *  @param centerOfGravity: Vector to save the center of gravity position later on.
             *  @param invCov: Vector to cache the inverse covariances of the problem. */ 
            bool recalibrate(const EventContext& ctx,
                             SegmentFitResult& fitResult) const;
            /** @brief Brief updates the hit summary from the contributing hits. Returns fals if there're too little valid hits.
             *  @param fitResult: Reference to the container containing all the measurements */
            bool updateHitSummary(SegmentFitResult& fitResult) const;
        public:
            /** @brief Updates the line parameters together with its first &
             *         second order partial derivatives.
             *  @param fitPars: Set of segment parameters in the current iteration
             *  @param line: Reference to the line object to be updated */            
            static void updateLinePartials(const Parameters& fitPars, 
                                           LineWithPartials& line);
            /** @brief Calculates the residuals together with the corresponding derivatives for a drift-circle measurement.
             *         The residual is calculated by projecting the hit into the plane perpendicular to the wire. 
             *         If the hit is a twin-hit the distance along the wire is taken into account as well.
             *  @param line: Current segment line together with the first & second order derivatives
             *  @param spacePoint: Reference to the measurement to calculate the residual to
             *  @param residual: Output residual object to save the actual residual and the derivatives */
            void calculateWireResiduals(const LineWithPartials& line,
                                        const CalibratedSpacePoint& spacePoint,
                                        ResidualWithPartials& residual) const;
            /** @brief Calculates the residual together with hte correspdonding derivatives for strip measurements.
             *         The residual is calculated by a straight line propagation of the segment onto the plane &
             *         then taking the absolute disances in the [precision] & [non-precision direction] 
             *  @param line: Current segment line together with the first & second order derivatives
             *  @param spacePoint: Reference to the measurement to calculate the residual to
             *  @param residual: Output residual object to save the actual residual and the derivatives */

            void calculateStripResiduals(const LineWithPartials& line,
                                        const CalibratedSpacePoint& spacePoint,
                                        ResidualWithPartials& residual) const;

            /** @brief Calculates the partial derivative of the intersection point between  the segment line and the 
             *         measurement's strip plane.
             *  @param normal: Normal vector on the measurment's plane pointing in positive z direction
             *  @param offset: Offset describing the refrence point of the normal vector
             *  @param line: Line describing the segment together with its derivatives
             *  @param fitPar: Parameter to which the derivative shall be calculated */
            static Amg::Vector3D partialPlaneIntersect(const Amg::Vector3D& normal,
                                                       const double offset, 
                                                       const LineWithPartials& line,
                                                       const ParamDefs fitPar);
        private:   
            /** @brief Update the signs of the measurement */
            void updateDriftSigns(const Amg::Vector3D& segPos, const Amg::Vector3D& segDir, 
                                  SegmentFitResult& fitRes)const;
            /** @brief Updates the chi2, its Gradient & Hessian from the measurement residual. Depending on whether 
             *          the time needs to be taken into account or not the template parameter nDim is either 3 or 2, respectively.
             *  @param fitMeas: Residual of the prediction from the measurement together with its derivatives
             *  @param measCovariance: Covariance matrix of the measurement
             *  @param gradient: Reference to the gradient of the chi2 which is updated by the function
             *  @param hessian: Reference to the Hessian matrix of the chi2 which is updated by the function
             *  @param chi2: Reference to the chi2 value itself which is updated by the function
             *  @param startPar: Number of the maximum fit parameter to consider */
            using MeasCov_t = CalibratedSpacePoint::Covariance_t;
            void updateDerivatives(const ResidualWithPartials& fitMeas,
                                   const MeasCov_t& measCovariance,
                                   AmgVector(5)& gradient,
                                   AmgSymMatrix(5)& hessian,
                                   double& chi2,
                                   int startPar) const;
            

            enum class UpdateStatus{
                allOkay = 0,
                outOfBounds = 1,
                noChange = 2,
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