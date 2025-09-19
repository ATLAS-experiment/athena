// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
/**
 * @file FPGATrackSimSSMap_test.cxx
 * @brief Unit tests for FPGATrackSimSSMap
 * @author Riley Xu - rixu@cern.ch
 * @date 2020-01-15
 */

#undef NDEBUG


#include "TestTools/initGaudi.h"
#include "AthenaKernel/getMessageSvc.h"
#include "FPGATrackSimMaps/FPGATrackSimPlaneMap.h"

#include <cassert>
#include <string>
#include <iostream>
#include <numeric>
#include <filesystem>
#include <stdexcept>

using namespace std;



// Hard-coded values from eta0103phi0305.pmap
// Make sure to change these if the file is changed
//void test(FPGATrackSimPlaneMap & pmap)
void test(const std::vector<std::unique_ptr<FPGATrackSimPlaneMap>> & pmaps)
{
    //assert(pmap.getNDetLayers() == 227);
    for(int ipmap=0; ipmap<6; ipmap++){
        assert(pmaps.at(ipmap)->getNLogiLayers() == 9);
        assert(pmaps.at(ipmap)->getNCoords() == 10);//why 10 when it was 9

        assert(pmaps.at(ipmap)->isPixel(0));
        assert(pmaps.at(ipmap)->isSCT(3));

        assert(pmaps.at(ipmap)->getNSections(5) == 1);
        assert(pmaps.at(ipmap)->getCoordOffset(1) == 2);
        assert(pmaps.at(ipmap)->getDim(0) == 2);

        assert(pmaps.at(ipmap)->getLayerInfo(0, 0).zone == DetectorZone::barrel);
        //assert(pmap.getLayerInfo(3, 0).physLayer == 3);
        assert(pmaps.at(ipmap)->getLayerInfo(3, 0).physLayer == 2);
        assert(pmaps.at(ipmap)->getLayerSection(SiliconTech::strip, DetectorZone::barrel, 0).layer == 1);
    }
}


int main(int, char**)
{
    ISvcLocator* pSvcLoc;
    if (!Athena_test::initGaudi(pSvcLoc))
    {
        std::cerr << "Gaudi failed to initialize. Test failing." << std::endl;
        return 1;
    }
    std::vector<std::unique_ptr<FPGATrackSimPlaneMap>>  pmap_vector_1st; //  pointer to the pmap object for 1st stage

    string pmap_path="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/ATLAS-P2-RUN4-03-00-00/maps_9L/OtherFPGAPipelines/v0.20/eta0103phi0305.pmap";
    std::ifstream finTest(pmap_path);
    if (!finTest.is_open())
    {
        throw ("FPGATrackSimPlaneMap Couldn't open " + pmap_path);
    }
    vector<int> overrides;    
    finTest.close();
    finTest.open(pmap_path);
    try{
      for (int i = 0; i<6; i++) {
          pmap_vector_1st.push_back(std::unique_ptr<FPGATrackSimPlaneMap>(new FPGATrackSimPlaneMap(finTest, 0, 1)));
      }
    } catch (std::exception & e){
      std::cerr<<"Exception "<<e.what()<<" in FPGATrackSimPlaneMap_test"<<std::endl;
      return 1;
    }

    test(pmap_vector_1st);

    return 0;
}
