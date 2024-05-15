/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGINDETCUDA_DEVCONTEXTS_H
#define TRIGINDETCUDA_DEVCONTEXTS_H

#include "TrigAccelEvent/TrigInDetAccelEDM.h"

//#include "SeedMakingDataStructures.h"
#include <cuda_runtime.h>
#include <tbb/tick_count.h>

#include "CommonStructures.h"


struct SeedMakingDeviceContext {
public:
  SeedMakingDeviceContext() : m_deviceId(-1), h_spacepoints(0), d_spacepoints(0), d_size(0), h_size(0) {};
  size_t hostSize() { return h_size;}
  size_t deviceSize() { return d_size;}
  
  int m_deviceId;
  cudaStream_t m_stream;
  unsigned char *h_settings;
  unsigned char *d_settings;
  unsigned char *h_spacepoints;
  unsigned char *d_spacepoints;
  
  unsigned char *d_detmodel;
  
  unsigned char *h_outputseeds;
  unsigned char *d_outputseeds;
  
  unsigned char *d_doubletstorage;
  unsigned char *d_doubletinfo;
  
  size_t d_size, h_size;
  GPU_PARAMETERS m_gpuParams;
  
private:
  SeedMakingDeviceContext(const SeedMakingDeviceContext& sc) : m_deviceId(sc.m_deviceId) {};
};

struct SeedMakingManagedDeviceContext {
public:
SeedMakingManagedDeviceContext() : m_deviceId(-1), m_spacepoints(0), d_size(0), h_size(0), m_size(0) {};
  size_t hostSize() { return h_size;}
  size_t deviceSize() { return d_size;}
  size_t managedSize() { return m_size;}
  
  int m_deviceId;
  cudaStream_t m_stream;


  unsigned char *m_settings;


  unsigned char *m_spacepoints;

  unsigned char *d_detmodel;
  



  unsigned char *m_outputseeds;
  unsigned char *m_confirmedseeds;
  
  unsigned char *d_doubletstorage;
  unsigned char *d_doubletinfo;
  
  size_t d_size, h_size, m_size;
  GPU_PARAMETERS m_gpuParams;
  
private:
  SeedMakingManagedDeviceContext(const SeedMakingManagedDeviceContext& sc) : m_deviceId(sc.m_deviceId) {};
};




#endif
