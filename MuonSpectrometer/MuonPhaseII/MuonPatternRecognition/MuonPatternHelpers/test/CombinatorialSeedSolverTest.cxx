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
    
    int testSeed(const std::array<SpacePoint*, 4>& spacePoints, const std::array<double,4>& truthDistances, 
                 Amg::Vector3D truthPosition, Amg::Vector3D truthDirection){
        
        const AmgSymMatrix(2) bMatrix = CombinatorialSeedSolver::betaMatrix(spacePoints);
        std::cout<<__FILE__<<":"<<__LINE__<<" Selected spacepoints: "<<Amg::toString(spacePoints[0]->positionInChamber())<<","<<Amg::toString(spacePoints[1]->positionInChamber())<<","<<Amg::toString(spacePoints[2]->positionInChamber())<<","<<Amg::toString(spacePoints[3]->positionInChamber())<<std::endl;
        std::cout<<__FILE__<<":"<<__LINE__<<" Beta matrix: "<<Amg::toString(bMatrix)<<", determinant: " <<bMatrix.determinant()<<"."<<std::endl;
        
        if (std::abs(bMatrix.determinant()) < std::numeric_limits<float>::epsilon()){
            std::cout<<__FILE__<<":"<<__LINE__<<" Beta matrix determinant = 0, try next combinatoric"<<std::endl;
            return EXIT_SUCCESS;
        }
        const std::array<double, 4> distsAlongStrip = CombinatorialSeedSolver::defineParameters(bMatrix, spacePoints);
                            // check that the distances along the strips are the exact opposite from what is generated
        if (distsAlongStrip[0] + truthDistances[0]  > std ::numeric_limits<float>::epsilon() ||
            distsAlongStrip[1] + truthDistances[1]  > std ::numeric_limits<float>::epsilon() ||
            distsAlongStrip[2] + truthDistances[2]  > std ::numeric_limits<float>::epsilon() ||
            distsAlongStrip[3] + truthDistances[3]  > std ::numeric_limits<float>::epsilon() ) {

            std::cerr<<__FILE__<<":"<<__LINE__<<" The initial line parameters could not be reconstructed:"
            <<"("<<(-truthDistances[0])<<", "<<(-truthDistances[1])<< ", "<<(-truthDistances[2])<<", "<<(-truthDistances[3])<<"). vs. "
            << "(" <<distsAlongStrip[0]<<", " <<distsAlongStrip[1]<<", " <<distsAlongStrip[2]<<", "<<distsAlongStrip[3] <<std::endl;
            return EXIT_FAILURE;
        }
        const auto [seedPos, seedDir] = CombinatorialSeedSolver::seedSolution(spacePoints, distsAlongStrip);
        std::cout<<__FILE__<<":"<<__LINE__<<" Reconstructed position "<<Amg::toString(seedPos)<<" + direction "<<Amg::toString(seedDir)<<std::endl;\
        
        if ( std::abs(std::abs(seedDir.dot(truthDirection)) - 1.) > std::numeric_limits<float>::epsilon()){
            std::cerr<<__FILE__<<":"<<__LINE__<<" The pointing direction is different "<<std::endl;
            std::cerr<<__FILE__<<":"<<__LINE__<<Amg::toString(truthDirection)<<std::endl;
            return EXIT_FAILURE;
        }

                        /// Last but not least check the intersection
        if ( (seedPos - truthPosition).mag() > std::numeric_limits<float>::epsilon()){
                std::cerr<<__FILE__<<":"<<__LINE__<<" The reference point is different: "
                <<Amg::toString(seedPos)<<" vs. "<<Amg::toString(truthPosition)<<std::endl;
                return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;

     }

    

    int main(){

    
        //strips' positions and directions in chamber's frame
        // let's pick XUXU combination of the layers


        
        const std::array<Amg::Vector3D, 8> stripDirections = {Amg::Vector3D::UnitX(), 
                                                             Amg::Vector3D::UnitX(),
                                                             Amg::dirFromAngles(1.5*Gaudi::Units::deg, 90.*Gaudi::Units::deg),
                                                             Amg::dirFromAngles(-1.5*Gaudi::Units::deg, 90.*Gaudi::Units::deg),
                                                             Amg::dirFromAngles(1.5*Gaudi::Units::deg, 90.*Gaudi::Units::deg),
                                                             Amg::dirFromAngles(-1.5*Gaudi::Units::deg, 90.*Gaudi::Units::deg),
                                                             Amg::Vector3D::UnitX(), 
                                                             Amg::Vector3D::UnitX()};


        //distances of planes from the 1st plane
        constexpr std::array<double, 8> distancesZ{-20.,-264.,0., 300., 350, 370.,400, 450.};
        
        TRandom3 rnd{12345};

        std::array<double, 8> linePars{};       
        std::array<Amg::Vector3D, 8> intersections{};

        for( unsigned int m = 0; m < 100; ++m) {

            std::cout<<"Processing #event "<<m<<std::endl;
            
            const Amg::Vector3D muonStart{rnd.Uniform(-500, 500),
                                        rnd.Uniform(-500, 500), -40};
            
            const Amg::Vector3D muonDir = Amg::Vector3D(rnd.Uniform(-5., 5.),
                                                        rnd.Uniform(-5., 5.), 1).unit();
            
            const Amg::Vector3D muonAtZ0 = muonStart + Amg::intersect<3>(muonStart, muonDir, Amg::Vector3D::UnitZ(), 0.).value_or(0.)*muonDir;
            
            std::array<std::unique_ptr<SpacePoint>, 8> spacePoints{};
            for (unsigned int layer = 0; layer < distancesZ.size(); ++layer) {
                /// Extrapolate onto the layer
                intersections[layer] = muonStart + Amg::intersect<3>(muonStart, muonDir, Amg::Vector3D::UnitZ(), distancesZ[layer]).value_or(0.) * muonDir ;

                linePars[layer] = Amg::intersect<3>(intersections[layer], stripDirections[layer], Amg::Vector3D::UnitX(), 0.).value_or(0.);
                Amg::Vector3D stripsPos = intersections[layer] + linePars[layer] * stripDirections[layer];
                std::cout<<"plane cross="<<Amg::toString(intersections[layer])<<std::endl;
                spacePoints[layer] = std::make_unique<SpacePoint>(stripsPos, stripDirections[layer]);
            }
            std::cout<<"Starting muon "<<Amg::toString(muonStart)<<"  "<<Amg::toString(muonDir)<<std::endl;
            std::cout<<"Intersections: "<<std::endl;

            for (unsigned int layer = 0; layer < distancesZ.size(); ++layer) {
            std::cout<<"  **** z="<<distancesZ[layer]<<", strip dist: "<<linePars[layer]<<", position: "<<Amg::toString(spacePoints[layer]->positionInChamber())<<std::endl;
            }

            //check combinatorics
            std::array<SpacePoint*, 4> seedPoints{};
            bool enableSwap = true;
            std::vector<std::vector<unsigned int>> swaps = {{0,1}, {0,2}, {0,3}, {1,2}, {1,3}, {2,3}};
            for (unsigned int l = 0; l < distancesZ.size() -3; ++l){
                seedPoints[0] = spacePoints[l].get();
                for (unsigned int k = l+1; k <distancesZ.size() - 2; ++k) {
                    seedPoints[1] = spacePoints[k].get();
                    for (unsigned int m = k+1; m < distancesZ.size() -1; ++m) {
                        seedPoints[2] = spacePoints[m].get();
                        //isSwapped = false;
                        for (unsigned int n = m+1; n < distancesZ.size(); ++n) {
                            seedPoints[3] = spacePoints[n].get();                      

                            //check consistency for swapping the layers -- all possible swaps
                            if(enableSwap){                        
                                for(auto& s :swaps){
                                    
                                    std::array<SpacePoint*,4> spacePointsSwapped = seedPoints;
                                    std::array<double,4> stripDistancesSwapped = {linePars[l], linePars[k], linePars[m], linePars[n]};
                                    std::swap(spacePointsSwapped[s[0]], spacePointsSwapped[s[1]]);
                                    std::swap(stripDistancesSwapped[s[0]], stripDistancesSwapped[s[1]]);

                                    int ret_test = testSeed(spacePointsSwapped, stripDistancesSwapped, muonAtZ0, muonDir); 
                                    if(ret_test == EXIT_FAILURE){
                                        return EXIT_FAILURE;

                                    }
                                              
                            
                                }
                            }else{

                                const std::array<double,4> truthDistances ={linePars[l], linePars[k], linePars[m], linePars[n]};
                                int ret_test =  testSeed(seedPoints, truthDistances, muonAtZ0, muonDir);
                                if(ret_test == EXIT_FAILURE){
                                    return EXIT_FAILURE;

                                }

                            }
                        }
                    }
                }

            }

         }
        
        std::cout<<__FILE__<<":"<<__LINE__<<"   -----  The test is a big success... "<<std::endl; 
        return EXIT_SUCCESS;
        
    }
