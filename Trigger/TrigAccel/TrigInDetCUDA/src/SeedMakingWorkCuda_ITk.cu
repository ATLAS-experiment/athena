/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include <cuda.h>
#include <cuda_runtime.h>

#include "SeedMakingDataStructures_ITk.h"
#include "TrigAccelEvent/TrigITkAccelEDM.h"
#include "SeedMakingWorkCuda_ITk.h"

#include "device_context.h" //for SeedMakingDeviceContext
#include "tbb/tick_count.h"
#include <cstring>
#include <cmath>
#include <iostream>
#include <algorithm>

#include "DoubletCountingKernelCuda_ITk.cuh"
#include "DoubletMakingKernelCuda_ITk.cuh"
#include "DoubletMatchingKernelCuda_ITk.cuh"
#include "TripletConfirmationKernelCuda_ITk.cuh"

SeedMakingWorkCudaITk::SeedMakingWorkCudaITk(unsigned int id, SeedMakingDeviceContext* ctx, std::shared_ptr<TrigAccel::OffloadBuffer> data, 
  WorkTimeStampQueue* TL) : 
  m_workId(id),
  m_context(ctx), 
  m_input(std::move(data)),
  m_timeLine(TL)
 {
  
  m_output = std::make_shared<TrigAccel::OffloadBuffer>(sizeof(TrigAccel::ITk::OUTPUT_SEED_STORAGE));//output data
}

SeedMakingWorkCudaITk::~SeedMakingWorkCudaITk() {
  
  SeedMakingDeviceContext* p = m_context;

  int id = p->m_deviceId;

  cudaSetDevice(id);

  cudaStreamDestroy(p->m_stream);

  cudaFree(p->d_settings);
  cudaFree(p->d_spacepoints);
  
  cudaFree(p->d_outputseeds);
  cudaFree(p->d_doubletstorage);
  cudaFree(p->d_doubletinfo);

  cudaFreeHost(p->h_settings);
  cudaFreeHost(p->h_spacepoints);
  cudaFreeHost(p->h_outputseeds);

  delete p;
  m_context = 0;
}

std::shared_ptr<TrigAccel::OffloadBuffer> SeedMakingWorkCudaITk::getOutput() {
  return m_output;
}

bool SeedMakingWorkCudaITk::run() {

  m_timeLine->push_back(WorkTimeStamp(m_workId, 0, tbb::tick_count::now()));

  const SeedMakingDeviceContext& p = *m_context;
  
  int id = p.m_deviceId;  
  
  TrigAccel::ITk::OUTPUT_SEED_STORAGE* ps = reinterpret_cast<TrigAccel::ITk::OUTPUT_SEED_STORAGE*>(p.h_outputseeds);
  
  cudaSetDevice(id);

  checkError();
  
  cudaMemcpyAsync(p.d_settings, p.h_settings, sizeof(TrigAccel::ITk::SEED_FINDER_SETTINGS), cudaMemcpyHostToDevice, p.m_stream);

  checkError();

  cudaMemcpyAsync(p.d_spacepoints, p.h_spacepoints, sizeof(TrigAccel::ITk::SPACEPOINT_STORAGE), cudaMemcpyHostToDevice, p.m_stream);

  checkError();

  cudaStreamSynchronize(p.m_stream);
    
  TrigAccel::ITk::SEED_FINDER_SETTINGS* dSettings  = reinterpret_cast<TrigAccel::ITk::SEED_FINDER_SETTINGS *>(p.d_settings);
  TrigAccel::ITk::SPACEPOINT_STORAGE* dSpacepoints = reinterpret_cast<TrigAccel::ITk::SPACEPOINT_STORAGE *>(p.d_spacepoints);
  TrigAccel::ITk::DETECTOR_MODEL* dDetModel        = reinterpret_cast<TrigAccel::ITk::DETECTOR_MODEL*>(p.d_detmodel);
  TrigAccel::ITk::OUTPUT_SEED_STORAGE* dOutput     = reinterpret_cast<TrigAccel::ITk::OUTPUT_SEED_STORAGE*>(p.d_outputseeds);

  DOUBLET_INFO_ITk* dInfo                         = reinterpret_cast<DOUBLET_INFO_ITk*>(p.d_doubletinfo);
  DOUBLET_STORAGE_ITk* dStorage                   = reinterpret_cast<DOUBLET_STORAGE_ITk*>(p.d_doubletstorage);

  cudaMemset(p.d_outputseeds,0,10*sizeof(int));

  checkError();

  cudaMemset(p.d_doubletstorage,0,3*sizeof(int));

  checkError();
  
  const TrigAccel::ITk::SPACEPOINT_STORAGE* pSPS = reinterpret_cast<const TrigAccel::ITk::SPACEPOINT_STORAGE *>(p.h_spacepoints);
  int nSlices = pSPS->m_nPhiSlices;
  int nLayers = pSPS->m_nLayers;
  
  int nMiddleSp = NUM_MIDDLE_THREADS_ITk;//determines size of the doublet/triplet buffers
  int nOtherSp = OUTER_THREADS_MULTIPLIER_ITk*p.m_gpuParams.m_nNUM_SMX_CORES/NUM_MIDDLE_THREADS_ITk;//the size of the spacepoint buffer

  dim3 gridDimensions(nSlices, nLayers);
  dim3 blockDimensions(nMiddleSp, nOtherSp);

  cudaMemset(p.d_doubletinfo,0,sizeof(DOUBLET_INFO_ITk));

  checkError();

  cudaStreamSynchronize(p.m_stream);

  checkError();

  doubletCountingKernel_ITk<<<gridDimensions, blockDimensions, 0, p.m_stream>>>(dSettings, dSpacepoints, dDetModel, dInfo, nLayers, nSlices);

  cudaStreamSynchronize(p.m_stream);

  checkError();

  doubletMakingKernel_ITk<<<gridDimensions, blockDimensions, 0, p.m_stream>>>(dSettings, dSpacepoints, dDetModel, dOutput, 
    dInfo, dStorage, nLayers, nSlices);

  cudaStreamSynchronize(p.m_stream);

  checkError();

  int nStats[3];

  cudaMemcpy(&nStats[0], p.d_doubletstorage, 3*sizeof(int), cudaMemcpyDeviceToHost);

  
  doubletMatchingKernel_ITk<<<p.m_gpuParams.m_nNUM_TRIPLET_BLOCKS, NUM_TRIPLET_THREADS_ITk, 0, p.m_stream>>>(dSettings, dSpacepoints, dDetModel, dInfo, 
    dStorage,  dOutput, nStats[0]);

  cudaStreamSynchronize(p.m_stream);

  checkError();

  TrigAccel::ITk::OUTPUT_SEED_STORAGE* pOutput = reinterpret_cast<TrigAccel::ITk::OUTPUT_SEED_STORAGE *>(m_output->m_rawBuffer);

  //Read back GPU results

  pOutput->m_nMiddleSps = 0;
  pOutput->m_nSeeds = 0;		
  pOutput->m_nI = 0;
  pOutput->m_nO = 0;

  cudaMemcpyAsync(p.h_outputseeds, p.d_outputseeds, sizeof(TrigAccel::ITk::OUTPUT_SEED_STORAGE), cudaMemcpyDeviceToHost, p.m_stream);

  checkError();
  
  cudaStreamSynchronize(p.m_stream);

  checkError();

  memcpy(pOutput, ps, sizeof(TrigAccel::ITk::OUTPUT_SEED_STORAGE));
  
  m_timeLine->push_back(WorkTimeStamp(m_workId, 1, tbb::tick_count::now()));

  return true;
}

SeedMakingWorkCudaManagedITk::SeedMakingWorkCudaManagedITk(unsigned int id, SeedMakingManagedDeviceContext* ctx, std::shared_ptr<TrigAccel::OffloadBuffer> data, WorkTimeStampQueue* TL) :
  m_workId(id),
  m_context(ctx), 
  m_input(std::move(data)),
  m_timeLine(TL){
  
  m_output = std::make_shared<TrigAccel::OffloadBuffer>(sizeof(TrigAccel::ITk::OUTPUT_SEED_STORAGE));//output data
}

SeedMakingWorkCudaManagedITk::~SeedMakingWorkCudaManagedITk() {
  
  SeedMakingManagedDeviceContext* p = m_context;

  int id = p->m_deviceId;

  cudaSetDevice(id);

  cudaStreamDestroy(p->m_stream);

  cudaFree(p->m_settings);
  cudaFree(p->m_spacepoints);
  
  cudaFree(p->m_outputseeds);
  cudaFree(p->m_confirmedseeds);
  cudaFree(p->d_doubletstorage);
  cudaFree(p->d_doubletinfo);

  delete p;
  m_context = 0;
}

std::shared_ptr<TrigAccel::OffloadBuffer> SeedMakingWorkCudaManagedITk::getOutput() {
  return m_output;
}

bool SeedMakingWorkCudaManagedITk::run() {

  m_timeLine->push_back(WorkTimeStamp(m_workId, 0, tbb::tick_count::now()));

  const SeedMakingManagedDeviceContext& p = *m_context;
  
  int id = p.m_deviceId;  
  
  TrigAccel::ITk::OUTPUT_SEED_STORAGE* ps = reinterpret_cast<TrigAccel::ITk::OUTPUT_SEED_STORAGE*>(p.m_confirmedseeds);
  
  cudaSetDevice(id);
  checkError();
  
  cudaMemPrefetchAsync(p.m_settings, sizeof(TrigAccel::ITk::SEED_FINDER_SETTINGS), id, p.m_stream);
  checkError();

  cudaMemPrefetchAsync(p.m_spacepoints, sizeof(TrigAccel::ITk::SPACEPOINT_STORAGE), id, p.m_stream);
  checkError();

  cudaStreamSynchronize(p.m_stream);
  TrigAccel::ITk::SEED_FINDER_SETTINGS* dSettings  = reinterpret_cast<TrigAccel::ITk::SEED_FINDER_SETTINGS *>(p.m_settings);
  TrigAccel::ITk::SPACEPOINT_STORAGE* dSpacepoints = reinterpret_cast<TrigAccel::ITk::SPACEPOINT_STORAGE *>(p.m_spacepoints);
  TrigAccel::ITk::DETECTOR_MODEL* dDetModel        = reinterpret_cast<TrigAccel::ITk::DETECTOR_MODEL*>(p.d_detmodel);
  TrigAccel::ITk::OUTPUT_SEED_STORAGE* dOutput     = reinterpret_cast<TrigAccel::ITk::OUTPUT_SEED_STORAGE*>(p.m_outputseeds);
  TrigAccel::ITk::OUTPUT_SEED_STORAGE* dConfirmed  = reinterpret_cast<TrigAccel::ITk::OUTPUT_SEED_STORAGE*>(p.m_confirmedseeds);

  DOUBLET_INFO_ITk* dInfo                         = reinterpret_cast<DOUBLET_INFO_ITk*>(p.d_doubletinfo);
  DOUBLET_STORAGE_ITk* dStorage                   = reinterpret_cast<DOUBLET_STORAGE_ITk*>(p.d_doubletstorage);

  cudaMemset(p.m_outputseeds,0,10*sizeof(int));
  checkError();

  cudaMemset(p.m_confirmedseeds,0,10*sizeof(int));
  checkError();

  cudaMemset(p.d_doubletstorage,0,3*sizeof(int));
  checkError();
  
  const TrigAccel::ITk::SPACEPOINT_STORAGE* pSPS = reinterpret_cast<const TrigAccel::ITk::SPACEPOINT_STORAGE *>(p.m_spacepoints);
  int nSlices = pSPS->m_nPhiSlices;
  int nMiddleLayers = pSPS->m_nMiddleLayers; // The first two kernels are launched for middle spacepoints
  int nLayers = pSPS->m_nLayers;
  
  int nMiddleSp = NUM_MIDDLE_THREADS_ITk;//determines size of the doublet/triplet buffers
  int nOtherSp = OUTER_THREADS_MULTIPLIER_ITk*p.m_gpuParams.m_nNUM_SMX_CORES/NUM_MIDDLE_THREADS_ITk;//the size of the spacepoint buffer

  dim3 gridDimensions(nSlices, nMiddleLayers);
  dim3 blockDimensions(nMiddleSp, nOtherSp);

  cudaMemset(p.d_doubletinfo,0,sizeof(DOUBLET_INFO_ITk));

  checkError();

  cudaStreamSynchronize(p.m_stream);

  checkError();

  doubletCountingKernel_ITk<<<gridDimensions, blockDimensions, 0, p.m_stream>>>(dSettings, dSpacepoints, dDetModel, dInfo, nLayers, nSlices);
  cudaStreamSynchronize(p.m_stream);

  checkError();

  doubletMakingKernel_ITk<<<gridDimensions, blockDimensions, 0, p.m_stream>>>(dSettings, dSpacepoints, dDetModel, dOutput, 
    dInfo, dStorage, nLayers, nSlices);
  cudaStreamSynchronize(p.m_stream);

  checkError();

  int nStats[3];

  cudaMemcpy(&nStats[0], p.d_doubletstorage, 3*sizeof(int), cudaMemcpyDeviceToHost);

  doubletMatchingKernel_ITk<<<p.m_gpuParams.m_nNUM_TRIPLET_BLOCKS, NUM_TRIPLET_THREADS_ITk, 0, p.m_stream>>>(dSettings, dSpacepoints, dDetModel, dInfo, 
    dStorage,  dOutput, nStats[0]);
  cudaStreamSynchronize(p.m_stream);

  checkError();

  tripletConfirmationKernel_ITk<<<p.m_gpuParams.m_nNUM_TRIPLET_BLOCKS, NUM_TRIPLET_THREADS_ITk, 0, p.m_stream>>>(dSpacepoints, dOutput, dConfirmed, dOutput->m_nSeeds);
  cudaStreamSynchronize(p.m_stream);

  checkError();

  TrigAccel::ITk::OUTPUT_SEED_STORAGE* pOutput = reinterpret_cast<TrigAccel::ITk::OUTPUT_SEED_STORAGE *>(m_output->m_rawBuffer);

  //Read back GPU results

  pOutput->m_nMiddleSps = 0;
  pOutput->m_nSeeds = 0;		
  pOutput->m_nI = 0;
  pOutput->m_nO = 0;


  checkError();

  memcpy(pOutput, ps, sizeof(TrigAccel::ITk::OUTPUT_SEED_STORAGE));
  
  m_timeLine->push_back(WorkTimeStamp(m_workId, 1, tbb::tick_count::now()));

  return true;
}
