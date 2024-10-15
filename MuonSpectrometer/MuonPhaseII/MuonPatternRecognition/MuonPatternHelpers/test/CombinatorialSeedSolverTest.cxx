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


#include <iostream>
#include <vector>
#include <TRandom3.h>

using namespace MuonR4;

 //space points class 
    class SpacePoint{

        public:

        SpacePoint() : m_pos(Amg::Vector3D()), m_dir(Amg::Vector3D()) {};

        SpacePoint(const Amg::Vector3D& chamberPos, const Amg::Vector3D& chamberDir):
        m_pos(chamberPos),
        m_dir(chamberDir){

        };

        const Amg::Vector3D& positionInChamber() const{return m_pos;};
        const Amg::Vector3D& directionInChamber() const{return m_dir;};

        private: 
        Amg::Vector3D m_pos;

        Amg::Vector3D m_dir;
    };

void test(){

   
    //strips' positions and directions in chamber's frame
    // let's pick XUXU combination of the layers


    
    std::vector<Amg::Vector3D> stripDirections = {{1., 0., 0.}, {std::cos(1.5*M_PI/180.), std::sin(1.5*M_PI/180.) , 0.0000}, 
                                                 {1., 0., 0.}, {std::cos(1.5*M_PI/180.),  std::sin(1.5*M_PI/180.) , 0.0000}};

    
    //distances of planes from the 1st plane
    std::vector<double> distancesZ = {0.,20.,40.,60.};
    
    TRandom3 rnd{12345};
    for( unsigned int m = 0; m < 1000; ++m) {

        std::cout<<"Processing #event "<<m<<std::endl;
        
        const Amg::Vector3D muonStart{rnd.Uniform(-500, 500),
                                   rnd.Uniform(-500, 500), -40};
        
        const Amg::Vector3D muonDir = Amg::Vector3D(rnd.Uniform(-5,5),
                                                    rnd.Uniform(-5,5), 1).unit();
        
        std::array<Amg::Vector3D, 4> stripsPos{};
        std::array<double, 4> linePars{};
        std::array<SpacePoint*,4> strips;
        std::vector<Amg::Vector3D> intersections;
        for (unsigned int layer = 0; layer < distancesZ.size(); ++layer) {
            /// Extrapolate onto the layer
            const Amg::Vector3D planeCross = muonStart + Amg::intersect<3>(muonStart, muonDir, Amg::Vector3D::UnitZ(), distancesZ[layer]).value_or(0.) * muonDir ;
            intersections.push_back(planeCross);

            linePars[layer] = Amg::intersect<3>(planeCross, stripDirections[layer], Amg::Vector3D::UnitX(), 0.).value_or(0.);
            stripsPos[layer] = planeCross + linePars[layer] * stripDirections[layer];
            std::cout<<"plane cross="<<Amg::toString(planeCross)<<std::endl;
            strips[layer] = new SpacePoint(stripsPos[layer], stripDirections[layer]);
        }
        std::cout<<"Starting muon "<<Amg::toString(muonStart)<<"  "<<Amg::toString(muonDir)<<std::endl;
        std::cout<<"Intersections: "<<std::endl;
        for (unsigned int layer = 0; layer < distancesZ.size(); ++layer) {
           std::cout<<"  **** z="<<distancesZ[layer]<<", strip dist: "<<linePars[layer]<<", position: "<<Amg::toString(stripsPos[layer])<<std::endl;

          
        }

        AmgSymMatrix(2) bMatrix = CombinatorialSeedSolver::betaMatrix(strips);
        std::array<double,4> parameters = CombinatorialSeedSolver::defineParameters(bMatrix, strips);
        std::pair<Amg::Vector3D, Amg::Vector3D> seed = CombinatorialSeedSolver::seedSolution(strips, parameters);

        for(unsigned int i = 0; i< parameters.size(); i++){
            
        if( std::fabs(std::fabs(parameters[i]) - std::fabs(linePars[i])) > 1e-6 ){
                throw std::runtime_error("Parameter from analytical solution is " + std::to_string(parameters[i]) + " but truth parameter is " + std::to_string(linePars[i]));
            }
        delete strips[i];

        }

        if(!intersections[0].isApprox(seed.first, 1e-6)){
            throw std::runtime_error("Seed position from analytical solution is " + Amg::toString(seed.first) + " but truth position is " + Amg::toString(intersections[0]));
        }

        if(!muonDir.isApprox(seed.second, 1e-6)){
            throw std::runtime_error("Seed direction from analytical solution is " + Amg::toString(seed.second) + " but truth direction is " + Amg::toString(muonDir));
        }

         

    }    

}

int main(){
    test();
    return 0;
}