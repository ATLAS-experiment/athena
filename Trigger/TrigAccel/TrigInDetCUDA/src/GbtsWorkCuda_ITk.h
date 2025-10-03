/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGINDETCUDA_GBTSWORKCUDA_ITK_H
#define TRIGINDETCUDA_GBTSWORKCUDA_ITK_H



#include "TrigAccelEvent/Work.h" //base class

#include <cstdio> // for printf
#include <memory> // for shared_ptr and unique_ptr

class GbtsDeviceContext;
class WorkTimeStampQueue;

class GbtsWorkCudaITk : public TrigAccel::Work{

public:
  GbtsWorkCudaITk(unsigned int, GbtsDeviceContext*, std::shared_ptr<TrigAccel::OffloadBuffer>, 
  WorkTimeStampQueue*);
  ~GbtsWorkCudaITk();
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
  GbtsDeviceContext* m_context;  
  std::shared_ptr<TrigAccel::OffloadBuffer> m_input, m_output;
  WorkTimeStampQueue* m_timeLine;

};

#endif
