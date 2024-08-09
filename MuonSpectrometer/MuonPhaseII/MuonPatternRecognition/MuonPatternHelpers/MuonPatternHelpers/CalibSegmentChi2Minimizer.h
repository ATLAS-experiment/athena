/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONR4__MUONPATTERNHELPERS_CALIBSEGMENTCHI2MINIMZER_H
#define MUONR4__MUONPATTERNHELPERS_CALIBSEGMENTCHI2MINIMZER_H

#include "Acts/Seeding/HoughTransformUtils.hpp"
#include "xAODMeasurementBase/UncalibratedMeasurement.h"
#include "xAODMuonPrepData/MdtDriftCircleContainer.h"
#include "xAODMuonPrepData/RpcStripContainer.h"
#include "MuonPatternEvent/MuonHoughDefs.h"
#include "AthenaBaseComps/AthMessaging.h"


#include "MuonSpacePoint/CalibratedSpacePoint.h"
#include <Math/Minimizer.h>

namespace MuonR4{
 
    class CalibSegmentChi2Minimizer: public ROOT::Math::IMultiGenFunction,
                                            AthMessaging {
      public:
          
          using HitType = std::unique_ptr<CalibratedSpacePoint>;
          using HitVec = std::vector<HitType>;
          using CalibratorFunc = std::function<HitVec(std::vector<const SpacePoint*>& spacePoints,
                                                      const Amg::Vector3D& segmentIsect,
                                                      const Amg::Vector3D& segmentDir,
                                                      const double timeOfArrival)>;
          /** Constructor taking the input hits to fit */
          CalibSegmentChi2Minimizer(const std::string& name,
                                    HitVec&& hits,
                                    CalibratorFunc calibrator,
                                    bool doT0Fit);
      
          /** @brief Evaluate the chi2 for the given set of parameters */
          double DoEval(const double* pars) const override final;
          /** @brief Clone of the object */
          CalibSegmentChi2Minimizer* Clone() const override final; 
          /** @brief Degrees of freedom */
          unsigned int NDim() const override final;
          /** @brief Are phi measurements in the collection */
          bool hasPhiMeas() const;
          /** @brief  Returns the degrees of freedom from the measurements & beamspot constraint */
          int nDoF() const;
          /** @brief Returns the used measurements */
          const HitVec& measurements() const;

          /** @brief Returns the contribution of each measurement to the chi2 */
          std::vector<double> chi2Terms(const double* pars) const;

          HitVec release(const double* pars);
         
      private:
          std::string m_name{};
          HitVec m_hits{};
          CalibratorFunc m_calibrator;
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
