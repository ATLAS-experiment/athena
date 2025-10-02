/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGINDETCUDA_DEVCONTEXTS_H
#define TRIGINDETCUDA_DEVCONTEXTS_H

#include "TrigAccelEvent/TrigInDetAccelEDM.h"

#include "TrigAccelEvent/TrigITkAccelEDM.h"

#include <cuda_runtime.h>
#include <tbb/tick_count.h>

#include "CommonStructures.h"


struct SeedMakingDeviceContext {
public:
  SeedMakingDeviceContext() : m_deviceId(-1), h_spacepoints(0), d_spacepoints(0), d_size(0), h_size(0) {};
  size_t hostSize() { return h_size;}
  size_t deviceSize() { return d_size;}
  
  int m_deviceId{};
  cudaStream_t m_stream{};
  unsigned char *h_settings{};
  unsigned char *d_settings{};
  unsigned char *h_spacepoints{};
  unsigned char *d_spacepoints{};
  
  unsigned char *d_detmodel{};
  
  unsigned char *h_outputseeds{};
  unsigned char *d_outputseeds{};
  
  unsigned char *d_doubletstorage{};
  unsigned char *d_doubletinfo{};
  
  size_t d_size{}, h_size{};
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
  
  int m_deviceId{};
  cudaStream_t m_stream{};


  unsigned char *m_settings{};


  unsigned char *m_spacepoints{};

  unsigned char *d_detmodel{};
  



  unsigned char *m_outputseeds{};
  unsigned char *m_confirmedseeds{};
  
  unsigned char *d_doubletstorage{};
  unsigned char *d_doubletinfo{};
  
  size_t d_size{}, h_size{}, m_size{};
  GPU_PARAMETERS m_gpuParams;
  
private:
  SeedMakingManagedDeviceContext(const SeedMakingManagedDeviceContext& sc) : m_deviceId(sc.m_deviceId) {};
};

struct GbtsDeviceContext {
public:
  GbtsDeviceContext() = default;
  size_t hostSize() { return h_size;}
  size_t deviceSize() { return d_size;}
  
  int m_deviceId{-1};
  cudaStream_t m_stream;
  size_t d_size{}, h_size{};

  int    m_nLayers{};
  int    m_nBinPairs{};
  int    m_maxEtaBin{};
  int    m_nNodes{};
  unsigned int m_nMaxEdges{};
		
  int m_minLevel{};
	
  bool m_useGPUseedExtraction{};
 
  //1. Input data structures on GPU
  
  float* d_sp_params{};//spacepoint parameters x, y, z, w
  int*   d_layer_info{};
  float* d_layer_geo{};

  //2. GBTS algorithm parameters
  
  float* d_algo_params{};

  //3. Graph nodes on GPU
  
  float* d_node_params{};//tau1, tau2, r, phi, z
  int* d_node_index{};//original spacepoint index
  int* d_eta_bin_views{};//views of the nodes
  float* d_bin_rads{};//minimum and maximum r of nodes inside an eta-bin
  
  int* d_node_eta_index{};//for data binning
  int* d_node_phi_index{};//for data binning
  
  unsigned int* d_eta_phi_histo{};//for data binning
  unsigned int* d_phi_cusums{};//for data binning
  unsigned int* d_eta_node_counter{};//for data binning
  
  int*   h_eta_bin_views{};//eta-bin views of the node_params array
  float* h_bin_rads{};
  
  unsigned int* h_bin_pair_views{};
  unsigned int* d_bin_pair_views{};
  
  float* h_bin_pair_dphi{};
  float* d_bin_pair_dphi{};
  
  int* d_edge_nodes{};
  float* d_edge_params{};
  
  unsigned int* d_num_incoming_edges{}; 
  int* d_edge_links{};
  
  unsigned char* d_num_neighbours{};
  int* d_reIndexer{};
  int* d_neighbours{};
  unsigned int*   d_counters{};
	
  // message-passing CCA
  
  int* d_active_edges{}; //holds indices of the edges that need more CCA iterations
  char* d_levels{}; //d_levels[edge_idx] = the maxium length of seeds starting with this edge  
  int* d_level_views{}; //edge indices by level
  int* d_level_boundaries{}; //number of edges for each level in the above
	
  // seed-extraction walkthrough
  
  int* d_mini_states{}; // edge_idx and prev mini_state forms a seeds uniuqe path through the graph
  int* d_seed_proposals{}; // int quality and final mini_state_idx
  float* d_state_store{}; // global overflow of live edgeStates 
  unsigned long long int* d_edge_bids{}; //first 32 bits are seed quality second 32 bits are seed_proposals index [int_m_J, prop_idx] 
  char* d_seed_ambiguity{}; //0 as default/is real seed, 1 as maybe seed, -1 as maybe not seed, -2 as fake 

  //output stats
  
  unsigned int    m_nEdges{};
  unsigned int    m_nLinks{};
  unsigned int    m_nUniqueEdges{};
  unsigned int    m_nSeeds{}; 
 
  //output array

  TrigAccel::ITk::Tracklet* d_seeds{};
 
  int* d_output_graph{};
  //stores each edges node-pair, number of connections and the self-refrencing indexes of those connections  
  
private:
  GbtsDeviceContext(const GbtsDeviceContext& sc) : m_deviceId(sc.m_deviceId) {};
};


#endif
