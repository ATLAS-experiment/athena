/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include <cuda.h>
#include <cuda_runtime.h>
#include <atomic>
#include <unistd.h>

#include "TrigInDetModuleCuda.h"
#include "SeedMakingDataStructures.h"
#include "SeedMakingWorkCuda.h"

#include "TrigAccelEvent/TrigInDetAccelCodes.h"

#include <sstream>
#include "gpu_helpers.h"

TrigInDetModuleCuda::TrigInDetModuleCuda() : m_maxDevice(0), m_dumpTimeLine(false) {

  m_h_detmodel = 0;

  m_maxDevice = GPUHelpers::getNumberOfGPUs();

  for(unsigned int i=0;i<getProvidedAlgs().size();i++) {
    m_workItemCounters[i] = 0;
  }

  m_timeLine.clear();

}

TrigInDetModuleCuda::~TrigInDetModuleCuda() {
  if(m_dumpTimeLine) {
    if(m_timeLine.size() > 0) {
       tbb::tick_count t0 = m_timeLine[0].m_time;
       std::ostringstream fileName;
       fileName <<"timeLine_"<<getpid()<<".csv";
       std::ofstream tl(fileName.str());
       tl<<"workId,eventType,time"<<std::endl;
       tl<<m_timeLine[0].m_workId<<","<<m_timeLine[0].m_eventType<<",0"<<std::endl;
       for(unsigned int tIdx = 1;tIdx < m_timeLine.size();++tIdx) {
          tbb::tick_count t1 = m_timeLine[tIdx].m_time;
          auto duration = t1-t0;
          tl<<m_timeLine[tIdx].m_workId<<","<<m_timeLine[tIdx].m_eventType<<","<<1000*duration.seconds()<<std::endl;
       }
       tl.close();
      m_timeLine.clear();
    }
 }

 for(auto pair: m_d_detmodel_ptrs){
  cudaFree(pair.second);
 }
}

bool TrigInDetModuleCuda::configure() {

  if(m_maxDevice == 0) {
     std::cout<<"No CUDA devices found"<<std::endl;
     return false;
  }
  return true;
}

SeedMakingDeviceContext* TrigInDetModuleCuda::createSeedMakingContext(int id) const {

  cudaSetDevice(id);
  checkError(11);
  SeedMakingDeviceContext* p = new SeedMakingDeviceContext;

  p->m_deviceId = id;

  //set stream

  cudaStreamCreate(&p->m_stream);
  checkError(12);
  //check device property

  cudaDeviceProp deviceProp;
  cudaGetDeviceProperties(&deviceProp, id);

  p->m_gpuParams.m_nSMX = deviceProp.multiProcessorCount;

  int ncores = GPUHelpers::getNumberOfCores(deviceProp.major, deviceProp.minor);
  
  p->m_gpuParams.m_nNUM_SMX_CORES = ncores;//_ConvertSMVer2Cores_local(deviceProp.major, deviceProp.minor);
  p->m_gpuParams.m_nNUM_TRIPLET_BLOCKS = NUM_TRIPLET_BLOCKS;
  if(deviceProp.maxThreadsPerBlock < p->m_gpuParams.m_nNUM_TRIPLET_BLOCKS) 
    p->m_gpuParams.m_nNUM_TRIPLET_BLOCKS = deviceProp.maxThreadsPerBlock;

  //Allocate memory
  
  cudaMalloc((void **)&p->d_settings,    sizeof(TrigAccel::SEED_FINDER_SETTINGS));
  cudaMalloc((void **)&p->d_spacepoints, sizeof(TrigAccel::SPACEPOINT_STORAGE));
  checkError();
  cudaMalloc((void **)&p->d_outputseeds, sizeof(TrigAccel::OUTPUT_SEED_STORAGE));
  cudaMalloc((void **)&p->d_doubletstorage, sizeof(DOUBLET_STORAGE));
  cudaMalloc((void **)&p->d_doubletinfo, sizeof(DOUBLET_INFO));
  checkError(13);

  p->d_size = sizeof(TrigAccel::SEED_FINDER_SETTINGS) +  
              sizeof(TrigAccel::SPACEPOINT_STORAGE) + sizeof(TrigAccel::OUTPUT_SEED_STORAGE) + sizeof(DOUBLET_STORAGE) + sizeof(DOUBLET_INFO) + 
              sizeof(TrigAccel::DETECTOR_MODEL);
  
  cudaMallocHost((void **)&p->h_settings, sizeof(TrigAccel::SEED_FINDER_SETTINGS));
  cudaMallocHost((void **)&p->h_spacepoints, sizeof(TrigAccel::SPACEPOINT_STORAGE));
  cudaMallocHost((void **)&p->h_outputseeds, sizeof(TrigAccel::OUTPUT_SEED_STORAGE));

  p->h_size = sizeof(TrigAccel::SEED_FINDER_SETTINGS) + sizeof(TrigAccel::SPACEPOINT_STORAGE) + sizeof(TrigAccel::OUTPUT_SEED_STORAGE);
  
  p->d_detmodel = m_d_detmodel_ptrs.at(id); //get the detmodel pointer for the selected device

  checkError(14);
  return p;
}

SeedMakingManagedDeviceContext* TrigInDetModuleCuda::createManagedSeedMakingContext(int id) const {

  cudaSetDevice(id);
  checkError(11);
  SeedMakingManagedDeviceContext* p = new SeedMakingManagedDeviceContext;

  p->m_deviceId = id;

  //set stream

  cudaStreamCreate(&p->m_stream);
  checkError(12);
  //check device property

  cudaDeviceProp deviceProp;
  cudaGetDeviceProperties(&deviceProp, id);

  p->m_gpuParams.m_nSMX = deviceProp.multiProcessorCount;

  int ncores = GPUHelpers::getNumberOfCores(deviceProp.major, deviceProp.minor);

  p->m_gpuParams.m_nNUM_SMX_CORES = ncores;//_ConvertSMVer2Cores_local(deviceProp.major, deviceProp.minor);
  p->m_gpuParams.m_nNUM_TRIPLET_BLOCKS = NUM_TRIPLET_BLOCKS;
  if(deviceProp.maxThreadsPerBlock < p->m_gpuParams.m_nNUM_TRIPLET_BLOCKS) 
    p->m_gpuParams.m_nNUM_TRIPLET_BLOCKS = deviceProp.maxThreadsPerBlock;

  //Allocate memory
  
  cudaMallocManaged((void **)&p->m_settings,    sizeof(TrigAccel::SEED_FINDER_SETTINGS));
  cudaMallocManaged((void **)&p->m_spacepoints, sizeof(TrigAccel::SPACEPOINT_STORAGE));
  cudaMallocManaged((void **)&p->m_outputseeds, sizeof(TrigAccel::OUTPUT_SEED_STORAGE));

  checkError();
  cudaMalloc((void **)&p->d_doubletstorage, sizeof(DOUBLET_STORAGE));
  cudaMalloc((void **)&p->d_doubletinfo, sizeof(DOUBLET_INFO));
  checkError(13);

  p->d_size = sizeof(DOUBLET_STORAGE) + sizeof(DOUBLET_INFO) + sizeof(TrigAccel::DETECTOR_MODEL);
  
  p->h_size = 0;
  
  p->m_size = sizeof(TrigAccel::SEED_FINDER_SETTINGS) + sizeof(TrigAccel::SPACEPOINT_STORAGE) + sizeof(TrigAccel::OUTPUT_SEED_STORAGE);

  p->d_detmodel = m_d_detmodel_ptrs.at(id); //get the detmodel pointer for the selected device

  checkError(14);
  return p;
}

TrigAccel::Work* TrigInDetModuleCuda::createWork(int workType, std::shared_ptr<TrigAccel::OffloadBuffer> data){
  if(workType == TrigAccel::InDetJobControlCode::SIL_LAYERS_EXPORT){
    unsigned char* d_detmodel;
    int deviceId = 0;//always using device 0 for the time being

    cudaSetDevice(deviceId);
    cudaMalloc(&d_detmodel, sizeof(TrigAccel::DETECTOR_MODEL));
    checkError();

    m_d_detmodel_ptrs[deviceId] = d_detmodel;

    cudaMemcpy(d_detmodel, (unsigned char*)data->get(), sizeof(TrigAccel::DETECTOR_MODEL), cudaMemcpyHostToDevice);
    checkError(21);

    return 0;
  }

  if(workType == TrigAccel::InDetJobControlCode::FIND_SEEDS){
 
    int deviceId = 0;//always using device 0 for the time being

    //TO-DO: to support mult-GPU load balancing get deviceId from a tbb_concurrent_queue

    SeedMakingDeviceContext* ctx = createSeedMakingContext(deviceId);

    TrigAccel::SEED_MAKING_JOB *pArray = reinterpret_cast<TrigAccel::SEED_MAKING_JOB*>(data->get());
    
    //1. copy settings to the context host array

    TrigAccel::SEED_FINDER_SETTINGS* p_settings = reinterpret_cast<TrigAccel::SEED_FINDER_SETTINGS*>(ctx->h_settings);
    memcpy(p_settings, &pArray->m_settings, sizeof(TrigAccel::SEED_FINDER_SETTINGS));

    //2. copy spacepoints to the context host array

    TrigAccel::SPACEPOINT_STORAGE* p_spacePoints = reinterpret_cast<TrigAccel::SPACEPOINT_STORAGE*>(ctx->h_spacepoints);
    memcpy(p_spacePoints, &pArray->m_data, sizeof(TrigAccel::SPACEPOINT_STORAGE));
    
    unsigned int workNum = m_workItemCounters[0]++;//seed making uses counter #0
    
    unsigned int workId = workNum*100;
    
    SeedMakingWorkCuda* w = new SeedMakingWorkCuda(workId, ctx, std::move(data), &m_timeLine);
    
    return w;
  }

  if(workType == TrigAccel::InDetJobControlCode::MAKE_SEEDS){
 
    int deviceId = 0;//always using device 0 for the time being

    //TO-DO: to support mult-GPU load balancing get deviceId from a tbb_concurrent_queue

    SeedMakingManagedDeviceContext* ctx = createManagedSeedMakingContext(deviceId);

    TrigAccel::SEED_MAKING_JOB *pArray = reinterpret_cast<TrigAccel::SEED_MAKING_JOB*>(data->get());
    
    //1. copy settings to the context host array

    TrigAccel::SEED_FINDER_SETTINGS* p_settings = reinterpret_cast<TrigAccel::SEED_FINDER_SETTINGS*>(ctx->m_settings);
    memcpy(p_settings, &pArray->m_settings, sizeof(TrigAccel::SEED_FINDER_SETTINGS));

    //2. copy spacepoints to the context host array

    TrigAccel::SPACEPOINT_STORAGE* p_spacePoints = reinterpret_cast<TrigAccel::SPACEPOINT_STORAGE*>(ctx->m_spacepoints);
    memcpy(p_spacePoints, &pArray->m_data, sizeof(TrigAccel::SPACEPOINT_STORAGE));
    
    unsigned int workNum = m_workItemCounters[0]++;//seed making uses counter #0
    
    unsigned int workId = workNum*100;
    
    SeedMakingWorkCudaManaged* w = new SeedMakingWorkCudaManaged(workId, ctx, std::move(data), &m_timeLine);
    
    return w;
  }

  return 0;
}

const std::vector<int> TrigInDetModuleCuda::getProvidedAlgs(){
  std::vector<int> v{
      TrigAccel::InDetJobControlCode::SIL_LAYERS_EXPORT,
      TrigAccel::InDetJobControlCode::MAKE_SEEDS,//the default
      TrigAccel::InDetJobControlCode::FIND_SEEDS //the alternative
  };
  return v;
}

