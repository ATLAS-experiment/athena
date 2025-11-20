/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISF_TFCSMLCALORIMETERSIMULATOR_H
#define ISF_TFCSMLCALORIMETERSIMULATOR_H

// ISF includes
#include <map>
#include <vector>
#include <string>
#include <fstream>

#include "ISF_FastCaloSimEvent/TFCSSimulationState.h"
#include "ISF_FastCaloSimEvent/MLogging.h"

// generic network class
#include "ISF_FastCaloSimEvent/VNetworkBase.h"




class TFCSMLCalorimeterSimulator : public ISF_FCS::MLogging {
public:
  TFCSMLCalorimeterSimulator();
  virtual ~TFCSMLCalorimeterSimulator();

  typedef struct {
    std::vector<unsigned int> bin_index_vector;
    std::vector<float> E_vector;
  } layer_t;

  typedef struct {
    std::vector<layer_t> event_data;
  } event_t;

  bool loadSimulator(std::string filename);

  void Print() const;

  VNetworkBase::NetworkOutputs predictVoxels(TFCSSimulationState &simulstate, float eta, float energy) const;
  event_t getEvent(TFCSSimulationState &simulstate, float eta, float energy) const;
  VNetworkBase::NetworkOutputs predictVoxels() const;

  void setInputShapes(std::vector<long unsigned int> layer_boundaries, std::vector<long unsigned int> used_layers) {
    m_layer_boundaries = layer_boundaries;
    m_used_layers = used_layers;
    m_nVoxels = layer_boundaries.back();
    m_nLayers = used_layers.size();
  };

private:

  std::unique_ptr<VNetworkBase> m_onnx_model = nullptr;

  int m_nEvents = 1; // Currently no batching supported from ONNX handler

  // Default shapes for the photon barrel-CFM
  // Should be set using setInputShapes for other models
  std::vector<long unsigned int> m_layer_boundaries = {0, 36, 200, 310, 346, 382};
  std::vector<long unsigned int> m_used_layers = {0, 1, 2, 3, 12};
  long unsigned int m_nVoxels = 382;
  long unsigned int m_nLayers= 5;


  ClassDef(TFCSMLCalorimeterSimulator, 1) // TFCSMLCalorimeterSimulator
};

#endif //> !ISF_TFCSMLCALORIMETERSIMULATOR_H
