/**
 * @file FPGATrackSimRegionMap_test.cxx
 * @brief Unit tests for FPGATrackSimRegionMap
 * @author Riley Xu - rixu@cern.ch
 * @date 2020-01-15
 */

#undef NDEBUG
#include <cassert>
#include <string>
#include <iostream>
#include <filesystem>

#include "TestTools/initGaudi.h"
#include "AthenaKernel/getMessageSvc.h"
#include "FPGATrackSimMaps/FPGATrackSimRegionMap.h"

using namespace std;




void test(FPGATrackSimRegionMap & rmap)
{
    // Hard-coded values from above files.
    // Make sure to change these if those files are changed
    //assert(rmap.getNRegions() == 96);
    assert(rmap.getRegionBoundaries(0, 2, 0).phi_min == 0);
    //assert(rmap.getRegionBoundaries(0, 2, 0).phi_max == 39);
}


void test_LUT(FPGATrackSimRegionMap & rmap)
{
    (void)rmap; // TODO
}


int main(int, char**)
{
    ISvcLocator* pSvcLoc;
    if (!Athena_test::initGaudi(pSvcLoc))
    {
        std::cerr << "Gaudi failed to initialize. Test failing." << std::endl;
        return 1;
    }

    string pmap_path="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/ATLAS-P2-RUN4-03-00-00/maps_9L/OtherFPGAPipelines/v0.20/eta0103phi0305.pmap";
    string rmap_path="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EFTracking/ATLAS-P2-RUN4-03-00-00/maps_9L/OtherFPGAPipelines/v0.20/eta0103phi0305.rmap";

    std::ifstream finTest(pmap_path);
    if (!finTest.is_open())
    {
        throw ("FPGATrackSimPlaneMap Couldn't open " + pmap_path);
    }
    vector<int> overrides;    
    finTest.close();
    finTest.open(pmap_path);

    std::vector<std::unique_ptr<FPGATrackSimPlaneMap>>  test_pmaps;
    for (int i = 0; i<6; i++)
    {
        test_pmaps.push_back(std::unique_ptr< FPGATrackSimPlaneMap> (new FPGATrackSimPlaneMap(finTest, 0, 1,overrides))); 
    }

    FPGATrackSimRegionMap rmap(test_pmaps, rmap_path, false);

    test(rmap);
    test_LUT(rmap);

    return 0;
}
