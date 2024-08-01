/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGINDETCUDA_SEEDMAKINGWORKCUDA_H
#define TRIGINDETCUDA_SEEDMAKINGWORKCUDA_H


#include "TrigAccelEvent/Work.h" //base class
#include "CommonStructures.h" //for WorkTimeStamp

#include <cstdio> // for printf
#include <memory> // for shared_ptr

class SeedMakingDeviceContext;
class SeedMakingManagedDeviceContext;

class SeedMakingWorkCuda : public TrigAccel::Work{

public:
  SeedMakingWorkCuda(unsigned int, SeedMakingDeviceContext*, std::shared_ptr<TrigAccel::OffloadBuffer>, 
  WorkTimeStampQueue*);
  ~SeedMakingWorkCuda();
  std::shared_ptr<TrigAccel::OffloadBuffer> getOutput();
  bool run();
  unsigned int getId() const {
    return m_workId;
  }
  
private:

  inline void checkError() const {
    cudaError_t error = cudaGetLastError();
    if(error != cudaSuccess) {
      printf("CUDA error: %s\n", cudaGetErrorString(error));
      exit(-1);
    }
  };
  
  unsigned int m_workId;
  SeedMakingDeviceContext* m_context;  
  std::shared_ptr<TrigAccel::OffloadBuffer> m_input, m_output;
  WorkTimeStampQueue* m_timeLine;

  float m_CovMS, m_ptCoeff, m_minPt2, m_ptCoeff2, m_maxD0; 
};

class SeedMakingWorkCudaManaged : public TrigAccel::Work{

public:
  SeedMakingWorkCudaManaged(unsigned int, SeedMakingManagedDeviceContext*, std::shared_ptr<TrigAccel::OffloadBuffer>, 
  WorkTimeStampQueue*);
  ~SeedMakingWorkCudaManaged();
  std::shared_ptr<TrigAccel::OffloadBuffer> getOutput();
  bool run();
  unsigned int getId() const {
    return m_workId;
  }
  
private:

  inline void checkError() const {
    cudaError_t error = cudaGetLastError();
    if(error != cudaSuccess) {
      printf("CUDA error: %s\n", cudaGetErrorString(error));
      exit(-1);
    }
  };
  
  unsigned int m_workId;
  SeedMakingManagedDeviceContext* m_context;  
  std::shared_ptr<TrigAccel::OffloadBuffer> m_input, m_output;
  WorkTimeStampQueue* m_timeLine;

  float m_CovMS{}, m_ptCoeff{}, m_minPt2{}, m_ptCoeff2{}, m_maxD0{}; 
};

#endif
