/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file CxxUtils/test/CombinatorialNSW_test.cxx
 * @author Dimitra Amperiadou
 * @brief Test for combinatorial seeding for strips
 */

#include "GeoPrimitives/GeoPrimitives.h"
#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"

#include "MuonPatternHelpers/CombinatorialSeedSolver.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "GeoModelKernel/throwExcept.h"

#include <iostream>
#include <stdlib.h>
#include <TRandom3.h>

using namespace MuonR4;

 //space points class 
    class SpacePoint{

        public:

        SpacePoint() = default;

        SpacePoint(const Amg::Vector3D& chamberPos, const Amg::Vector3D& chamberDir):
            m_pos{chamberPos},
            m_dir{chamberDir} {}

        const Amg::Vector3D& positionInChamber() const{return m_pos;};
        const Amg::Vector3D& directionInChamber() const{return m_dir;};

        private: 
        Amg::Vector3D m_pos{Amg::Vector3D::Zero()};
        Amg::Vector3D m_dir{Amg::Vector3D::Zero()};
    };

int main(){

   
    //strips' positions and directions in chamber's frame
    // let's pick XUXU combination of the layers


    
    std::array<Amg::Vector3D, 4> stripDirections = {Amg::Vector3D::UnitX(), 
                                                    Amg::dirFromAngles(1.5*Gaudi::Units::deg, 90.*Gaudi::Units::deg),
                                                    Amg::Vector3D::UnitX(),
                                                    Amg::dirFromAngles(-1.5*Gaudi::Units::deg, 90.*Gaudi::Units::deg)};

    
    //distances of planes from the 1st plane
    constexpr std::array<double, 4> distancesZ{0.,20.,40.,60.};
    
    TRandom3 rnd{12345};

    std::array<Amg::Vector3D, 4> stripsPos{};
    std::array<double, 4> linePars{};
    std::array<SpacePoint*,4> strips{};
    std::array<Amg::Vector3D, 4> intersections{};

    for( unsigned int m = 0; m < 1000; ++m) {

        std::cout<<"Processing #event "<<m<<std::endl;
        
        const Amg::Vector3D muonStart{rnd.Uniform(-500, 500),
                                      rnd.Uniform(-500, 500), -40};
        
        const Amg::Vector3D muonDir = Amg::Vector3D(rnd.Uniform(-5., 5.),
                                                    rnd.Uniform(-5., 5.), 1).unit();
        
        std::array<std::unique_ptr<SpacePoint>, 4> spacePoints{};
        for (unsigned int layer = 0; layer < distancesZ.size(); ++layer) {
            /// Extrapolate onto the layer
            intersections[layer] = muonStart + Amg::intersect<3>(muonStart, muonDir, Amg::Vector3D::UnitZ(), distancesZ[layer]).value_or(0.) * muonDir ;

            linePars[layer] = Amg::intersect<3>(intersections[layer], stripDirections[layer], Amg::Vector3D::UnitX(), 0.).value_or(0.);
            stripsPos[layer] = intersections[layer] + linePars[layer] * stripDirections[layer];
            std::cout<<"plane cross="<<Amg::toString(intersections[layer])<<std::endl;
            spacePoints[layer] = std::make_unique<SpacePoint>(stripsPos[layer], stripDirections[layer]);
            strips[layer] = spacePoints[layer].get();
        }
        std::cout<<"Starting muon "<<Amg::toString(muonStart)<<"  "<<Amg::toString(muonDir)<<std::endl;
        std::cout<<"Intersections: "<<std::endl;
        for (unsigned int layer = 0; layer < distancesZ.size(); ++layer) {
           std::cout<<"  **** z="<<distancesZ[layer]<<", strip dist: "<<linePars[layer]<<", position: "<<Amg::toString(stripsPos[layer])<<std::endl;
        }

        AmgSymMatrix(2) bMatrix = CombinatorialSeedSolver::betaMatrix(strips);
        std::array<double,4> parameters = CombinatorialSeedSolver::defineParameters(bMatrix, strips);
        std::pair<Amg::Vector3D, Amg::Vector3D> seed = CombinatorialSeedSolver::seedSolution(strips, parameters);
        
        constexpr double tolerance = 1.e-9;
        /** Check that the parameters are recuperated */
        for(unsigned int i = 0; i< parameters.size(); i++){            
             /// The predicted parameters need to exactly cancel the extracted line pars
            if( std::abs(parameters[i] +  linePars[i]) > tolerance ){
                std::cerr<<__FILE__<<":"<<__LINE__<<" -- Parameter from analytical solution is "<<parameters[i]<<" but truth parameter is "<<linePars[i]<<std::endl;
                return EXIT_FAILURE;
            }
        }

        if((intersections[0]-seed.first).mag() > tolerance) {
            std::cerr<<__FILE__<<":"<<__LINE__<<" -- Seed position from analytical solution is " 
                     <<Amg::toString(seed.first)<<" but truth position is "<<Amg::toString(intersections[0])<<std::endl;
            return EXIT_FAILURE;
        }

        if( (muonDir - seed.second).mag() > tolerance){
            std::cerr<<__FILE__<<":"<<__LINE__<<" -- Seed direction from analytical solution is " 
                     <<Amg::toString(seed.second)<<" but truth direction is "<<Amg::toString(muonDir)<<std::endl;
            return EXIT_FAILURE;
        }
    }    
    return EXIT_SUCCESS;
}
