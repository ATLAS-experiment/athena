/*
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
*/

#include <cstdlib>

#include <GeoModelUtilities/GeoAlignmentStore.h>
#include <GeoModelKernel/GeoAlignableTransform.h>
#include <GeoModelKernel/GeoIntrusivePtr.h>
#include <GeoModelKernel/GeoFullPhysVol.h>

#include <GeoModelHelpers/TransformToStringConverter.h>
#include <GeoModelHelpers/TransformSorter.h>
#include <GeoModelHelpers/defineWorld.h>

#include <GeoModelKernel/Units.h>
#include <cstdlib>
#include <ctime>
#include <array>
#include <future>
#include <chrono>
#include <random>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <thread>

#include "TStopwatch.h"

constexpr size_t nNodesToTest{35000};
constexpr size_t nAccessTests{2500};
constexpr unsigned int accThreads = 16;
using AlignNodeArray = std::array<GeoIntrusivePtr<GeoAlignableTransform>, nNodesToTest>;
using TransformArray = std::array<std::shared_ptr<const GeoTrf::Transform3D>, nNodesToTest>;
/// @brief Cache the access pattern
using AccessArray = std::array<unsigned int, nNodesToTest>;
/// @brief Unordered map as reference container to measure the performance
using RefStoreType = std::unordered_map<const GeoTransform*, std::shared_ptr<const Amg::Transform3D>>;


/** Returns how many threads have not yet finished their work*/
template <class T> size_t count_active(const std::vector<std::future<T>>& threads) {
    size_t counts{0};
    for (const std::future<T>& thread : threads) {
         using namespace std::chrono_literals;
        if (thread.wait_for(0ms) != std::future_status::ready) ++counts;
    }
    return counts;
}
/** Converts the time interval into a human readable string*/
std::string timeToString(double time) {
    std::stringstream sstr{};
    if (time > 1.e-3) {
        const int hours = std::floor(time / 3600);
        time -= 3600.*hours;        
        const int minutes = std::floor(time / 60 );
        time -= 60. * minutes;
        const int seconds = std::floor(time);
        if (hours){
            sstr<<std::setw(2)<<std::setfill('0')<<hours<<":"
                <<std::setw(2)<<std::setfill('0')<<minutes<<":"
                <<std::setw(2)<<std::setfill('0')<<seconds<<" h";
        } else if (minutes){
             sstr<<std::setw(2)<<std::setfill('0')<<minutes<<":"
                 <<std::setw(2)<<std::setfill('0')<<seconds<<" m";
        }else sstr<<std::setw(2)<<std::setfill('0')<<seconds<<" s";
    } else {
        const int milliseconds = std::floor(time * 1.3);
        const int microseconds = std::floor(time * 1.e6) - 1000 *milliseconds;
        const int nanoseconds = std::floor(time * 1.e9) - 1000* microseconds - 1000000 * milliseconds;
        if (milliseconds) {
            sstr<<std::setw(2)<<std::setfill('0')<<milliseconds<<" ms ";
        } if (microseconds) {
            sstr<<std::setw(2)<<std::setfill('0')<<microseconds<<" mus ";
        } if (nanoseconds)  {
            sstr<<std::setw(2)<<std::setfill('0')<<nanoseconds<<" ns";
        }
    }
    return sstr.str();
}

/** Helper class to partially fill the alignment store from the reference array 
 *  in a multi-threaded like environment  */
struct StoreFiller{
    StoreFiller(const AlignNodeArray& transformNodes_, 
                const TransformArray& deltas_,
                GeoAlignmentStore& store_, 
                size_t begin_, size_t end_):
        transformNodes{transformNodes_},
        deltas{deltas_},
        store{store_},
        begin{begin_},
        end{end_}{}
    bool execute() {
        if (executed) return result;
        result = executed = true;
        for (size_t node = begin; node < std::min(end, nNodesToTest); ++node) {
            store.setDelta(transformNodes[node], deltas[node]);
            if (!store.getDelta(transformNodes[node])) {
                std::cerr<<"testGeoAlignmentStore() "<<__LINE__<<" failed to store the "
                            <<(node+1)<<"-th element "
                            <<GeoTrf::toString(*deltas[node])<<std::endl;
                result = false;
            }        
        }            
        return result;
    }
    const AlignNodeArray& transformNodes;
    const TransformArray& deltas;
    GeoAlignmentStore& store;
    size_t begin{0};
    size_t end{0};

    bool executed{false};
    bool result{false};
};

/**
 * Starts a container access timing test. The elements in the transformNodes are retrieved and the 
 * associated transform is then fatched from the container and then compared to the transforms in the
 * reference. The order in which the particular elements are accesssed is given by the accessPattern array.
*/
template <class MapType> bool measureAccessTime(const AlignNodeArray& transformNodes,
                                                const TransformArray& deltas,
                                                const MapType& storeToTest,
                                                const AccessArray& accessPattern,
                                                TStopwatch& watch,
                                                unsigned int nThreads) { 

    nThreads = std::min(nThreads, std::thread::hardware_concurrency());
 
    struct AccessPatternTest{
        AccessPatternTest(const AlignNodeArray& transformNodes_,
                          const TransformArray& deltas_,
                          const MapType& storeToTest_,
                          const AccessArray& accessPattern_):
            transformNodes{transformNodes_},
            deltas{deltas_},
            storeToTest{storeToTest_},
            accessPattern{accessPattern_} {
                static_assert(std::is_same<RefStoreType, MapType>::value ||
                              std::is_same<GeoAlignmentStore, MapType>::value,
                              "Unsupported alignment store type");
            }
        bool execute() {
            if (executed) return return_code;
            executed = true;
        
            for (unsigned int trial = 0; trial < nAccessTests; ++trial) {
                for (unsigned int accIdx : accessPattern){
                    const GeoAlignableTransform* alignTrf = transformNodes[accIdx];
                    const std::shared_ptr<const Amg::Transform3D>& trf{deltas[accIdx]};
                    const Amg::Transform3D* testTrf{nullptr};
                    if constexpr(std::is_same<MapType, RefStoreType>::value ) {
                        testTrf = storeToTest.find(alignTrf)->second.get();
                    } else if constexpr(std::is_same<MapType, GeoAlignmentStore>::value ){
                        testTrf = storeToTest.getDelta(alignTrf);
                    }
                    if (testTrf != trf.get()) {
                        std::cerr<<"testGeoAlignmentStore() "<<__LINE__<<" - Access test failed "
                                  <<GeoTrf::toString(*trf,true)<<" vs. "<<GeoTrf::toString(*testTrf, true)<<std::endl;
                        return_code = false;
                    }
                }
                if (!return_code) break;
            }
            return return_code;
        }    
        const AlignNodeArray& transformNodes;
        const TransformArray& deltas;
        const MapType& storeToTest;
        const AccessArray& accessPattern;

        bool executed{false};
        bool return_code{true};
    
    };
    std::vector<std::unique_ptr<AccessPatternTest>> testers{};
    for (unsigned int th =0; th < nThreads; ++th) {
        testers.emplace_back(std::make_unique<AccessPatternTest>(transformNodes, deltas,
                                                                 storeToTest, accessPattern));
    }
    std::vector<std::future<bool>> threads{};
    watch.Reset();
    watch.Start();
    for (std::unique_ptr<AccessPatternTest>& test : testers) {
        AccessPatternTest* testerPtr = test.get();
        threads.emplace_back(std::async(std::launch::async,[testerPtr](){return testerPtr->execute();}));
    }
    while (count_active(threads) > 0){
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    watch.Stop();
    bool return_code{true};
    for (std::future<bool>& th : threads) {
        return_code &= th.get();
    }
    return return_code;
}


/** @brief Tests whether the alignment store is filled with all delta transformations and
 *         also whether the GeoTransform -> Amg::Transform association is correct.
 */
bool testStoreAssignment(const AlignNodeArray& transformNodes,
                         const TransformArray& deltas,
                         const GeoAlignmentStore& store,
                         const PVLink world) {
    bool success = true;
    GeoTrf::TransformSorter sorter{};
    for (size_t node = 0; node < nNodesToTest; ++node) {
        const GeoTrf::Transform3D* deltaPtr = store.getDelta(transformNodes[node]);
        if (!deltaPtr){
            std::cerr<<"testGeoAlignmentStore() "<<__LINE__<<" No delta transform has been "
                     <<"cached for node number "<<node<<"."<<std::endl;
            success = false;
            continue;
        } else if (deltaPtr != deltas[node].get()) {
            std::cerr<<"testGeoAlignmentStore() "<<__LINE__<<" Different deltas have been cached for "
                     <<"node number "<<node<<". Found: "<<GeoTrf::toString(*deltaPtr,true)<<", but expected "
                     <<GeoTrf::toString(*deltas[node])<<std::endl;
            success = false;
        } else if (world && sorter.compare(*deltaPtr, world->getChildVol(node)->getX(&store))) {
            std::cerr<<"testGeoAlignmentStore() "<<__LINE__<<" The physical volume number "<<node<<" does not move "
                     <<" under the alignment. Expect: "<<GeoTrf::toString(*deltaPtr, true)<<" vs. observed: "
                     <<GeoTrf::toString(world->getChildVol(node)->getX(&store), true)<<std::endl;
            success = false;
        }
    }
    return success;
}

void fillStoreMT(const AlignNodeArray& transformNodes,
                 const TransformArray& deltas,
                 GeoAlignmentStore& store,
                 unsigned int nThreads) {
    nThreads = std::min(nThreads, std::thread::hardware_concurrency());
    const size_t fillsPerThread = nNodesToTest / nThreads;

    std::vector<std::unique_ptr<StoreFiller>> fillers{};
    std::vector<std::future<bool>> threads{};
    size_t begin =0;
    do {
        fillers.emplace_back(std::make_unique<StoreFiller>(transformNodes, deltas, store, begin, begin + fillsPerThread));
        begin+=fillsPerThread;
    } while (begin < nNodesToTest);
    
    for (std::unique_ptr<StoreFiller>& fill : fillers) {
        StoreFiller* fillerPtr = fill.get();
        threads.emplace_back(std::async(std::launch::async,[fillerPtr](){return fillerPtr->execute();}));
    }
    while (count_active(threads) > 0){
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

int main() {
    int return_code = EXIT_SUCCESS;
    /// Initialize the transformations
    AlignNodeArray transformNodes{};
    PVLink world {createGeoWorld()};
    const GeoLogVol* airVol = world->getLogVol();
    for (size_t node = 0; node < nNodesToTest; ++node) {
        transformNodes[node] = make_intrusive<GeoAlignableTransform>(GeoTrf::Transform3D::Identity());
        world->add(transformNodes[node]);
        world->add(make_intrusive<GeoFullPhysVol>(airVol));
    }
    
    TransformArray deltas{};
    
    std::srand(12345);
    /// Initialize a reference set delta transformations
    for (size_t delta = 0; delta < nNodesToTest; ++delta) {
        GeoTrf::GeoRotation deltaRot{std::rand() * GeoModelKernelUnits::deg,
                                     std::rand() * GeoModelKernelUnits::deg,
                                     std::rand() * GeoModelKernelUnits::deg };
        GeoTrf::Vector3D deltaTrans{std::rand()*(std::rand() % 3 == 0 ? -1. : 1.), 
                                    std::rand()*(std::rand() % 3 == 1 ? -1. : 1.) , 
                                    std::rand()*(std::rand() % 3 == 0 ? -1. : 1.)};
        deltas[delta] = std::make_shared<GeoTrf::Transform3D>(GeoTrf::GeoTransformRT{deltaRot, deltaTrans});
    }
    
    /// Fill the alignment store in serial mode
    {
        GeoAlignmentStore alignStore{};
        StoreFiller(transformNodes, deltas, alignStore, 0, transformNodes.size()).execute();
        if (!testStoreAssignment(transformNodes,deltas, alignStore, world)) {
            std::cerr<<"testAlignStore() "<<__LINE__<<" Serial alignment store filling failed."<<std::endl;
            return_code = EXIT_FAILURE;
            return return_code;
        }
    }
    /// Test the multi threaded alignment store filling
    std::random_device rd;
    std::mt19937 g(rd());

    for (size_t trial =1 ; trial <= 5; ++ trial) {
        GeoAlignmentStore alignStore{};
        fillStoreMT(transformNodes, deltas, alignStore, accThreads);
        if (!testStoreAssignment(transformNodes, deltas, alignStore, world)) {
            std::cerr<<"testGeoAlignmentStore() "<<__LINE__<<" alignment store filling failed with "<<accThreads
                        <<" threads failed at the "<<trial<<"-th trial."<<std::endl;
            return_code = EXIT_FAILURE;
        }
        world.reset();
        std::shuffle(deltas.begin(),deltas.end(), g);
        fillStoreMT(transformNodes, deltas, alignStore, accThreads);
        if (!testStoreAssignment(transformNodes, deltas, alignStore, world)) {
            std::cerr<<"testGeoAlignmentStore() "<<__LINE__<<" alignment store filling failed with "<<accThreads
                        <<" threads failed at the "<<trial<<"-th trial."<<std::endl;
            return_code = EXIT_FAILURE;
        }

    }


    /// Take std::unordered_map as refernce for the timing tests
    RefStoreType unOrderedStore{};
    for (size_t k = 0 ; k < nNodesToTest; ++k) {
        unOrderedStore[transformNodes[k]] = deltas[k];
    }
    
    /// Define the array to read the indices from. Will be shuffled later for random access test
    std::array<unsigned int, nNodesToTest> accessArray{};
    for (unsigned int k =0; k < nNodesToTest; ++k) {
        accessArray[k] =k;
    }
    

    /// Access timing test 
    std::cout<<std::endl<<"testGeoAlignmentStore() "<<__LINE__<<" - Start linear access timing test over "<<nAccessTests
                        <<" samples using "<<accThreads<<" threads. "<<std::endl<<std::endl;
    
    TStopwatch refClock{};
    constexpr size_t nEleAcc = nAccessTests * nNodesToTest;
    if (!measureAccessTime(transformNodes, deltas, unOrderedStore, accessArray, refClock, accThreads)){
        std::cout<<"testGeoAlignmentStore() "<<__LINE__<<" - Failed "<<std::endl;
        return_code = EXIT_FAILURE;
    } else {
        std::cout<<"testGeoAlignmentStore() "<<__LINE__<<" -   Simple access test took CPU: "
                 <<timeToString(refClock.CpuTime())<<", real: "<<timeToString(refClock.RealTime())
                 <<". Per access time: "<<timeToString(refClock.CpuTime() / nEleAcc)<<" (CPU) "
                 << timeToString(refClock.RealTime()  / nEleAcc)<<" (real)"<<std::endl;
    }

    GeoAlignmentStore unLockedStore{};
    StoreFiller(transformNodes, deltas, unLockedStore, 0, transformNodes.size()).execute();
    TStopwatch unlockedClock{};
    
    if (!measureAccessTime(transformNodes, deltas, unLockedStore, accessArray, unlockedClock, accThreads)) {
        std::cout<<"testGeoAlignmentStore() "<<__LINE__<<" - Failed "<<std::endl;
        return_code = EXIT_FAILURE;
    } else {
        std::cout<<"testGeoAlignmentStore() "<<__LINE__<<" - Unlocked access test took CPU: "
                <<timeToString(unlockedClock.CpuTime())  
                << " ("<<( 100.*unlockedClock.CpuTime() / refClock.CpuTime())<<"% w.r.t. simple), real: "
                <<timeToString(unlockedClock.RealTime()) 
                << " ("<<( 100.*unlockedClock.RealTime() / refClock.RealTime())<<"% w.r.t. simple)."
                <<" Per access time CPU: "<<timeToString(unlockedClock.CpuTime() / nEleAcc)<<", real: "
                <<timeToString(unlockedClock.RealTime()  / nEleAcc)<<std::endl;
    }
    GeoAlignmentStore lockedStore{};
    StoreFiller(transformNodes, deltas, lockedStore, 0, transformNodes.size()).execute();
    lockedStore.lockDelta();

    TStopwatch lockedClock{};
    if (!measureAccessTime(transformNodes, deltas, lockedStore, accessArray, lockedClock, accThreads)) {
        std::cout<<"testGeoAlignmentStore() "<<__LINE__<<" - Failed "<<std::endl;
        return_code = EXIT_FAILURE;
    } else {
        std::cout<<"testGeoAlignmentStore() "<<__LINE__<<" -  Locked access test took CPU: "
                <<timeToString(lockedClock.CpuTime())  
                << " ("<<( 100.*lockedClock.CpuTime() / refClock.CpuTime())<<"% w.r.t. simple), real: "
                <<timeToString(lockedClock.RealTime()) 
                << " ("<<( 100.*lockedClock.RealTime() / refClock.RealTime())<<"% w.r.t. simple)."
                <<" Per access time CPU: "<<timeToString(lockedClock.CpuTime() / nEleAcc)<<", real: "
                <<timeToString(lockedClock.RealTime()  / nEleAcc)<<std::endl;
    }
    
    std::shuffle(accessArray.begin(),accessArray.end(), g);
  
    std::cout<<std::endl<<"testGeoAlignmentStore() "<<__LINE__<<" - Start random access timing test over "<<nAccessTests<<" samples. "<<std::endl<<std::endl;
    if (!measureAccessTime(transformNodes, deltas, unOrderedStore, accessArray, refClock, accThreads)){
        std::cout<<"testGeoAlignmentStore() "<<__LINE__<<" - Failed "<<std::endl;
        return_code = EXIT_FAILURE;
    } else {
        std::cout<<"testGeoAlignmentStore() "<<__LINE__<<" -   Simple access test took CPU: "
                 <<timeToString(refClock.CpuTime())<<", real: "<<timeToString(refClock.RealTime())
                 <<". Per access time: "<<timeToString(refClock.CpuTime() / nEleAcc)<<" (CPU) "
                 <<timeToString(refClock.RealTime()  / nEleAcc)<<" (real)"<<std::endl;
    }
   
    if (!measureAccessTime(transformNodes, deltas, unLockedStore, accessArray, unlockedClock, accThreads)) {
        std::cout<<"testGeoAlignmentStore() "<<__LINE__<<" - Failed "<<std::endl;
        return_code = EXIT_FAILURE;
    } else {
        std::cout<<"testGeoAlignmentStore() "<<__LINE__<<" - Unlocked access test took CPU: "
                <<timeToString(unlockedClock.CpuTime())  
                << " ("<<( 100.*unlockedClock.CpuTime() / refClock.CpuTime())<<"% w.r.t. simple), real: "
                <<timeToString(unlockedClock.RealTime()) 
                << " ("<<( 100.*unlockedClock.RealTime() / refClock.RealTime())<<"% w.r.t. simple)."
                <<" Per access time CPU: "<<timeToString(unlockedClock.CpuTime() / nEleAcc)<<", real: "
                <<timeToString(unlockedClock.RealTime()  / nEleAcc)<<std::endl;
    }

    if (!measureAccessTime(transformNodes, deltas, lockedStore, accessArray, lockedClock, accThreads)) {
        std::cout<<"testGeoAlignmentStore() "<<__LINE__<<" - Failed "<<std::endl;
        return_code = EXIT_FAILURE;
    } else {
        std::cout<<"testGeoAlignmentStore() "<<__LINE__<<" -  Locked access test took CPU: "
                <<timeToString(lockedClock.CpuTime())  
                << " ("<<( 100.*lockedClock.CpuTime() / refClock.CpuTime())<<"% w.r.t. simple), real: "
                <<timeToString(lockedClock.RealTime()) 
                << " ("<<( 100.*lockedClock.RealTime() / refClock.RealTime())<<"% w.r.t. simple)."
                <<" Per access time CPU: "<<timeToString(lockedClock.CpuTime() / nEleAcc)<<", real: "
                <<timeToString(lockedClock.RealTime()  / nEleAcc)<<std::endl;;
    }



    return return_code;
}
