/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONR4__MuonSegmentFitHelperFunctions__H
#define MUONR4__MuonSegmentFitHelperFunctions__H

#include "Acts/Seeding/HoughTransformUtils.hpp"
#include "xAODMeasurementBase/UncalibratedMeasurement.h"
#include "xAODMuonPrepData/MdtDriftCircleContainer.h"
#include "xAODMuonPrepData/RpcStripContainer.h"
#include "MuonPatternEvent/MuonHoughDefs.h"

namespace MuonR4{
    namespace SegmentFitHelpers{
      double chiSqTermMdt(double y0, double tanTheta, const MuonR4::HoughHitType & measurement);
      double chiSqTermStrip(double x0, double y0, double tanPhi, double tanTheta, const MuonR4::HoughHitType & measurement);
      double segmentChiSquare(const double* par, const std::vector<MuonR4::HoughHitType> & hits, std::vector<double> & chi2PerMeas, const ActsGeometryContext & gctx, bool doBSConstraint);
      double chiSqTermBeamspot(double x0, double y0, double tanPhi, double tanTheta, const MuonR4::HoughHitType & hit, const ActsGeometryContext & gctx);
    }
}

#endif // MUONR4__MuonSegmentFitHelperFunctions__H
