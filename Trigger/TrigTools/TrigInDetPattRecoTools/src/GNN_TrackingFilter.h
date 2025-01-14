/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGINDETPATTRECOTOOLS_GNN_TRACKING_FILTER_H
#define TRIGINDETPATTRECOTOOLS_GNN_TRACKING_FILTER_H

#include "GNN_DataStorage.h"
#include "TrigInDetPattRecoEvent/TrigInDetSiLayer.h"

template<typename externalSP>
struct TrigFTF_GNN_EdgeState {

public:

struct Compare {
    bool operator()(const struct TrigFTF_GNN_EdgeState<externalSP>* s1, const struct TrigFTF_GNN_EdgeState<externalSP>* s2) {
      return s1->m_J > s2->m_J;
    }
  };


  TrigFTF_GNN_EdgeState() {};

TrigFTF_GNN_EdgeState(bool f) : m_initialized(f) {};

  ~TrigFTF_GNN_EdgeState() {};

  void initialize(TrigFTF_GNN_Edge<externalSP>*);
  void clone(const struct TrigFTF_GNN_EdgeState<externalSP>&);

  float m_J{};

  std::vector<TrigFTF_GNN_Edge<externalSP>*> m_vs;

  float m_X[3]{}, m_Y[2]{}, m_Cx[3][3]{}, m_Cy[2][2]{};
  float m_refX{}, m_refY{}, m_c{}, m_s{};
  
  bool m_initialized{false};

};

#define MAX_EDGE_STATE 2500

template<typename externalSP>
class TrigFTF_GNN_TrackingFilter {
 public:
  TrigFTF_GNN_TrackingFilter(const std::vector<TrigInDetSiLayer>&, std::vector<TrigFTF_GNN_Edge<externalSP>>&);
  ~TrigFTF_GNN_TrackingFilter(){};

  void followTrack(TrigFTF_GNN_Edge<externalSP>*, TrigFTF_GNN_EdgeState<externalSP>&);

 protected:

  void propagate(TrigFTF_GNN_Edge<externalSP>*, TrigFTF_GNN_EdgeState<externalSP>&);

  bool update(TrigFTF_GNN_Edge<externalSP>*, TrigFTF_GNN_EdgeState<externalSP>&);

  int getLayerType(int);  


  const std::vector<TrigInDetSiLayer>& m_geo;
  
  std::vector<TrigFTF_GNN_Edge<externalSP>>& m_segStore;
 
  std::vector<TrigFTF_GNN_EdgeState<externalSP>*> m_stateVec;

  TrigFTF_GNN_EdgeState<externalSP> m_stateStore[MAX_EDGE_STATE];

  int m_globalStateCounter{0};

};

#endif

#include "GNN_TrackingFilter.ipp"