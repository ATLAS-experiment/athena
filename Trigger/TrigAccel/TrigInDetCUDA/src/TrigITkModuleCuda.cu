/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include <cuda.h>
#include <cuda_runtime.h>
#include <cuda_fp16.h>

#include <atomic>

#include "device_context.h"
#include "TrigITkModuleCuda.h"
#include "SeedMakingDataStructures_ITk.h"
#include "SeedMakingWorkCuda_ITk.h"
#include "GbtsWorkCuda_ITk.h"

#include "TrigAccelEvent/TrigInDetAccelCodes.h"
#include "gpu_helpers.h"

#include <sstream>


TrigITkModuleCuda::TrigITkModuleCuda() : m_maxDevice(0), m_dumpTimeLine(false) {

  m_h_detmodel = 0;

  m_maxDevice = GPUHelpers::getNumberOfGPUs();

  for(unsigned int i=0;i<getProvidedAlgs().size();i++) {
    m_workItemCounters[i] = 0;
  }

  m_timeLine.clear();

}

TrigITkModuleCuda::~TrigITkModuleCuda() {

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

bool TrigITkModuleCuda::configure() {

  if(m_maxDevice == 0) {
     std::cout<<"No CUDA devices found"<<std::endl;
     return false;
  }
  return true;
}

SeedMakingDeviceContext* TrigITkModuleCuda::createSeedMakingContext(int id) const {

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
  p->m_gpuParams.m_nNUM_TRIPLET_BLOCKS = NUM_TRIPLET_BLOCKS_ITk;
  if(deviceProp.maxThreadsPerBlock < p->m_gpuParams.m_nNUM_TRIPLET_BLOCKS) 
    p->m_gpuParams.m_nNUM_TRIPLET_BLOCKS = deviceProp.maxThreadsPerBlock;

  //Allocate memory
  
  cudaMalloc((void **)&p->d_settings,    sizeof(TrigAccel::ITk::SEED_FINDER_SETTINGS));
  cudaMalloc((void **)&p->d_spacepoints, sizeof(TrigAccel::ITk::SPACEPOINT_STORAGE));

  checkError();
  cudaMalloc((void **)&p->d_outputseeds, sizeof(TrigAccel::ITk::OUTPUT_SEED_STORAGE));
  cudaMalloc((void **)&p->d_doubletstorage, sizeof(DOUBLET_STORAGE_ITk));
  cudaMalloc((void **)&p->d_doubletinfo, sizeof(DOUBLET_INFO_ITk));
  checkError(13);

  p->d_size = sizeof(TrigAccel::ITk::SEED_FINDER_SETTINGS) +  
              sizeof(TrigAccel::ITk::SPACEPOINT_STORAGE) + sizeof(TrigAccel::ITk::OUTPUT_SEED_STORAGE) + sizeof(DOUBLET_STORAGE_ITk) + sizeof(DOUBLET_INFO_ITk) + 
              sizeof(TrigAccel::ITk::DETECTOR_MODEL);
  
  cudaMallocHost((void **)&p->h_settings, sizeof(TrigAccel::ITk::SEED_FINDER_SETTINGS));
  cudaMallocHost((void **)&p->h_spacepoints, sizeof(TrigAccel::ITk::SPACEPOINT_STORAGE));
  cudaMallocHost((void **)&p->h_outputseeds, sizeof(TrigAccel::ITk::OUTPUT_SEED_STORAGE));

  p->h_size = sizeof(TrigAccel::ITk::SEED_FINDER_SETTINGS) + sizeof(TrigAccel::ITk::SPACEPOINT_STORAGE) + sizeof(TrigAccel::ITk::OUTPUT_SEED_STORAGE);

  p->d_detmodel = m_d_detmodel_ptrs.at(id);
  
  checkError(14);
  return p;
}

SeedMakingManagedDeviceContext* TrigITkModuleCuda::createManagedSeedMakingContext(int id) const {

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
  p->m_gpuParams.m_nNUM_TRIPLET_BLOCKS = NUM_TRIPLET_BLOCKS_ITk;
  if(deviceProp.maxThreadsPerBlock < p->m_gpuParams.m_nNUM_TRIPLET_BLOCKS) 
    p->m_gpuParams.m_nNUM_TRIPLET_BLOCKS = deviceProp.maxThreadsPerBlock;

  //Allocate memory
  
  cudaMallocManaged((void **)&p->m_settings,    sizeof(TrigAccel::ITk::SEED_FINDER_SETTINGS));
  cudaMallocManaged((void **)&p->m_spacepoints, sizeof(TrigAccel::ITk::SPACEPOINT_STORAGE));
  cudaMallocManaged((void **)&p->m_outputseeds, sizeof(TrigAccel::ITk::OUTPUT_SEED_STORAGE));
  cudaMallocManaged((void **)&p->m_confirmedseeds, sizeof(TrigAccel::ITk::OUTPUT_SEED_STORAGE));


  checkError();
  cudaMalloc((void **)&p->d_doubletstorage, sizeof(DOUBLET_STORAGE_ITk));
  cudaMalloc((void **)&p->d_doubletinfo, sizeof(DOUBLET_INFO_ITk));
  checkError(13);

  p->d_size = sizeof(DOUBLET_STORAGE_ITk) + sizeof(DOUBLET_INFO_ITk) + sizeof(TrigAccel::ITk::DETECTOR_MODEL);
  
  p->h_size = 0;
  
  p->m_size = sizeof(TrigAccel::ITk::SEED_FINDER_SETTINGS) + sizeof(TrigAccel::ITk::SPACEPOINT_STORAGE) + sizeof(TrigAccel::ITk::OUTPUT_SEED_STORAGE);

  p->d_detmodel = m_d_detmodel_ptrs.at(id);

  checkError(14);
  return p;
}


GbtsDeviceContext* TrigITkModuleCuda::createGbtsContext(int id, const TrigAccel::ITk::GRAPH_MAKING_INPUT_DATA *pData) const {

  cudaSetDevice(id);
  checkError(11);
  
  GbtsDeviceContext* p = new GbtsDeviceContext;

  p->m_deviceId = id;

  //set stream

  cudaStreamCreate(&p->m_stream);
  checkError(12);
  
  //Allocate memory and copy input data

  GbtsDeviceContext& ctx = *p;
	
  ctx.m_useGPUseedExtraction = pData->m_useGPUseedExtraction;

  ctx.m_maxEtaBin = pData->m_maxEtaBin + 1;
  ctx.m_nNodes    = pData->m_nSpacepoints;
  ctx.m_nLayers   = pData->m_nLayers;
  ctx.m_nMaxEdges = pData->m_nMaxEdges;

  ctx.m_minLevel  = pData->m_minLevel;
  //1. spacepoint params storage

  size_t data_size = 4*pData->m_nSpacepoints*sizeof(float);//x,y,z,w

  cudaMalloc((void**) &ctx.d_sp_params, data_size);

  cudaMemcpy(ctx.d_sp_params, &pData->m_params, data_size, cudaMemcpyHostToDevice);

  ctx.d_size += data_size;
	
  //2. layer information: spacepoint views and geometry
	
  data_size = 4*pData->m_nLayers*sizeof(int);

  cudaMalloc((void**) &ctx.d_layer_info, data_size);
  cudaMemcpy(ctx.d_layer_info, pData->m_layerInfo, data_size, cudaMemcpyHostToDevice);

  ctx.d_size += data_size;
    
  data_size = 2*pData->m_nLayers*sizeof(float);

  cudaMalloc((void**) &ctx.d_layer_geo, data_size);
  cudaMemcpy(ctx.d_layer_geo, pData->m_layerGeo, data_size, cudaMemcpyHostToDevice);

  ctx.d_size += data_size;
    
  //4. algorithm parameters
    
  data_size = sizeof(pData->m_algo_params);

  cudaMalloc((void**) &ctx.d_algo_params, data_size);
    
  cudaMemcpy(ctx.d_algo_params, pData->m_algo_params, data_size, cudaMemcpyHostToDevice);
    
  ctx.d_size += data_size;
    
  //5. graph node storage
    
  data_size = 5*pData->m_nSpacepoints*sizeof(float);//tau1, tau2, r, phi, z

  cudaMalloc((void**) &ctx.d_node_params, data_size);

  ctx.d_size += data_size;

  //6. eta-phi binning structures

  data_size = pData->m_nSpacepoints*sizeof(int);

  cudaMalloc((void**) &ctx.d_node_eta_index, data_size);

  ctx.d_size += data_size;

  cudaMalloc((void**) &ctx.d_node_phi_index, data_size);

  ctx.d_size += data_size;

  cudaMalloc((void**) &ctx.d_node_index, data_size);

  ctx.d_size += data_size;

  data_size = ctx.m_maxEtaBin*TrigAccel::ITk::GBTS_MAX_PHI_BIN*sizeof(unsigned int);

  cudaMalloc((void**) &ctx.d_eta_phi_histo, data_size);

  ctx.d_size += data_size;

  cudaMemset(ctx.d_eta_phi_histo, 0, data_size);

  cudaMalloc((void**) &ctx.d_phi_cusums, data_size);

  ctx.d_size += data_size;

  data_size = ctx.m_maxEtaBin*sizeof(unsigned int);

  cudaMalloc((void**) &ctx.d_eta_node_counter, data_size);

  ctx.d_size += data_size;

  data_size = 2*ctx.m_maxEtaBin*sizeof(int);

  cudaMalloc((void**) &ctx.d_eta_bin_views, data_size);

  ctx.d_size += data_size;

  data_size = 2*ctx.m_maxEtaBin*sizeof(float);

  cudaMalloc((void**) &ctx.d_bin_rads, data_size);
  
  ctx.d_size += data_size;

  checkError(13);

  //8. data structure for graph building,  will be allocated later

  ctx.h_bin_rads = 0;
  ctx.h_eta_bin_views = 0;
  ctx.d_bin_pair_views = 0;
  ctx.d_bin_pair_dphi  = 0;
  ctx.d_edge_links = 0;
  ctx.d_num_neighbours = 0;
  ctx.d_reIndexer = 0;
  ctx.d_neighbours = 0;
  ctx.d_output_graph = 0;

 //9. data structure for graph processing

  ctx.d_active_edges = 0;
  ctx.d_levels = 0;
  ctx.d_level_views = 0;
  ctx.d_level_boundaries = 0;

  ctx.d_mini_states = 0;
  ctx.d_seed_proposals = 0;
  ctx.d_state_store = 0;
  ctx.d_edge_bids = 0;
  ctx.d_seed_ambiguity = 0;

  //10. the graph

  data_size = 12*sizeof(unsigned int);

  cudaMalloc((void **)&ctx.d_counters, data_size);
  cudaMemset(ctx.d_counters, 0, data_size);

  ctx.d_size += data_size;
    
  data_size = 2*ctx.m_nMaxEdges*sizeof(int);
  cudaMalloc((void **)&ctx.d_edge_nodes, data_size);

  ctx.d_size += data_size;
    
  data_size = 4*ctx.m_nMaxEdges*sizeof(__half);
  cudaMalloc((void **)&ctx.d_edge_params, data_size);

  ctx.d_size += data_size;

  data_size = (ctx.m_nNodes+1)*sizeof(unsigned int);

  cudaMalloc((void **)&ctx.d_num_incoming_edges, data_size);
  cudaMemset(ctx.d_num_incoming_edges, 0, data_size);

  ctx.d_size += data_size;
  
  data_size = ctx.m_nNodes*sizeof(int);
    
  checkError(14);

  return p;
}

TrigAccel::Work* TrigITkModuleCuda::createWork(int workType, std::shared_ptr<TrigAccel::OffloadBuffer> data){
  
  if(workType == TrigAccel::InDetJobControlCode::SIL_LAYERS_EXPORT){
    unsigned char* d_detmodel;
    int deviceId = 0;//always using device 0 for the time being

    cudaSetDevice(deviceId);
    cudaMalloc(&d_detmodel, sizeof(TrigAccel::ITk::DETECTOR_MODEL));
    checkError();

    m_d_detmodel_ptrs[deviceId] = d_detmodel;

    cudaMemcpy(d_detmodel, (unsigned char*)data->get(), sizeof(TrigAccel::ITk::DETECTOR_MODEL), cudaMemcpyHostToDevice);
    checkError(21);

    return 0;
  }

  if(workType == TrigAccel::InDetJobControlCode::FIND_SEEDS){
 
    int deviceId = 0;//always using device 0 for the time being

    //TO-DO: to support mult-GPU load balancing get deviceId from a tbb_concurrent_queue

    SeedMakingDeviceContext* ctx = createSeedMakingContext(deviceId);

    TrigAccel::ITk::SEED_MAKING_JOB *pArray = reinterpret_cast<TrigAccel::ITk::SEED_MAKING_JOB*>(data->get());
    
    //1. copy settings to the context host array

    TrigAccel::ITk::SEED_FINDER_SETTINGS* p_settings = reinterpret_cast<TrigAccel::ITk::SEED_FINDER_SETTINGS*>(ctx->h_settings);
    memcpy(p_settings, &pArray->m_settings, sizeof(TrigAccel::ITk::SEED_FINDER_SETTINGS));

    //2. copy spacepoints to the context host array

    TrigAccel::ITk::SPACEPOINT_STORAGE* p_spacePoints = reinterpret_cast<TrigAccel::ITk::SPACEPOINT_STORAGE*>(ctx->h_spacepoints);
    memcpy(p_spacePoints, &pArray->m_data, sizeof(TrigAccel::ITk::SPACEPOINT_STORAGE));
    
    unsigned int workNum = m_workItemCounters[0]++;//seed making uses counter #0
    
    unsigned int workId = workNum*100;
    
    SeedMakingWorkCudaITk* w = new SeedMakingWorkCudaITk(workId, ctx, std::move(data), &m_timeLine);
    
    return w;
  }

  if(workType == TrigAccel::InDetJobControlCode::MAKE_SEEDS){
 
    int deviceId = 0;//always using device 0 for the time being

    //TO-DO: to support mult-GPU load balancing get deviceId from a tbb_concurrent_queue

    SeedMakingManagedDeviceContext* ctx = createManagedSeedMakingContext(deviceId);

    TrigAccel::ITk::SEED_MAKING_JOB *pArray = reinterpret_cast<TrigAccel::ITk::SEED_MAKING_JOB*>(data->get());
    
    //1. copy settings to the context host array

    TrigAccel::ITk::SEED_FINDER_SETTINGS* p_settings = reinterpret_cast<TrigAccel::ITk::SEED_FINDER_SETTINGS*>(ctx->m_settings);
    memcpy(p_settings, &pArray->m_settings, sizeof(TrigAccel::ITk::SEED_FINDER_SETTINGS));

    //2. copy spacepoints to the context host array

    TrigAccel::ITk::SPACEPOINT_STORAGE* p_spacePoints = reinterpret_cast<TrigAccel::ITk::SPACEPOINT_STORAGE*>(ctx->m_spacepoints);
    memcpy(p_spacePoints, &pArray->m_data, sizeof(TrigAccel::ITk::SPACEPOINT_STORAGE));
    
    unsigned int workNum = m_workItemCounters[0]++;//seed making uses counter #0
    
    unsigned int workId = workNum*100;
    
    SeedMakingWorkCudaManagedITk* w = new SeedMakingWorkCudaManagedITk(workId, ctx, std::move(data), &m_timeLine);
    
    return w;
  }

  if(workType == TrigAccel::InDetJobControlCode::RUN_GBTS){

    int deviceId = 0;//always using device 0 for the time being

    //TO-DO: to support mult-GPU load balancing get deviceId from a tbb_concurrent_queue

    TrigAccel::ITk::GRAPH_MAKING_INPUT_DATA *pData = reinterpret_cast<TrigAccel::ITk::GRAPH_MAKING_INPUT_DATA*>(data->get());

    GbtsDeviceContext* ctx = createGbtsContext(deviceId, pData);
    
    
    unsigned int workNum = m_workItemCounters[0]++;
    
    unsigned int workId = workNum*100;
    
    GbtsWorkCudaITk* w = new GbtsWorkCudaITk(workId, ctx, std::move(data), &m_timeLine);
    
    return w;
  }

  return 0;
}

const std::vector<int> TrigITkModuleCuda::getProvidedAlgs(){
  std::vector<int> v{
      TrigAccel::InDetJobControlCode::SIL_LAYERS_EXPORT,
      TrigAccel::InDetJobControlCode::MAKE_SEEDS,//the default
      TrigAccel::InDetJobControlCode::FIND_SEEDS, //the alternative
      TrigAccel::InDetJobControlCode::RUN_GBTS
  };
  return v;
}

