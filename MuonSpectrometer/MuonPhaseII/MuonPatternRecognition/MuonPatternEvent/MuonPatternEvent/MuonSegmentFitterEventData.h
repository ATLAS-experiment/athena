/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONR4__MUONSEGMENTFITEVENTDATA__H
#define MUONR4__MUONSEGMENTFITEVENTDATA__H

#include <Math/Factory.h>
#include <Math/Minimizer.h>
#include "ActsGeometryInterfaces/ActsGeometryContext.h"

namespace MuonR4{
    template <class HitType> 
    struct MuonSegmentFitterEventData_impl{
        enum class parameterIndices{
            y0 = 0,
            tanTheta = 1,
            x0 = 2,
            tanPhi = 3
        };
        std::unique_ptr<ROOT::Math::Minimizer> minimizer{ROOT::Math::Factory::CreateMinimizer("Minuit2", "")};
        const ActsGeometryContext* gctx; 
        double y0{0.}; 
        double x0{0.};
        double sigmaY0{0.}; 
        double sigmaX0{0.};
        double tanTheta{0.}; 
        double tanPhi{0.}; 
        double sigmaTanTheta{0.}; 
        double sigmaTanPhi{0.}; 
        double chi2{0.}; 
        bool foundMin{false};
        std::vector<HitType> measurementsToFit{}; 
        std::vector<double> chi2_per_measurement{};
    };
}

#endif 
