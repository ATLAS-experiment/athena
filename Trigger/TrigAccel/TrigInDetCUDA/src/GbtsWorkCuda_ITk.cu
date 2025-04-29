/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include <cuda.h>
#include <cuda_runtime.h>

#include "TrigAccelEvent/TrigITkAccelEDM.h"

#include "GbtsWorkCuda_ITk.h"

#include "device_context.h" //for GbtsDeviceContext
#include "tbb/tick_count.h"
#include <cstring>
#include <cmath>
#include <iostream>
#include <algorithm>

#include "GraphNodesMakingKernelsCuda_ITk.cuh"

#include "GbtsGraphMakingKernelsCuda_ITk.cuh"


GbtsWorkCudaITk::GbtsWorkCudaITk(unsigned int id, GbtsDeviceContext* ctx, std::shared_ptr<TrigAccel::OffloadBuffer> data, 
  WorkTimeStampQueue* TL) : 
  m_workId(id),
  m_context(ctx), 
  m_input(data),
  m_timeLine(TL)
 {
  
  m_output = std::make_shared<TrigAccel::OffloadBuffer>(sizeof(TrigAccel::ITk::COMPRESSED_GRAPH));//output data
}

GbtsWorkCudaITk::~GbtsWorkCudaITk() {
  
  GbtsDeviceContext* p = m_context;

  int id = p->m_deviceId;

  cudaSetDevice(id);

  cudaStreamDestroy(p->m_stream);

  GbtsDeviceContext& ctx = *p;

  cudaFree(ctx.d_node_params);
  cudaFree(ctx.d_node_index);
    
  cudaFree(ctx.d_sp_params);
  cudaFree(ctx.d_algo_params);
  cudaFree(ctx.d_node_eta_index);
  cudaFree(ctx.d_node_phi_index);

  cudaFree(ctx.d_layer_info);
  cudaFree(ctx.d_layer_geo);

  cudaFree(ctx.d_eta_phi_histo);
  cudaFree(ctx.d_eta_node_counter);
  cudaFree(ctx.d_phi_cusums);

  delete[] ctx.h_bin_rads;
  delete[] ctx.h_eta_bin_views;

  delete[] ctx.h_bin_pair_views;
  delete[] ctx.h_bin_pair_dphi;

  cudaFree(ctx.d_eta_bin_views);
  cudaFree(ctx.d_bin_rads);
  
  cudaFree(ctx.d_bin_pair_views);
  cudaFree(ctx.d_bin_pair_dphi);

  cudaFree(ctx.d_counters);
  cudaFree(ctx.d_edge_nodes);

  cudaFree(ctx.d_edge_params);

  cudaFree(ctx.d_num_incoming_edges);

  cudaFree(ctx.d_edge_links);
  cudaFree(ctx.d_link_counters);

  cudaFree(ctx.d_num_neighbours);
  cudaFree(ctx.d_reIndexer);
  cudaFree(ctx.d_neighbours);

  cudaFree(ctx.d_output_graph);

  delete p;
  m_context = 0;
}

std::shared_ptr<TrigAccel::OffloadBuffer> GbtsWorkCudaITk::getOutput() {
  return m_output;
}

bool GbtsWorkCudaITk::run() {

  m_timeLine->push_back(WorkTimeStamp(m_workId, 0, tbb::tick_count::now()));

  const GbtsDeviceContext& ctx = *m_context;
  
  int id = ctx.m_deviceId;  
    
  cudaSetDevice(id);

  checkError();

  //1. create graph nodes and order them by eta and phi

  int nThreads = 256;
  int nNodesPerBlock = nThreads*64;
    
  int nBlocks = (int)(std::ceil((1.0*ctx.m_nNodes)/nNodesPerBlock));

  node_phi_binning_kernel<<<nBlocks, nThreads, 0, ctx.m_stream>>>(reinterpret_cast<const float4*>(ctx.d_sp_params),
  				                 ctx.d_node_phi_index, nNodesPerBlock, ctx.m_nNodes);

  cudaStreamSynchronize(ctx.m_stream);

  cudaError_t error = cudaGetLastError();

  if(error != cudaSuccess) {
      printf("node parameters: CUDA error: %s\n", cudaGetErrorString(error));
      return false;
  }

  nBlocks = ctx.m_nLayers;

  node_eta_binning_kernel<<<nBlocks, nThreads, 0, ctx.m_stream>>>(reinterpret_cast<const float4*>(ctx.d_sp_params), reinterpret_cast<const int4*>(ctx.d_layer_info),
                                                 reinterpret_cast<const float2*>(ctx.d_layer_geo), ctx.d_node_eta_index, ctx.m_nLayers);
  cudaStreamSynchronize(ctx.m_stream);

  error = cudaGetLastError();

  if(error != cudaSuccess) {
      printf("eta binning: CUDA error: %s\n", cudaGetErrorString(error));
      return false;
  }

  nBlocks = (int)(std::ceil((1.0*ctx.m_nNodes)/nNodesPerBlock));

  eta_phi_histo_kernel<<<nBlocks, nThreads, 0, ctx.m_stream>>>(ctx.d_node_phi_index, ctx.d_node_eta_index, ctx.d_eta_phi_histo, nNodesPerBlock, ctx.m_nNodes);

  cudaStreamSynchronize(ctx.m_stream);
 
  error = cudaGetLastError();

  if(error != cudaSuccess) {
      printf("eta-phi histo: CUDA error: %s\n", cudaGetErrorString(error));
      return false;
  }

  int nBinsPerBlock = 128;
    
  nThreads = nBinsPerBlock;

  nBlocks = (int)(std::ceil((1.0*ctx.m_maxEtaBin)/nBinsPerBlock));

  eta_phi_counting_kernel<<<nBlocks, nThreads, 0, ctx.m_stream>>>(ctx.d_eta_phi_histo, ctx.d_eta_node_counter, ctx.d_phi_cusums, nBinsPerBlock, ctx.m_maxEtaBin);

  cudaStreamSynchronize(ctx.m_stream);

  error = cudaGetLastError();

  if(error != cudaSuccess) {
      printf("eta-phi counting: CUDA error: %s\n", cudaGetErrorString(error));
      return false;
  }

  unsigned int* eta_sums = new unsigned int[ctx.m_maxEtaBin];

  cudaMemcpyAsync(&eta_sums[0], &ctx.d_eta_node_counter[0], sizeof(int)*ctx.m_maxEtaBin, cudaMemcpyDeviceToHost, ctx.m_stream);

  cudaStreamSynchronize(ctx.m_stream);

  for(int k=1;k<ctx.m_maxEtaBin;k++) eta_sums[k] += eta_sums[k-1];

  //send back

  cudaMemcpyAsync(&ctx.d_eta_node_counter[0], &eta_sums[0], sizeof(int)*ctx.m_maxEtaBin, cudaMemcpyHostToDevice, ctx.m_stream);

  int* eta_bin_views = new int[2*ctx.m_maxEtaBin];

  for(int view_idx = 0; view_idx < ctx.m_maxEtaBin; view_idx++) {
        int pos = 2*view_idx;
        eta_bin_views[pos]   = (view_idx == 0) ? 0 : eta_sums[view_idx-1];
        eta_bin_views[pos+1] = eta_sums[view_idx];
  }

  delete[] eta_sums;

  cudaStreamSynchronize(ctx.m_stream);

  eta_phi_prefix_sum_kernel<<<nBlocks, nThreads, 0, ctx.m_stream>>>(ctx.d_eta_phi_histo, ctx.d_eta_node_counter, ctx.d_phi_cusums, nBinsPerBlock, ctx.m_maxEtaBin);

  cudaStreamSynchronize(ctx.m_stream);

  error = cudaGetLastError();

  if(error != cudaSuccess) {
      printf("eta-phi cusum: CUDA error: %s\n", cudaGetErrorString(error));
      return false;
  }

  nThreads = 256;
  nNodesPerBlock = nThreads*64;
    
  nBlocks = (int)(std::ceil((1.0*ctx.m_nNodes)/nNodesPerBlock));

  node_sorting_kernel<<<nBlocks, nThreads, 0, ctx.m_stream>>>(reinterpret_cast<const float4*>(ctx.d_sp_params), ctx.d_node_eta_index, ctx.d_node_phi_index, 
                                               ctx.d_phi_cusums, ctx.d_node_params, ctx.d_node_index, nNodesPerBlock, ctx.m_nNodes);

  cudaStreamSynchronize(ctx.m_stream);

  error = cudaGetLastError();

  if(error != cudaSuccess) {
      printf("node sorting: CUDA error: %s\n", cudaGetErrorString(error));
      return false;
  }

  cudaMemcpyAsync(&ctx.d_eta_bin_views[0], &eta_bin_views[0], 2*ctx.m_maxEtaBin*sizeof(int), cudaMemcpyHostToDevice, ctx.m_stream);

  cudaStreamSynchronize(ctx.m_stream);

  nBinsPerBlock = 128;
    
  nThreads = nBinsPerBlock;

  nBlocks = (int)(std::ceil((1.0*ctx.m_maxEtaBin)/nBinsPerBlock));

  minmax_rad_kernel<<<nBlocks, nThreads, 0, ctx.m_stream>>>(reinterpret_cast<const int2*>(ctx.d_eta_bin_views), ctx.d_node_params,
  			       		    		    reinterpret_cast<float2*>(ctx.d_bin_rads), nBinsPerBlock, ctx.m_maxEtaBin);
    

  cudaStreamSynchronize(ctx.m_stream);

  error = cudaGetLastError();

  if(error != cudaSuccess) {
      printf("node sorting: CUDA error: %s\n", cudaGetErrorString(error));
      return false;
  }

  float* bin_rads = new float[2*ctx.m_maxEtaBin];

  cudaMemcpyAsync(&bin_rads[0], &ctx.d_bin_rads[0], 2*sizeof(float)*ctx.m_maxEtaBin, cudaMemcpyDeviceToHost, ctx.m_stream);

  cudaStreamSynchronize(ctx.m_stream);

  m_context->h_bin_rads = bin_rads;
  m_context->h_eta_bin_views = eta_bin_views;

  //2. prepare input for the graph making part of the code:

  TrigAccel::ITk::GRAPH_MAKING_INPUT_DATA *pInput = reinterpret_cast<TrigAccel::ITk::GRAPH_MAKING_INPUT_DATA*>(m_input->get());

  unsigned int nBinPairs = 0;//the number of eta bin pairs

  for(unsigned int pIdx = 0;pIdx < pInput->m_nBinPairs; pIdx++) {//loop over bin pairs defined by the layer connection table and geometry settings
  
        int bin1       = pInput->m_bin_pairs[2*pIdx];
	
        int bin1_begin = ctx.h_eta_bin_views[2*bin1];
        int bin1_end   = ctx.h_eta_bin_views[2*bin1+1];

 	//large bins will be split into smaller sub-views

	unsigned int nNodesInBin1 = bin1_end - bin1_begin;

        nBinPairs += (int)(std::ceil((1.0*nNodesInBin1)/TrigAccel::ITk::GBTS_NODE_BUFFER_LENGTH));
  }

  m_context->h_bin_pair_views = new unsigned int[4*nBinPairs];
  m_context->h_bin_pair_dphi  = new float[nBinPairs];

  int pairIdx = 0;
  
  for(unsigned int k = 0;k < pInput->m_nBinPairs;k++) {

        int bin1 = pInput->m_bin_pairs[2*k];
        int bin2 = pInput->m_bin_pairs[2*k+1];
      
        float rb1 = ctx.h_bin_rads[2*bin1];//min radius

        unsigned int begin_bin1 = ctx.h_eta_bin_views[2*bin1];
        unsigned int end_bin1   = ctx.h_eta_bin_views[2*bin1 + 1];

        float rb2 = ctx.h_bin_rads[2*bin2+1];//max radius

        float maxDeltaR = rb2 - rb1;// max radius of bin2 - min radius of bin1
        
        float deltaPhi   = pInput->m_algo_params[0] + pInput->m_algo_params[1]*std::abs(maxDeltaR);

        //splitting large bins into more consistent sizes
        
        unsigned int currBegin_bin1 = begin_bin1;

        unsigned int currEnd_bin1 = end_bin1 < TrigAccel::ITk::GBTS_NODE_BUFFER_LENGTH ? end_bin1 : begin_bin1 + TrigAccel::ITk::GBTS_NODE_BUFFER_LENGTH;
        
        for(;currEnd_bin1 < end_bin1; currEnd_bin1 += TrigAccel::ITk::GBTS_NODE_BUFFER_LENGTH, pairIdx++) {
          
          unsigned int offset = 4*pairIdx;

          ctx.h_bin_pair_views[    offset] = currBegin_bin1;
          ctx.h_bin_pair_views[1 + offset] = currEnd_bin1;
          ctx.h_bin_pair_views[2 + offset] = ctx.h_eta_bin_views[2*bin2];
          ctx.h_bin_pair_views[3 + offset] = ctx.h_eta_bin_views[2*bin2 + 1];
          ctx.h_bin_pair_dphi[pairIdx]     = deltaPhi;
              
          currBegin_bin1 = currEnd_bin1;
        }

        currEnd_bin1 = end_bin1;
        
        unsigned int offset = 4*pairIdx;

        ctx.h_bin_pair_views[    offset] = currBegin_bin1;
        ctx.h_bin_pair_views[1 + offset] = currEnd_bin1;
        ctx.h_bin_pair_views[2 + offset] = ctx.h_eta_bin_views[2*bin2];
        ctx.h_bin_pair_views[3 + offset] = ctx.h_eta_bin_views[2*bin2 + 1];
        ctx.h_bin_pair_dphi[pairIdx]     = deltaPhi;
        pairIdx++;
  }
  
  m_context->m_nBinPairs = pairIdx;

  // allocate memory and copy bin pair views and phi cuts to GPU

  size_t data_size = ctx.m_nBinPairs*4*sizeof(unsigned int);
    
  cudaMalloc((void **)&m_context->d_bin_pair_views, data_size);
  cudaMemcpyAsync(&m_context->d_bin_pair_views[0], &ctx.h_bin_pair_views[0], data_size, cudaMemcpyHostToDevice, ctx.m_stream);

  m_context->d_size += data_size;

  data_size = ctx.m_nBinPairs*sizeof(float);

  cudaMalloc((void **)&m_context->d_bin_pair_dphi, data_size);
  cudaMemcpyAsync(&m_context->d_bin_pair_dphi[0], &ctx.h_bin_pair_dphi[0], data_size, cudaMemcpyHostToDevice, ctx.m_stream);

  m_context->d_size += data_size;

  data_size = 4*sizeof(unsigned int);
  cudaMemset(ctx.d_counters, 0, data_size);
  
  cudaStreamSynchronize(ctx.m_stream);

  //3. graph edge making kernel

  nBlocks = ctx.m_nBinPairs;
  nThreads = 128;

  graphEdgeMakingKernel_ITk<<<nBlocks, nThreads, 0, ctx.m_stream>>>(reinterpret_cast<uint4*>(ctx.d_bin_pair_views),
                                                     ctx.d_bin_pair_dphi, ctx.d_node_params,
                                                     ctx.d_algo_params, ctx.d_counters, reinterpret_cast<int2*>(ctx.d_edge_nodes), 
                                                     reinterpret_cast<float4*>(ctx.d_edge_params),
                                                     ctx.d_num_incoming_edges, ctx.m_nMaxEdges);

  cudaStreamSynchronize(ctx.m_stream);

  error = cudaGetLastError();

  if(error != cudaSuccess) {
      printf("edge making: CUDA error: %s\n", cudaGetErrorString(error));
      return false;
  }

  unsigned int nStats[4];

  cudaMemcpy(&nStats[0], ctx.d_counters, 4*sizeof(unsigned int), cudaMemcpyDeviceToHost);
  
  //printf("Created %d edges\n",nStats[0]);

  m_context->m_nEdges = nStats[0];
    
  if(ctx.m_nEdges > ctx.m_nMaxEdges) m_context->m_nEdges = ctx.m_nMaxEdges;

  //4. import incoming edges counters and calculate prefix sum

  unsigned int* cusum = new unsigned int[ctx.m_nNodes];

  data_size = ctx.m_nNodes*sizeof(unsigned int);
  
  cudaMemcpyAsync(&cusum[0], ctx.d_num_incoming_edges, data_size, cudaMemcpyDeviceToHost, ctx.m_stream);

  cudaStreamSynchronize(ctx.m_stream);

  for(int k=1;k<ctx.m_nNodes;k++) cusum[k] += cusum[k-1];

  cudaMemcpyAsync(ctx.d_num_incoming_edges, &cusum[0], data_size, cudaMemcpyHostToDevice, ctx.m_stream);

  delete[] cusum;

  cudaStreamSynchronize(ctx.m_stream);

  //5. link edges and nodes

  data_size = ctx.m_nEdges*sizeof(int);

  cudaMalloc((void **)&m_context->d_edge_links, data_size);

  m_context->d_size += data_size;

  nThreads = 256;
  nBlocks = (int)(std::ceil((1.0*ctx.m_nEdges)/nThreads));

  graphEdgeLinkingKernel_ITk<<<nBlocks, nThreads, 0, ctx.m_stream>>>(reinterpret_cast<int2*>(ctx.d_edge_nodes), ctx.d_num_incoming_edges,
  					                            ctx.d_edge_links, ctx.d_link_counters, ctx.m_nEdges);

  cudaStreamSynchronize(ctx.m_stream);

  error = cudaGetLastError();

  if(error != cudaSuccess) {
      printf("edge linking: CUDA error: %s\n", cudaGetErrorString(error));
      return false;
  }

  //6. edge matching to create edge-to-edge connections

  data_size = ctx.m_nEdges*sizeof(int);

  cudaMalloc((void **)&m_context->d_num_neighbours, data_size);
  cudaMemset(m_context->d_num_neighbours, 0, data_size);

  m_context->d_size += data_size;
    
  cudaMalloc((void **)&m_context->d_reIndexer, data_size);
  cudaMemset(m_context->d_reIndexer, 0xFF, data_size);

  m_context->d_size += data_size;

  data_size = TrigAccel::ITk::GBTS_MAX_NUM_NEIGHBOURS*ctx.m_nEdges * sizeof(int);
  cudaMalloc((void**) &m_context->d_neighbours, data_size);

  m_context->d_size += data_size;

  graphEdgeMatchingKernel_ITk<<<nBlocks, nThreads, 0, ctx.m_stream>>>(ctx.d_algo_params, reinterpret_cast<float4*>(ctx.d_edge_params),
                                            reinterpret_cast<int2*>(ctx.d_edge_nodes), ctx.d_link_counters, ctx.d_num_incoming_edges, ctx.d_edge_links,
                                             ctx.d_num_neighbours, ctx.d_neighbours, ctx.d_reIndexer, ctx.d_counters, ctx.m_nEdges);

  cudaStreamSynchronize(ctx.m_stream);
  
  error = cudaGetLastError();

  if(error != cudaSuccess) {
      printf("edge matching: CUDA error: %s\n", cudaGetErrorString(error));
      return false;
  }

  //7. Edge re-indexing to keep only edges involved in any connection

  edgeReIndexingKernel_ITk<<<nBlocks, nThreads, 0, ctx.m_stream>>>(ctx.d_reIndexer, ctx.d_counters, ctx.m_nEdges);

  cudaStreamSynchronize(ctx.m_stream);

  error = cudaGetLastError();

  if(error != cudaSuccess) {
      printf("edge re-indexing: CUDA error: %s\n", cudaGetErrorString(error));
      return false;
  }

  cudaMemcpy(&nStats[0], ctx.d_counters, 4*sizeof(unsigned int), cudaMemcpyDeviceToHost);
  
  m_context->m_nLinks = nStats[1];
  m_context->m_nUniqueEdges = nStats[2];

  //printf("created %d edge links, found %d unique edges for export\n",ctx.m_nLinks, ctx.m_nUniqueEdges);

  int nIntsPerEdge = 2 + 1 + TrigAccel::ITk::GBTS_MAX_NUM_NEIGHBOURS;

  data_size = ctx.m_nUniqueEdges*nIntsPerEdge*sizeof(int);
  
  cudaMalloc((void **)&m_context->d_output_graph, data_size);

  m_context->d_size += data_size;

  nThreads = 256;
  int nEdgesPerBlock = nThreads*64;
  
  nBlocks = (int)(std::ceil((1.0*ctx.m_nEdges)/nEdgesPerBlock));
  
  graphCompressionKernel_ITk<<<nBlocks, nThreads, 0, ctx.m_stream>>>(ctx.d_node_index, ctx.d_edge_nodes, ctx.d_num_neighbours, ctx.d_neighbours, ctx.d_reIndexer, 
                                                      ctx.d_output_graph, nEdgesPerBlock, ctx.m_nEdges);

  cudaStreamSynchronize(ctx.m_stream);

  error = cudaGetLastError();

  if(error != cudaSuccess) {
      printf("graph compression: CUDA error: %s\n", cudaGetErrorString(error));
      return false;
  }

  TrigAccel::ITk::COMPRESSED_GRAPH* pOutput = reinterpret_cast<TrigAccel::ITk::COMPRESSED_GRAPH *>(m_output->m_rawBuffer);


  pOutput->m_nEdges = ctx.m_nUniqueEdges;
  pOutput->m_nMaxNeighbours = TrigAccel::ITk::GBTS_MAX_NUM_NEIGHBOURS;
  pOutput->m_nLinks = ctx.m_nLinks;
  pOutput->m_graphArray = new int[ctx.m_nUniqueEdges*nIntsPerEdge];
  
  cudaMemcpyAsync(&pOutput->m_graphArray[0], ctx.d_output_graph, data_size, cudaMemcpyDeviceToHost, ctx.m_stream);

  checkError();
  
  cudaStreamSynchronize(ctx.m_stream);

  m_timeLine->push_back(WorkTimeStamp(m_workId, 1, tbb::tick_count::now()));

  return true;
}

