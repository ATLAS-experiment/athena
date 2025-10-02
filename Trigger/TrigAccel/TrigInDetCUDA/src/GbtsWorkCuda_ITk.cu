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

#include "GbtsGraphProcessingKernelsCuda_ITk.cuh"

GbtsWorkCudaITk::GbtsWorkCudaITk(unsigned int id, GbtsDeviceContext* ctx, std::shared_ptr<TrigAccel::OffloadBuffer> data, 
	WorkTimeStampQueue* TL) : 
	m_workId(id),
	m_context(ctx), 
	m_input(data),
	m_timeLine(TL)
 {
	m_output = std::make_shared<TrigAccel::OffloadBuffer>(sizeof(TrigAccel::ITk::GRAPH_AND_SEEDS_OUTPUT));//output data
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
	
	cudaFree(ctx.d_num_neighbours);
	cudaFree(ctx.d_reIndexer);
	cudaFree(ctx.d_neighbours);
	//CCA
	cudaFree(ctx.d_active_edges);
	cudaFree(ctx.d_levels);	
	cudaFree(ctx.d_level_views);	
	cudaFree(ctx.d_level_boundaries);	
	
	//Seed extraction
	cudaFree(ctx.d_mini_states);	
	cudaFree(ctx.d_seed_proposals);	
	cudaFree(ctx.d_state_store);
	cudaFree(ctx.d_edge_bids);
	cudaFree(ctx.d_seed_ambiguity);
	
	cudaFree(ctx.d_seeds);	
	
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
	
	//initalization of the m_rawBuffer for the unique pointers 
	TrigAccel::ITk::GRAPH_AND_SEEDS_OUTPUT* pOutput = reinterpret_cast<TrigAccel::ITk::GRAPH_AND_SEEDS_OUTPUT*>(m_output->m_rawBuffer);
	TrigAccel::ITk::GRAPH_AND_SEEDS_OUTPUT InitialOutput;
	memcpy(m_output->m_rawBuffer, &InitialOutput, sizeof(TrigAccel::ITk::GRAPH_AND_SEEDS_OUTPUT));
	
	//1. create graph nodes and order them by eta and phi
	
	int nThreads = 256;
	int nNodesPerBlock = nThreads*64;
	  
	int nBlocks = (int)(std::ceil((1.0f*ctx.m_nNodes)/nNodesPerBlock));
	
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

	nBlocks = (int)(std::ceil((1.0f*ctx.m_nNodes)/nNodesPerBlock));

	eta_phi_histo_kernel<<<nBlocks, nThreads, 0, ctx.m_stream>>>(ctx.d_node_phi_index, ctx.d_node_eta_index, ctx.d_eta_phi_histo, nNodesPerBlock, ctx.m_nNodes);

	cudaStreamSynchronize(ctx.m_stream);
 
	error = cudaGetLastError();

	if(error != cudaSuccess) {
		printf("eta-phi histo: CUDA error: %s\n", cudaGetErrorString(error));
		return false;
	}

	int nBinsPerBlock = 128;
		
	nThreads = nBinsPerBlock;

	nBlocks = (int)(std::ceil((1.0f*ctx.m_maxEtaBin)/nBinsPerBlock));

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
		eta_bin_views[pos]	 = (view_idx == 0) ? 0 : eta_sums[view_idx-1];
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
		
	nBlocks = (int)(std::ceil((1.0f*ctx.m_nNodes)/nNodesPerBlock));

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

	nBlocks = (int)(std::ceil((1.0f*ctx.m_maxEtaBin)/nBinsPerBlock));

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

		nBinPairs += (int)(std::ceil((1.0f*nNodesInBin1)/TrigAccel::ITk::GBTS_NODE_BUFFER_LENGTH));
	}
	
	m_context->h_bin_pair_views = new unsigned int[4*nBinPairs];
	m_context->h_bin_pair_dphi  = new float[nBinPairs];

	int pairIdx = 0;
	for(unsigned int k = 0;k < pInput->m_nBinPairs;k++) {
		
		int bin1 = pInput->m_bin_pairs[2*k];
		int bin2 = pInput->m_bin_pairs[2*k+1];
		
		float rb1 = ctx.h_bin_rads[2*bin1];//min radius

		unsigned int begin_bin1 = ctx.h_eta_bin_views[2*bin1];
		unsigned int end_bin1	 = ctx.h_eta_bin_views[2*bin1+1];
		//skip empty pairs
		if(begin_bin1 == end_bin1) continue;
		if(ctx.h_eta_bin_views[2*bin2] == ctx.h_eta_bin_views[2*bin2+1]) continue;

		float rb2 = ctx.h_bin_rads[2*bin2+1];//max radius
		
		float maxDeltaR = std::fabs(rb2 - rb1);// max radius of bin2 - min radius of bin1
				
		float deltaPhi = pInput->m_algo_params[0] + pInput->m_algo_params[1]*maxDeltaR;
		if(maxDeltaR < 60) deltaPhi = pInput->m_algo_params[2] + pInput->m_algo_params[3]*maxDeltaR;

		//splitting large bins into more consistent sizes
				
		unsigned int currBegin_bin1 = begin_bin1;

		unsigned int currEnd_bin1 = end_bin1 < TrigAccel::ITk::GBTS_NODE_BUFFER_LENGTH ? end_bin1 : begin_bin1 + TrigAccel::ITk::GBTS_NODE_BUFFER_LENGTH;
		
		for(;currEnd_bin1 < end_bin1; currEnd_bin1 += TrigAccel::ITk::GBTS_NODE_BUFFER_LENGTH, pairIdx++) {
			unsigned int offset = 4*pairIdx;
			
			ctx.h_bin_pair_views[offset] = currBegin_bin1;
			ctx.h_bin_pair_views[1 + offset] = currEnd_bin1;
			ctx.h_bin_pair_views[2 + offset] = ctx.h_eta_bin_views[2*bin2];
			ctx.h_bin_pair_views[3 + offset] = ctx.h_eta_bin_views[2*bin2 + 1];
			ctx.h_bin_pair_dphi[pairIdx]     = deltaPhi;
							
			currBegin_bin1 = currEnd_bin1;
		}
		currEnd_bin1 = end_bin1;
		
		unsigned int offset = 4*pairIdx;

		ctx.h_bin_pair_views[offset]     = currBegin_bin1;
		ctx.h_bin_pair_views[1 + offset] = currEnd_bin1;
		ctx.h_bin_pair_views[2 + offset] = ctx.h_eta_bin_views[2*bin2];
		ctx.h_bin_pair_views[3 + offset] = ctx.h_eta_bin_views[2*bin2 + 1];
		ctx.h_bin_pair_dphi[pairIdx]     = deltaPhi;
		pairIdx++;
		
	}
	m_context->m_nBinPairs = pairIdx;
	if(pairIdx == 0) return true;

	// allocate memory and copy bin pair views and phi cuts to GPU

	size_t data_size = ctx.m_nBinPairs*4*sizeof(unsigned int);
		
	cudaMalloc((void **)&m_context->d_bin_pair_views, data_size);
	cudaMemcpyAsync(&m_context->d_bin_pair_views[0], &ctx.h_bin_pair_views[0], data_size, cudaMemcpyHostToDevice, ctx.m_stream);

	m_context->d_size += data_size;

	data_size = ctx.m_nBinPairs*sizeof(float);

	cudaMalloc((void **)&m_context->d_bin_pair_dphi, data_size);
	cudaMemcpyAsync(&m_context->d_bin_pair_dphi[0], &ctx.h_bin_pair_dphi[0], data_size, cudaMemcpyHostToDevice, ctx.m_stream);

	m_context->d_size += data_size;

	data_size = 12*sizeof(unsigned int);
	cudaMemset(ctx.d_counters, 0, data_size);
	
	cudaStreamSynchronize(ctx.m_stream);

	//3. graph edge making kernel
	

	nBlocks = ctx.m_nBinPairs;
	nThreads = 128;
	
	graphEdgeMakingKernel_ITk<<<nBlocks, nThreads, 0, ctx.m_stream>>>(reinterpret_cast<uint4*>(ctx.d_bin_pair_views),
	                                                 ctx.d_bin_pair_dphi, ctx.d_node_params,
	                                                 ctx.d_algo_params, ctx.d_counters, reinterpret_cast<int2*>(ctx.d_edge_nodes), 
	                                                 reinterpret_cast<half4*>(ctx.d_edge_params),
	                                                 ctx.d_num_incoming_edges, ctx.m_nMaxEdges);

	cudaStreamSynchronize(ctx.m_stream);

	error = cudaGetLastError();

	if(error != cudaSuccess) {
		printf("edge making: CUDA error: %s\n", cudaGetErrorString(error));
		return false;
	}

	unsigned int nStats[4];

	cudaMemcpy(&nStats[0], ctx.d_counters, 4*sizeof(unsigned int), cudaMemcpyDeviceToHost);
	//printf("Created %d edges under a cap of %d\n",nStats[0], ctx.m_nMaxEdges);

	m_context->m_nEdges = nStats[0];
		
	if(ctx.m_nEdges >= ctx.m_nMaxEdges) m_context->m_nEdges = ctx.m_nMaxEdges-1;
	else if(ctx.m_nEdges == 0) return true;
	//4. import incoming edges counters and calculate prefix sum

	unsigned int* cusum = new unsigned int[ctx.m_nNodes+1];

	data_size = (ctx.m_nNodes+1)*sizeof(unsigned int);
	
	cudaMemcpyAsync(&cusum[0], ctx.d_num_incoming_edges, data_size, cudaMemcpyDeviceToHost, ctx.m_stream);

	cudaStreamSynchronize(ctx.m_stream);
	
	for(int k=0;k<ctx.m_nNodes;k++) cusum[k+1] += cusum[k];
	
	cudaMemcpyAsync(ctx.d_num_incoming_edges, &cusum[0], data_size, cudaMemcpyHostToDevice, ctx.m_stream);
	
	delete[] cusum;

	cudaStreamSynchronize(ctx.m_stream);

	//5. link edges and nodes

	data_size = ctx.m_nEdges*sizeof(int);

	cudaMalloc((void **)&m_context->d_edge_links, data_size);

	m_context->d_size += data_size;

	nThreads = 256;
	nBlocks = (int)(std::ceil((1.0f*ctx.m_nEdges)/nThreads));

	graphEdgeLinkingKernel_ITk<<<nBlocks, nThreads, 0, ctx.m_stream>>>(reinterpret_cast<int2*>(ctx.d_edge_nodes), 
	                                                                  ctx.d_edge_links, ctx.d_num_incoming_edges,
	                                                                  ctx.m_nEdges);

	cudaStreamSynchronize(ctx.m_stream);
	
	error = cudaGetLastError();

	if(error != cudaSuccess) {
		printf("edge linking: CUDA error: %s\n", cudaGetErrorString(error));
		return false;
	}

	//6. edge matching to create edge-to-edge connections

	data_size = ctx.m_nEdges*sizeof(unsigned char);	

	cudaMalloc((void **)&m_context->d_num_neighbours, data_size);
	cudaMemset(m_context->d_num_neighbours, 0, data_size);

	m_context->d_size += data_size;
		
	data_size = ctx.m_nEdges*sizeof(int);
	
	cudaMalloc((void **)&m_context->d_reIndexer, data_size);
	cudaMemset(m_context->d_reIndexer, 0xFF, data_size);

	m_context->d_size += data_size;

	data_size = TrigAccel::ITk::GBTS_MAX_NUM_NEIGHBOURS*ctx.m_nEdges * sizeof(int);
	cudaMalloc((void**) &m_context->d_neighbours, data_size);

	m_context->d_size += data_size;

	graphEdgeMatchingKernel_ITk<<<nBlocks, nThreads, 0, ctx.m_stream>>>(ctx.d_algo_params, reinterpret_cast<half4*>(ctx.d_edge_params),
	                                         reinterpret_cast<int2*>(ctx.d_edge_nodes), ctx.d_num_incoming_edges, ctx.d_edge_links,
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

	//printf("created %d edge links, found %d unique edges for export\n",m_context->m_nLinks, m_context->m_nUniqueEdges);
	if(m_context->m_nUniqueEdges == 0) return true;

	int nIntsPerEdge = 2 + 1 + TrigAccel::ITk::GBTS_MAX_NUM_NEIGHBOURS;

	data_size = ctx.m_nUniqueEdges*nIntsPerEdge*sizeof(int);
	
	cudaMalloc((void **)&m_context->d_output_graph, data_size);

	m_context->d_size += data_size;

	nThreads = 256;
	int nEdgesPerBlock = nThreads*64;
	
	nBlocks = (int)(std::ceil((1.0f*ctx.m_nEdges)/nEdgesPerBlock));
		
	graphCompressionKernel_ITk<<<nBlocks, nThreads, 0, ctx.m_stream>>>(reinterpret_cast<float4*>(ctx.d_sp_params), ctx.d_node_index, 
	                                                                 ctx.d_edge_nodes, ctx.d_num_neighbours, ctx.d_neighbours,
	                                                                 ctx.d_reIndexer, ctx.d_output_graph, nEdgesPerBlock, ctx.m_nEdges);

	cudaStreamSynchronize(ctx.m_stream);

	error = cudaGetLastError();
	if(error != cudaSuccess) {
		printf("graph compression: CUDA error: %s\n", cudaGetErrorString(error));
		return false;
	}
	if(!ctx.m_useGPUseedExtraction) {
		//export graph for CPU seed extraction
		pOutput->m_CompressedGraph.m_nEdges = ctx.m_nUniqueEdges;
		pOutput->m_CompressedGraph.m_nMaxNeighbours = TrigAccel::ITk::GBTS_MAX_NUM_NEIGHBOURS;
		pOutput->m_CompressedGraph.m_nLinks = ctx.m_nLinks;
		if(ctx.m_nUniqueEdges > 0) {
			pOutput->m_CompressedGraph.m_graphArray = std::make_unique<int[]>(ctx.m_nUniqueEdges*nIntsPerEdge);
			cudaMemcpyAsync(&pOutput->m_CompressedGraph.m_graphArray[0], ctx.d_output_graph, sizeof(int)*ctx.m_nUniqueEdges*nIntsPerEdge, cudaMemcpyDeviceToHost, ctx.m_stream);
		}
	}
	else {
	// 8. Message-passing CCA
		
		data_size = ctx.m_nUniqueEdges*sizeof(int);

		cudaMalloc((void**) &m_context->d_active_edges, data_size);
		cudaMemset(m_context->d_active_edges, 0xFF, data_size);// initialize to -1
		m_context->d_size += data_size;

		data_size = 2*ctx.m_nUniqueEdges*sizeof(unsigned char);

		cudaMalloc((void **)&m_context->d_levels, data_size); //old levels and new levels are kept in opposite halves of the array
		cudaMemset(m_context->d_levels, 0x1, data_size); //initalize to 1 so level counts the maxium number of edge segments for a seed originating at the edge
		m_context->d_size += data_size;
		
		data_size = ctx.m_nUniqueEdges*sizeof(int);

		cudaMalloc((void**) &m_context->d_level_views, data_size);//level-based edge views

		m_context->d_size += data_size;

		data_size = (TrigAccel::ITk::GBTS_MAX_CCA_ITERATIONS+1)*sizeof(int);

		cudaMalloc((void**) &m_context->d_level_boundaries, data_size);
		cudaMemset(m_context->d_level_boundaries, 0, data_size);

		m_context->d_size += data_size;

		int nEdgesLeft = ctx.m_nUniqueEdges;

		cudaMemcpyAsync(&ctx.d_counters[3], &nEdgesLeft, sizeof(int), cudaMemcpyHostToDevice, ctx.m_stream);

		checkError();

		cudaStreamSynchronize(ctx.m_stream);

		nThreads = 128;
		nBlocks = (int) std::ceil(1.0f*nEdgesLeft/nThreads);
		for(int iter = 0;iter < TrigAccel::ITk::GBTS_MAX_CCA_ITERATIONS; iter++) {
			CCA_IterationKernel_ITk<<<nBlocks, nThreads, 0, ctx.m_stream>>>(ctx.d_output_graph, ctx.d_levels, ctx.d_active_edges, ctx.d_level_views,
			                                                                ctx.d_level_boundaries, ctx.d_counters, iter, ctx.m_nUniqueEdges);
			cudaStreamSynchronize(ctx.m_stream);							     
		}
		
		cudaStreamSynchronize(ctx.m_stream);
		
		error = cudaGetLastError();
		
		if(error != cudaSuccess) {
			printf("message-passing CCA: CUDA error: %s\n", cudaGetErrorString(error));
			return false;
		}
	
		int nEdgesByLevel_cuml[TrigAccel::ITk::GBTS_MAX_CCA_ITERATIONS + 1];
		nEdgesByLevel_cuml[TrigAccel::ITk::GBTS_MAX_CCA_ITERATIONS] = 0;		
		cudaMemcpyAsync(&nEdgesByLevel_cuml[0], ctx.d_level_boundaries, sizeof(nEdgesByLevel_cuml), cudaMemcpyDeviceToHost, ctx.m_stream);
		int level_max = TrigAccel::ITk::GBTS_MAX_CCA_ITERATIONS; for(;nEdgesByLevel_cuml[level_max-1] == 0; level_max--); 
		
		if(level_max < ctx.m_minLevel) return true;	
		checkError();
		
		//9. seed extraction
		int device; cudaGetDevice(&device);
		int SM_count; cudaDeviceGetAttribute(&SM_count, cudaDevAttrMultiProcessorCount, device);
		int smem; cudaDeviceGetAttribute(&smem, cudaDevAttrMaxSharedMemoryPerMultiprocessor, device);

		nThreads = 896; //448 for two blocks per SM limited by registers
		
		nBlocks = 0;
		int soft_max_blocks = 0.8*SM_count*(smem/(sizeof(edgeState)*TrigAccel::ITk::GBTS_MAX_SHARED_STATES)); 

		//TO-DO better fit malloc sizes
		int nMaxMini = 10000 + ctx.m_nUniqueEdges*3;
		cudaMalloc((void**) &m_context->d_mini_states, sizeof(int2)*nMaxMini);
		m_context->d_size+=sizeof(int2)*nMaxMini;	

		int nMaxStateStore = 2000 + ctx.m_nUniqueEdges*4;
		cudaMalloc((void**) &m_context->d_state_store, sizeof(edgeState)*nMaxStateStore);
		m_context->d_size+=sizeof(edgeState)*nMaxStateStore;	
		
		int nMaxProps = 4000 + ctx.m_nUniqueEdges;
		cudaMalloc((void**) &m_context->d_seed_proposals, sizeof(int2)*nMaxProps); 
		cudaMalloc((void**) &m_context->d_seed_ambiguity, sizeof(char)*nMaxProps); 
		m_context->d_size+=(sizeof(int2)+sizeof(char))*nMaxProps;	
		
		cudaMalloc((void**) &m_context->d_edge_bids, sizeof(unsigned long long int)*ctx.m_nUniqueEdges);
		m_context->d_size+=sizeof(unsigned long long int)*ctx.m_nUniqueEdges;	

		int nMaxSeeds = 20 + ctx.m_nUniqueEdges/4;
		cudaMalloc((void**) &m_context->d_seeds, sizeof(TrigAccel::ITk::Tracklet)*nMaxSeeds);
		m_context->d_size+=sizeof(TrigAccel::ITk::Tracklet)*nMaxSeeds;	
		
		int view_shift = nEdgesByLevel_cuml[0];
		for(int level = level_max-1; level+1>=ctx.m_minLevel; level--) {
			int nRootEdges = (nEdgesByLevel_cuml[level]-nEdgesByLevel_cuml[level_max]);
				
			if(nRootEdges == 0) continue;
			nBlocks += std::ceil(nRootEdges*std::pow(1.3f, 1.0f*(level+1))/TrigAccel::ITk::GBTS_MAX_SHARED_STATES);
			if(nBlocks > soft_max_blocks || level_max-level>1 || level+1==ctx.m_minLevel) {

				int view_min = view_shift-nEdgesByLevel_cuml[level]; 
				int view_max = view_shift-nEdgesByLevel_cuml[level_max];	
				
				if(view_min == view_max || nBlocks < 1) continue;	
				
				cudaMemset(m_context->d_edge_bids, 0, sizeof(unsigned long long int)*ctx.m_nUniqueEdges);
				
				seed_extracting_kernel_ITk<<<nBlocks, nThreads, 0, ctx.m_stream>>>(view_min, view_max, ctx.d_level_views, ctx.d_levels, 
				            reinterpret_cast<float4*>(ctx.d_sp_params), ctx.d_output_graph,
				            reinterpret_cast<int2*>(ctx.d_mini_states), reinterpret_cast<edgeState*>(ctx.d_state_store),
				            ctx.d_edge_bids, ctx.d_seed_ambiguity, reinterpret_cast<int2*>(ctx.d_seed_proposals), ctx.d_seeds, 
				            ctx.d_counters, ctx.m_minLevel, nMaxMini, nMaxProps, nMaxStateStore/nBlocks, nMaxSeeds);	
				level_max = level;
				nBlocks = 0;
			}
		}
		cudaStreamSynchronize(ctx.m_stream);
	
		error = cudaGetLastError();

		if(error != cudaSuccess) {
			printf("seed-extracting kalman filter: CUDA error: %s\n", cudaGetErrorString(error));
			return false;
		}
		
		cudaMemcpyAsync(&m_context->m_nSeeds, &ctx.d_counters[9], sizeof(unsigned int) ,cudaMemcpyDeviceToHost, ctx.m_stream);
		if(m_context->m_nSeeds > nMaxSeeds) m_context->m_nSeeds = nMaxSeeds;
		pOutput->m_OutputSeeds.m_nSeeds = m_context->m_nSeeds;
		if(m_context->m_nSeeds > 0) {
			pOutput->m_OutputSeeds.m_seedsArray = std::make_unique<TrigAccel::ITk::Tracklet[]>(m_context->m_nSeeds);
			cudaMemcpyAsync(&pOutput->m_OutputSeeds.m_seedsArray[0], ctx.d_seeds, sizeof(TrigAccel::ITk::Tracklet)*m_context->m_nSeeds, cudaMemcpyDeviceToHost, ctx.m_stream);
		}
	}
	checkError();

	cudaStreamSynchronize(ctx.m_stream);

	m_timeLine->push_back(WorkTimeStamp(m_workId, 1, tbb::tick_count::now()));
	
	return true;
}

