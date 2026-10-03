/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include <cstdlib>
#include <iostream>

#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"
#include "ActsGeometry/CylinderUtils.h"
#include "Acts/Definitions/Units.hpp"
#include "Acts/Definitions/Tolerance.hpp"
#include "Acts/Utilities/UnitVectors.hpp"
using namespace ActsTrk;
using namespace Acts::UnitLiterals;

std::ostream& operator<<(std::ostream& ostr, const std::array<double, 2>& solutions) {
    return ostr<<" x1: "<<solutions[0]<<" [mm], x2: "<<solutions[1]<<" [mm]";
}


int main () {
    constexpr double testRadius = 345._mm;
    constexpr unsigned testSteps = 15;
    constexpr double stepLength = 360._degree / testSteps;

    int retCode = EXIT_SUCCESS;
    for (unsigned step = 0; step <= testSteps; ++step) {
        const double phi = stepLength * step;
        const Amg::Vector3D dir = Acts::makeDirectionFromPhiTheta(phi, 90._degree);
        const std::array cylinderIsect = cylinderIntersectPaths(Amg::Vector3D::Zero(), dir, testRadius); 
        
        std::cout<<"Test step "<<step<<",  phi: "<<(phi / 1._degree)<<" -> dir: "<<Amg::toString(dir)
                 <<" ==> soluions: "<<cylinderIsect<<std::endl;
        if (std::abs(cylinderIsect[0] - testRadius) > Acts::s_transformEquivalentTolerance) {
            std::cerr<<__LINE__<<" - Mismatch in the positive solution "<<std::endl;
            retCode = EXIT_FAILURE;
        }
        if (std::abs(cylinderIsect[1] + testRadius) > Acts::s_transformEquivalentTolerance) {
            std::cerr<<__LINE__<<" - Mismatch in the negative solution "<<std::endl;
            retCode = EXIT_FAILURE;
        }
        /// Check shift the soluion by some mm
        constexpr double shift = 234._mm;
        const std::array shiftSolution = cylinderIntersectPaths(shift * dir, dir, testRadius);
        std::cout<<"              Shifted solution  ===> "<<shiftSolution<<std::endl;
        if (std::abs(shiftSolution[0] + shift - testRadius) > 
            Acts::s_transformEquivalentTolerance) {
            std::cerr<<__LINE__<<" - Mismatch in the positive solution "<<std::endl;
            retCode = EXIT_FAILURE;
        }
        if (std::abs(shiftSolution[1] + shift  + testRadius) > 
           Acts::s_transformEquivalentTolerance) {
            std::cerr<<__LINE__<<" - Mismatch in the negative solution "<<std::endl;
            retCode = EXIT_FAILURE;
        }
        const std::array outsideSolution = cylinderIntersectPaths(2*testRadius * dir, dir, testRadius);
        std::cout<<"              Outside solution  ===> "<<outsideSolution<<std::endl;
        if (outsideSolution[0] > 0){
            std::cerr<<__LINE__<<" - No positive solution is expected "<<std::endl;
            retCode = EXIT_FAILURE;
        }
        if(outsideSolution[0] < outsideSolution[1]) {
            std::cerr<<__LINE__<<" - The first solution is expected to be closer than the second one"<<std::endl;
            retCode = EXIT_FAILURE;
        }
        const std::array backwardSolution = cylinderIntersectPaths(-2*testRadius * dir, dir, testRadius);
        std::cout<<"     Backward outside solution  ===> "<<backwardSolution<<std::endl;
         if (backwardSolution[0] < 0){
            std::cerr<<__LINE__<<" - The backward solutiuon needs to be still positive "<<std::endl;
            retCode = EXIT_FAILURE;
        }
        if(backwardSolution[0] > backwardSolution[1]) {
            std::cerr<<__LINE__<<" - The first solution is expected to be closer than the second one"<<std::endl;
            retCode = EXIT_FAILURE;
        }
    }
    return retCode;
}