/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONR4__MUONPATTERNHELPERS_CALIBSEGMENTCHI2MINIMZER_H
#define MUONR4__MUONPATTERNHELPERS_CALIBSEGMENTCHI2MINIMZER_H
#include "GeoPrimitives/GeoPrimitives.h"
///
#include "Acts/Seeding/HoughTransformUtils.hpp"
#include "xAODMeasurementBase/UncalibratedMeasurement.h"
#include "xAODMuonPrepData/MdtDriftCircleContainer.h"
#include "xAODMuonPrepData/RpcStripContainer.h"
#include "MuonPatternEvent/MuonHoughDefs.h"
#include "AthenaBaseComps/AthMessaging.h"


#include "MuonSpacePoint/CalibratedSpacePoint.h"
#include <Math/Minimizer.h>

namespace MuonR4{
    class ISpacePointCalibrator;
    class CalibSegmentChi2Minimizer: public ROOT::Math::IMultiGenFunction, AthMessaging {
      public:
          
          using HitType = std::unique_ptr<CalibratedSpacePoint>;
          using HitVec = std::vector<HitType>;
          
          /** Constructor taking the input hits to fit */
          CalibSegmentChi2Minimizer(const std::string& name,
                                    const EventContext& ctx,
                                    const Amg::Transform3D& locToGlobTrf,
                                    HitVec&& hits,
                                    const ISpacePointCalibrator* calibrator,
                                    bool doT0Fit);
      
          /** @brief Evaluate the chi2 for the given set of parameters */
          double DoEval(const double* pars) const override final;
          /** @brief Clone of the object */
          CalibSegmentChi2Minimizer* Clone() const override final; 
          /** @brief Degrees of freedom */
          unsigned int NDim() const override final;
          /** @brief Are phi measurements in the collection */
          bool hasPhiMeas() const;
          /** @brief Does the fit include the time */
          bool doTimeFit() const;
          /** @brief  Returns the degrees of freedom from the measurements & beamspot constraint */
          int nDoF() const;
          /** @brief Returns the used measurements */
          const HitVec& measurements() const;

          /** @brief Returns the contribution of each measurement to the chi2 */
          std::vector<double> chi2Terms(const double* pars) const;
          /** @brief Releases all measurements */
          HitVec release(const double* pars);
          /** @brief Returns the local to global transformation */
          const Amg::Transform3D& localToGlobTrans() const;
         
      private:
          std::string m_name{};
          const EventContext& m_ctx;
          Amg::Transform3D m_locToGlob{Amg::Transform3D::Identity()};
          mutable HitVec m_hits ATLAS_THREAD_SAFE{};
          const ISpacePointCalibrator* m_calibrator{};
          bool m_doT0Fit{false};

          double segmentChi2TermStrip(const CalibratedSpacePoint& hit,  
                                      const Amg::Vector3D& chamberIsect, 
                                      const Amg::Vector3D& segmentDir) const;

          double segmentChi2TermMdt(const CalibratedSpacePoint& hit,
                                    const Amg::Vector3D& chamberIsect, 
                                    const Amg::Vector3D& segmentDir) const;
          


          bool m_hasPhi{false};
          unsigned int m_nMeas{0};
    };
}

#endif // MUONR4__MuonSegmentFitHelperFunctions__H
