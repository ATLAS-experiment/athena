/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGACCELEVENT_TRIGINDETACCELEDM_ITK_H
#define TRIGACCELEVENT_TRIGINDETACCELEDM_ITK_H

#include<cstdint>
#include<memory>

namespace TrigAccel {

namespace ITk {

  //A. GPU-accelerated track seeding
  
  static constexpr unsigned int MAX_SILICON_LAYERS           = 216;
  static constexpr unsigned int MAX_NUMBER_PIX_MODULES       = 6300;
  static constexpr unsigned int MAX_NUMBER_SCT_MODULES       = 24600;
  static constexpr unsigned int MAX_NUMBER_SPACEPOINTS       = 300000;
  static constexpr unsigned int MAX_PHI_SLICES               = 100;
  static constexpr unsigned int MAX_NUMBER_OUTPUT_SEEDS      = 250000;
  
  typedef struct SiliconLayer {
  public:
    int m_subdet;//1 : Pixel, 2 : Strips
    int m_type;//0: barrel, +/-n : endcap
    float m_refCoord;
    int m_nElements;
    float m_minBound, m_maxBound;
    float m_phiBinWidth, m_rzBinWidth;
    int m_nPhiSlices;

  } SILICON_LAYER;

  typedef struct DetectorModel {
  public:
    int m_nLayers;
    int m_nModules;
    SILICON_LAYER m_layers[MAX_SILICON_LAYERS];
    int m_middleSpacePointLayers[MAX_SILICON_LAYERS];
    int m_hashArray[MAX_NUMBER_PIX_MODULES+MAX_NUMBER_SCT_MODULES];
    float m_minRZ[MAX_NUMBER_PIX_MODULES+MAX_NUMBER_SCT_MODULES];
    float m_maxRZ[MAX_NUMBER_PIX_MODULES+MAX_NUMBER_SCT_MODULES];
  } DETECTOR_MODEL;

  typedef struct SpacePointLayerRange {
  public:
    int m_layerBegin[MAX_SILICON_LAYERS];
    int m_layerEnd[MAX_SILICON_LAYERS];
  } SPACEPOINT_LAYER_RANGE;
 
  typedef struct SpacePointStorage {
  public:
    int m_nSpacepoints;
    int m_nPhiSlices;
    int m_nLayers;
    int m_nMiddleLayers;
    int m_index[MAX_NUMBER_SPACEPOINTS];
    int m_type[MAX_NUMBER_SPACEPOINTS];
    float m_x[MAX_NUMBER_SPACEPOINTS];
    float m_y[MAX_NUMBER_SPACEPOINTS];
    float m_z[MAX_NUMBER_SPACEPOINTS];
    float m_r[MAX_NUMBER_SPACEPOINTS];
    float m_phi[MAX_NUMBER_SPACEPOINTS];
    float m_covR[MAX_NUMBER_SPACEPOINTS];
    float m_covZ[MAX_NUMBER_SPACEPOINTS];
    float m_clusterWidth[MAX_NUMBER_SPACEPOINTS];
    SPACEPOINT_LAYER_RANGE m_phiSlices[MAX_PHI_SLICES];
  } SPACEPOINT_STORAGE;

  typedef struct SeedFinderSettings {
  public:
    unsigned int m_maxBarrelPix, m_minEndcapPix, m_maxEndcapPix, m_maxSiliconLayer; 
    float m_magFieldZ; 
    float m_tripletD0Max; 
    float m_tripletD0_PPS_Max; 
    float m_tripletPtMin; 
    int  m_tripletDoPSS, m_tripletDoPPS, m_doubletFilterRZ; 
    int m_nMaxPhiSlice; 
    unsigned int m_maxTripletBufferLength; 
    int m_isFullScan;
    float m_zedMinus, m_zedPlus;
    float m_maxEta, m_minDoubletLength, m_maxDoubletLength;
    float m_phiMinus, m_phiPlus;
    
  } SEED_FINDER_SETTINGS;

  typedef struct SeedMakingJob {
  public:
    SEED_FINDER_SETTINGS m_settings;
    SPACEPOINT_STORAGE m_data;
  } SEED_MAKING_JOB;

  typedef struct OutputSeedStorage {
  public:
    int m_nSeeds;
    int m_nMiddleSps;
    int m_nI, m_nO;
    int m_nErrors;
    int m_innerIndex[MAX_NUMBER_OUTPUT_SEEDS];
    int m_middleIndex[MAX_NUMBER_OUTPUT_SEEDS];
    int m_outerIndex[MAX_NUMBER_OUTPUT_SEEDS];
    float m_Q[MAX_NUMBER_OUTPUT_SEEDS];
    float m_pT[MAX_NUMBER_OUTPUT_SEEDS];
  } OUTPUT_SEED_STORAGE;

  //B: Graph-based track seeding algorithm implementation on GPU
  
  static constexpr unsigned int GBTS_MAX_NUMBER_SPACEPOINTS  = 350000;
  static constexpr unsigned int GBTS_MAX_SILICON_LAYERS      = 216;
  static constexpr unsigned int GBTS_MAX_PHI_BIN             = 120;
  static constexpr unsigned int GBTS_MAX_ETA_BIN             = 1000;
  static constexpr unsigned int GBTS_MAX_ETA_BIN_PAIR        = 8000;
  static constexpr unsigned int GBTS_NODE_BUFFER_LENGTH      = 250;
  static constexpr unsigned int GBTS_MAX_NUM_NEIGHBOURS      = 10;
  static constexpr unsigned int GBTS_MAX_CCA_ITERATIONS      = 20;
  static constexpr unsigned int GBTS_MAX_SHARED_STATES       = 544;
 
  //offsets for d_output_graph array
	static constexpr unsigned char node1 = 0;
	static constexpr unsigned char node2 = 1;
	static constexpr unsigned char nNei = 2;
	static constexpr unsigned char nei_idx_start = 3;

  typedef struct GraphMakingInputData {
  public:
    
    unsigned int m_nSpacepoints, m_nLayers, m_nEtaBins, m_maxEtaBin, m_nBinPairs, m_nMaxEdges;
    
    float m_params[4*GBTS_MAX_NUMBER_SPACEPOINTS];//x,y,z,cluster width
  
    int m_layerIdx[GBTS_MAX_SILICON_LAYERS];

    //the views for the above storage space assuming float4 packing (x,y,z,w)
    
    int m_layerInfo[4*GBTS_MAX_SILICON_LAYERS];//view begin, view end, num eta bins, first eta bin

    //eta binning geometry of the layers

    float m_layerGeo[2*GBTS_MAX_SILICON_LAYERS];//min eta, eta bin width

    //eta bin pairings

    int m_bin_pairs[2*GBTS_MAX_ETA_BIN_PAIR];

    float m_algo_params[32];//reserved space for GBTS algoritm parameters
		
    int m_minLevel;
		
    bool m_useGPUseedExtraction;		
 
  } GRAPH_MAKING_INPUT_DATA;

  typedef struct CompressedGraph {
  public:
    CompressedGraph() : m_nEdges(0), m_nMaxNeighbours(0), m_nLinks(0), m_graphArray(nullptr) {};
    unsigned int m_nEdges;
    unsigned int m_nMaxNeighbours;
    unsigned int m_nLinks;
    std::unique_ptr<int[]> m_graphArray;
  
	} COMPRESSED_GRAPH;

  struct Tracklet {
    int m_nodes[GBTS_MAX_CCA_ITERATIONS+1];
    int m_size;
    float m_Q;
  };

  typedef struct OutputSeeds {
  public:
    OutputSeeds() : m_nSeeds(0), m_seedsArray(nullptr) {};
		unsigned int m_nSeeds;
    std::unique_ptr<Tracklet[]> m_seedsArray;

  } OUTPUT_SEEDS;

  typedef struct GraphAndSeedsOutput {
  public:
    COMPRESSED_GRAPH m_CompressedGraph;
    OUTPUT_SEEDS m_OutputSeeds;
  } GRAPH_AND_SEEDS_OUTPUT;

}
}
#endif
