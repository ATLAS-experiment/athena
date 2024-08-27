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
#include <GaudiKernel/SystemOfUnits.h>


namespace MuonR4{
    class ISpacePointCalibrator;
    class MdtSegmentFitter: public AthMessaging{
        public:
            using AxisDefs = SegmentFit::AxisDefs;
            using Parameters = SegmentFit::Parameters;
            using HitType = std::unique_ptr<CalibratedSpacePoint>;
            using HitVec = std::vector<HitType>;

            struct Config{
                /** @brief How many calls shall be executed */
                unsigned int nMaxCalls{100};
                /** @brief Gradient cut off */
                double tolerance{1.e-5};
                /** @brief Switch toggling whether the T0 shall be fitted or not*/
                bool doTimeFit{true};
                /** @brief Abort the fit as soon as more than n parameters leave the fit range*/
                unsigned int nParsOutOfBounds{1};
                /** @brief Pointer to the calibrator tool*/
                const ISpacePointCalibrator* calibrator{nullptr};
                /** @brief Allowed parameter ranges */
                using RangeArray = std::array<std::array<double,2>, SegmentFit::toInt(AxisDefs::nPars)>;
                /** @brief Function that returns a set of predefined ranges for testing */
                static RangeArray defaultRanges();

                RangeArray ranges{defaultRanges()};
                
            };


            MdtSegmentFitter(const std::string& name,
                             Config&& config);
            
            SegmentFitResult fitSegment(const EventContext& ctx,
                                        HitVec&& calibHits,
                                        const Parameters& startPars,
                                        const Amg::Transform3D& localToGlobal) const;
        private:
            Config m_cfg{};

            /** @brief Store the partial derivative of the line w.r.t. the fit parameters
             *         at the slots x0,y0 the derivatives of the position are saved
             *         while at the slot tanPhi, tanTheta, the deriviatives of the direction vector are saved */        
            using LinePartialArray = std::array<Amg::Vector3D, toInt(AxisDefs::nPars)>;
            /** @brief Updates the partial derivaitves of the line w.r.t the fit parameters 
             *  @param fitPars: Set of segment parameters in the current iteration
             *  @param updateNonBending: If set to true, the derivatives for tanPhi / x0 are calculated
             *  @param linePartials: Storage to safe the line partials to */            
            void updateLinePartials(const Parameters& fitPars, 
                                    const bool updateNonBending,
                                    LinePartialArray& linePartials) const;

            /** @brief Calculates the partial derivative of the point of closest approach
             *         to the meausrement's wire w.r.t. the fit parameter
             *  @param sp: Calibrated muon space point (Usually a Mdt)
             *  @param segPos: Position of the segment line at chamber (z=0)
             *  @param segDir: Direction of the segment line
             *  @param linePartials: Array with the partial derivatives of segPos, segDir
             *                        w.r.t. the fit parameters
             *  @param fitPar: Parameter to which the derivative shall be calculated */
            static Amg::Vector3D partialClosestApproach(const MuonR4::CalibratedSpacePoint& sp,
                                                        const Amg::Vector3D& segPos,
                                                        const Amg::Vector3D& segDir,
                                                        const LinePartialArray& linePartials,
                                                        const AxisDefs fitPar);


            /** @brief Calculates the partial derivative of the intersection point between  the segment line and the 
             *         measurement's strip plane.
             *  @param normal: Normal vector on the measurment's plane pointing in positive z direction
             *  @param offset: Offset describing the refrence point of the normal vector
             *  @param segPos: Position of the segment line at chamber (z=0)
             *  @param segDir: Direction of the segment line
             *  @param linePartials: Array with the partial derivatives of segPos, segDir
             *                        w.r.t. the fit parameters
             *  @param fitPar: Parameter to which the derivative shall be calculated */
            static Amg::Vector3D partialPlaneIntersect(const Amg::Vector3D& normal,
                                                       const double offset, 
                                                       const Amg::Vector3D& segPos, 
                                                       const Amg::Vector3D& segDir,
                                                       const LinePartialArray& linePartials,
                                                       const AxisDefs fitPar);
            
            /** @brief Updates the chi2, its Gradient & Hessian from the measurement residual. Depending on whether 
             *          the time needs to be taken into account or not the template parameter nDim is either 3 or 2, respectively.
             *  @param residual: Residual of the prediction from the measurement
             *  @param residualPartials: First order partial derivatives of the residual w.r.t the fit parameters
             *  @param measCovariance: Covariance matrix of the measurement
             *  @param gradient: Reference to the gradient of the chi2 which is updated by the function
             *  @param hessian: Reference to the Hessian matrix of the chi2 which is updated by the function
             *  @param chi2: Reference to the chi2 value itself which is updated by the function
             *  @param startPar: Number of the maximum fit parameter to consider */
            using MeasCov_t = CalibratedSpacePoint::Covariance_t;
            void updateDerivatives(const Amg::Vector3D& residual,
                                    const LinePartialArray& residualPartials,
                                    const MeasCov_t& measCovariance,
                                    AmgVector(5)& gradient,
                                    AmgSymMatrix(5)& hessian,
                                    double& chi2,
                                    int startPar) const;
            
            /** @brief Update step of the segment parameters using the Hessian and the gradient. If the Hessian is definite,
             *         the currentParameters are updated according to
             *                   x_{n+1} = x_{n} - H_{n}^{1} * grad(x_{n})
             *          Otherwise, the method of steepest descent is attempted.
             */            
            template <unsigned int nDim>
                bool updateParameters(Parameters& currentPars,
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