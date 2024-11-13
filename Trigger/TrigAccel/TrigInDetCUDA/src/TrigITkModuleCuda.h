/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGINDETCUDA_TRIGITkMODULECUDA_H
#define TRIGINDETCUDA_TRIGITkMODULECUDA_H

#include <map>
#include <atomic>
#include <unistd.h>
#include "TrigAccelEvent/WorkFactory.h"
#include "TrigAccelEvent/TrigITkAccelEDM.h"
#include "TrigAccelEvent/TrigInDetAccelCodes.h"

#include "device_context.h"

#include "tbb/concurrent_queue.h"
#include "tbb/concurrent_vector.h"

class TrigITkModuleCuda : public TrigAccel::WorkFactory {

 public:

  TrigITkModuleCuda();

  ~TrigITkModuleCuda();

  bool configure();

  TrigAccel::Work* createWork(int, std::shared_ptr<TrigAccel::OffloadBuffer>);

  const std::vector<int> getProvidedAlgs();

  virtual int getFactoryId() {
    return TrigAccel::TrigITkModuleID_CUDA;
  }
    
  private:

    inline void checkError(int code = 0) const {
      cudaError_t error = cudaGetLastError();
      if(error != cudaSuccess) {
        printf("%d CUDA error %d: %s\n", getpid(), code, cudaGetErrorString(error));
        exit(-1);
      }
    };
    
    //data structures

    //1. "const" data: managed by the Factory

    unsigned char* m_h_detmodel;

    std::map<int, unsigned char*> m_d_detmodel_ptrs;
    
    int m_maxDevice;

    // data context allocation / de-allocation
    
    SeedMakingDeviceContext* createSeedMakingContext(int) const;


    SeedMakingManagedDeviceContext* createManagedSeedMakingContext(int) const;


    bool m_dumpTimeLine;
    
    std::atomic<unsigned int> m_workItemCounters[100];//atomic counters for unique Work identification
    tbb::concurrent_vector<WorkTimeStamp> m_timeLine;
  };

#endif
