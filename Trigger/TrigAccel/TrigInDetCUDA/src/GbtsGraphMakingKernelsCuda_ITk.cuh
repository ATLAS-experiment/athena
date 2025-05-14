/*
	Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGINDETCUDA_GBTSGRAPHMAKINGKERNELSCUDA_ITK_CUH
#define TRIGINDETCUDA_GBTSGRAPHMAKINGKERNELSCUDA_ITK_CUH

#include <cuda_runtime.h>

__global__ static void graphEdgeMakingKernel_ITk(const uint4* d_bin_pair_views, const float* d_bin_pair_dphi, const float* d_node_params, const float* d_algo_params,
                                                 unsigned int* d_counters, int2* d_edge_nodes, float4* d_edge_params, unsigned int* d_num_outgoing_edges, 
                                                 unsigned int nMaxEdges) {
	__shared__ unsigned int begin_bin1; 
	__shared__ unsigned int begin_bin2; 
	__shared__ unsigned int num_nodes1;
	__shared__ unsigned int num_nodes2;
	__shared__ float deltaPhi;

	__shared__ float minDeltaRad;
	__shared__ float min_z0;
	__shared__ float max_z0;
	__shared__ float maxOuterRad;
	__shared__ float min_zU;
	__shared__ float max_zU;
	__shared__ float max_kappa_high;
	__shared__ float max_kappa_low;

	__shared__ float tau_min[TrigAccel::ITk::GBTS_NODE_BUFFER_LENGTH];
	__shared__ float tau_max[TrigAccel::ITk::GBTS_NODE_BUFFER_LENGTH];
	__shared__ float phi[TrigAccel::ITk::GBTS_NODE_BUFFER_LENGTH];
	__shared__ float r[TrigAccel::ITk::GBTS_NODE_BUFFER_LENGTH];
	__shared__ float z[TrigAccel::ITk::GBTS_NODE_BUFFER_LENGTH];

	if(threadIdx.x == 0) {
		uint4 views    = d_bin_pair_views[blockIdx.x];
		deltaPhi       = d_bin_pair_dphi[blockIdx.x];
		
		begin_bin1     = views.x; 
		begin_bin2     = views.z;
		
		num_nodes1 = views.y - begin_bin1;
		num_nodes2 = views.w - begin_bin2;
		
		minDeltaRad    = d_algo_params[2];
		min_z0         = d_algo_params[3];
		max_z0         = d_algo_params[4];
		maxOuterRad    = d_algo_params[5];
		min_zU         = d_algo_params[6];
		max_zU         = d_algo_params[7];
		max_kappa_low  = d_algo_params[8];
		max_kappa_high = d_algo_params[9];
	}

	__syncthreads();
	for(int idx = threadIdx.x; idx < num_nodes1; idx += blockDim.x) {//loading a chunk of nodes1 into shared mem buffers
		int offset = 5*(idx + begin_bin1);
		tau_min[idx] = d_node_params[offset];
		tau_max[idx] = d_node_params[offset+1];
		phi[idx]     = d_node_params[offset+2];
		r[idx]       = d_node_params[offset+3];
		z[idx]       = d_node_params[offset+4];
	}

	__syncthreads();

	int last_n1 = 0;//initial value for the sliding window

	float phi_bin_width = 2.0f * CUDART_PI_F / TrigAccel::ITk::GBTS_MAX_PHI_BIN;

	for(int n2Idx = threadIdx.x; n2Idx < num_nodes2; n2Idx += blockDim.x) {
			
		int globalIdx2 = begin_bin2 + n2Idx;
		int o2 = 5*globalIdx2;
		
		float phi2 = d_node_params[2 + o2];
		
		float min_phi1 = phi2 - deltaPhi;
		float max_phi1 = phi2 + deltaPhi;
		
		if (min_phi1 < -CUDART_PI_F) min_phi1 += 2.0f * CUDART_PI_F;
		if (max_phi1 >  CUDART_PI_F) max_phi1 -= 2.0f * CUDART_PI_F;
						
		bool boundary = max_phi1 < min_phi1;// +/- pi wraparound

		float max_phi1_bin = max_phi1 + phi_bin_width;
		float min_phi1_bin = min_phi1 - phi_bin_width;
		
		if(!boundary) {
			if(phi[0] > max_phi1_bin) continue;
			if(phi[num_nodes1-1] < min_phi1_bin) continue;
		}
		else {
			if(phi[num_nodes1 - 1] < min_phi1_bin && phi[0] > max_phi1_bin) continue;
		}        
		
		if(boundary) last_n1 = 0;//reset the sliding window if we hit the boundary

		float r2       = d_node_params[3 + o2];
		float z2       = d_node_params[4 + o2];
		float tau_min2 = d_node_params[    o2];
		float tau_max2 = d_node_params[1 + o2];
		
		for(int n1Idx = last_n1; n1Idx < num_nodes1; n1Idx++) {
			float phi1 = phi[n1Idx];

			if(!boundary) {
				if(phi1 > max_phi1_bin) break;
				if(phi1 < min_phi1_bin) continue;
				last_n1 = n1Idx;
			}
			else {
				 if(phi1 > max_phi1_bin && phi1 < min_phi1_bin) continue;
			}

			float r1 = r[n1Idx];
			float dr = r2-r1;
 
			if(dr < minDeltaRad) continue;
															
			float z1 = z[n1Idx];
			float dz = z2 - z1;
			float tau = dz/dr;
			float ftau = fabsf(tau);

			if(ftau > 36.0) continue; //detector acceptance

			if((ftau < tau_min2) || (ftau > tau_max2)) continue;

			if((ftau < tau_min[n1Idx]) || (ftau > tau_max[n1Idx])) continue;

			//RZ doublet filter cuts

			float z0 = z1 - r1*tau;
							
			if((z0 < min_z0) || (z0 > max_z0)) continue;

			float zouter = z0 + maxOuterRad*tau;

			if(zouter < min_zU || zouter > max_zU) continue;

			float dphi = phi2 - phi1;

			if(boundary) {
				if (dphi < -CUDART_PI_F) dphi += 2.0f * CUDART_PI_F;
				else if (dphi >  CUDART_PI_F) dphi -= 2.0f * CUDART_PI_F;
			}

			if(fabsf(dphi) > deltaPhi) continue;

			float curv = dphi/dr;

			float abs_curv = fabsf(curv);
 
			if(ftau < 4.0) {//for eta < 2.1
				if(abs_curv > max_kappa_low) continue;
			}
			else { 
				if(abs_curv > max_kappa_high) continue;
			}
		
			int nEdges = atomicAdd(&d_counters[0], 1);

			if(nEdges < nMaxEdges) {
				float exp_eta = sqrtf(1 + tau*tau) - tau;
				atomicAdd(&d_num_outgoing_edges[globalIdx2], 1);
				d_edge_nodes[nEdges]  = make_int2(begin_bin1 + n1Idx, globalIdx2);
				d_edge_params[nEdges] = make_float4(exp_eta, curv, phi1 + curv*r1, phi2 + curv*r2);
			}
		}
	}
}

__global__ static void graphEdgeLinkingKernel_ITk(const int2* d_edge_nodes, const unsigned int* d_link_ends, int* d_edge_links, int* d_link_counters, int nEdges) {

	int edge_idx = blockIdx.x * blockDim.x + threadIdx.x;

	if(edge_idx >= nEdges) return;

	int n2Idx = d_edge_nodes[edge_idx].y;//global index of n2

	int link_end = d_link_ends[n2Idx] - 1;

	int k = atomicAdd(&d_link_counters[n2Idx], 1);

	d_edge_links[link_end - k] = edge_idx; //this edge starts from n2, matching will check edge's n1 and then loop over edges outgoing from that node

}

__global__ static void graphEdgeMatchingKernel_ITk(const float* d_algo_params, const float4* d_edge_params, const int2* d_edge_nodes, 
                                                   const int* d_link_counters, const unsigned int* d_link_ends, const int* d_edge_links, 
                                                   unsigned char* d_num_neighbours, int* d_neighbours, int* d_reIndexer, unsigned int* d_counters, int nEdges) {

	int edge1_idx = blockIdx.x * blockDim.x + threadIdx.x;

	if (edge1_idx >= nEdges) return;

	int n1Idx = d_edge_nodes[edge1_idx].x;//global index of n1 node of the edge1

	int nLinks = d_link_counters[n1Idx];//the number of edges which has n1 as their starting node (n2)

	if (nLinks == 0) return;

	float4 params1 = d_edge_params[edge1_idx]; // [exp_eta, curv, Phi1, Phi2]

	int link_end = d_link_ends[n1Idx] - 1;

	float uat_2  = 1.0f/params1.x;
	float Phi2   = params1.z;
	float curv2  = params1.y;

	int nei_pos  = TrigAccel::ITk::GBTS_MAX_NUM_NEIGHBOURS*edge1_idx;

	unsigned char num_nei  = 0;

	for(int k=0;k<nLinks;k++) {//loop over potential neighbours

		if (num_nei == TrigAccel::ITk::GBTS_MAX_NUM_NEIGHBOURS-1) break;

		int edge2_idx = d_edge_links[link_end - k];

		float4 params2 = d_edge_params[edge2_idx];
				
		float tau_ratio = params2.x*uat_2 - 1.0f;
					
		if(fabsf(tau_ratio) > d_algo_params[12]) {//bad match
			continue;
		}
					
		float dPhi =  Phi2 - params2.w;//Phi2
					
		if (dPhi < -CUDART_PI_F) dPhi += 2.0f * CUDART_PI_F;
		else if (dPhi > CUDART_PI_F) dPhi -= 2.0f * CUDART_PI_F;
		if(fabsf(dPhi) > d_algo_params[10]) {
			continue;
		}
				
		float dcurv = curv2 - params2.y;
				
		if(fabsf(dcurv) > d_algo_params[11]) {
			continue;
		}
			
		d_neighbours[nei_pos + num_nei] = edge2_idx;
		d_reIndexer[edge2_idx] = 1;
				
		++num_nei;
						
		atomicAdd(&d_counters[1], 1);
	}

	d_num_neighbours[edge1_idx] = num_nei;

	if(num_nei!=0) d_reIndexer[edge1_idx] = 1;
}

__global__ void edgeReIndexingKernel_ITk(int* d_reIndexer, unsigned int* d_counters, int nEdges) {

	//each thread gets an edge      

	int edge_idx = threadIdx.x + blockIdx.x*blockDim.x;

	if(edge_idx >= nEdges) return;

	if(d_reIndexer[edge_idx] == -1) return;

	d_reIndexer[edge_idx] = atomicAdd(&d_counters[2], 1);
}

__global__ static void graphCompressionKernel_ITk(float4* d_sp_params, const int* d_orig_node_index, 
                                                  const int* d_edge_nodes, const unsigned char* d_num_neighbours, const int* d_neighbours, 
                                                  const int* d_reIndexer, int* d_output_graph, int nEdgesPerBlock, int nEdges) {
           
	int begin_edge = blockIdx.x * nEdgesPerBlock;
	int edge_size  = 2 + 1 + TrigAccel::ITk::GBTS_MAX_NUM_NEIGHBOURS;

	for(int idx = threadIdx.x + begin_edge;idx < begin_edge + nEdgesPerBlock; idx += blockDim.x) {

		if (idx >= nEdges) continue;

		int newIdx = d_reIndexer[idx];
		if (newIdx == -1) continue;
		 
		int pos = edge_size*newIdx;
		int node1_idx = d_orig_node_index[d_edge_nodes[2*idx]];
		d_output_graph[pos + TrigAccel::ITk::node1] = node1_idx;
		int node2_idx = d_orig_node_index[d_edge_nodes[2*idx + 1]];
		d_output_graph[pos + TrigAccel::ITk::node2] = node2_idx;

		unsigned char nNei = d_num_neighbours[idx];
		d_output_graph[pos + TrigAccel::ITk::nNei] = nNei;

		int nei_pos = TrigAccel::ITk::GBTS_MAX_NUM_NEIGHBOURS*idx;
		for(int k=0;k<nNei;k++) {
			d_output_graph[pos + TrigAccel::ITk::nei_idx_start + k] = d_reIndexer[d_neighbours[nei_pos + k]];
		}	
	}
}

#endif
