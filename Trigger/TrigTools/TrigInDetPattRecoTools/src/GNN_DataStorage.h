/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGINDETPATTRECOTOOLS_GNN_DATA_STORAGE_H
#define TRIGINDETPATTRECOTOOLS_GNN_DATA_STORAGE_H

#include<vector>
#include<map>
#include<algorithm>
#include<array>

#define MAX_SEG_PER_NODE 1000 //was 30
#define N_SEG_CONNS  6 //was 6


#include "TrigInDetEvent/TrigSiSpacePointBase.h"

class TrigFTF_GNN_Geometry;

class TrigFTF_GNN_EtaBin {
public:

   struct CompareByPhi {

    bool operator()(const TrigSiSpacePointBase* n1, const TrigSiSpacePointBase* n2) {
      return n1->phi() < n2->phi();
    }

  };
  
  TrigFTF_GNN_EtaBin();
  ~TrigFTF_GNN_EtaBin();

  void sortByPhi();
  void initializeNodes();
  bool empty() const {
    return m_vn.empty();
  }
  
  void generatePhiIndexing(float);
  
  std::vector<const TrigSiSpacePointBase*> m_vn;//nodes of the graph
  std::vector<std::pair<float, unsigned int> > m_vPhiNodes;
  std::vector<std::vector<unsigned int> > m_in;//vectors of incoming edges
  std::vector<std::array<float,5> > m_params;//node attributes: m_minCutOnTau, m_maxCutOnTau, m_phi, m_r, m_z;
};

class TrigFTF_GNN_DataStorage {
public:
  TrigFTF_GNN_DataStorage(const TrigFTF_GNN_Geometry&);
  ~TrigFTF_GNN_DataStorage();

  int addSpacePoint(const TrigSiSpacePointBase*, bool);
  unsigned int numberOfNodes() const;
  void sortByPhi();
  void initializeNodes(bool);
  void generatePhiIndexing(float);


  TrigFTF_GNN_EtaBin& getEtaBin(int idx) {
    if(idx >= static_cast<int>(m_etaBins.size())) idx = idx-1;
    return m_etaBins.at(idx);
  }

protected:

  const TrigFTF_GNN_Geometry& m_geo;

  std::vector<TrigFTF_GNN_EtaBin> m_etaBins; 

};

class TrigFTF_GNN_Edge {
public:

  struct CompareLevel {
  public:
    bool operator()(const TrigFTF_GNN_Edge* pS1, const TrigFTF_GNN_Edge* pS2) {
      return pS1->m_level > pS2->m_level;
    }
  };

  TrigFTF_GNN_Edge(const TrigSiSpacePointBase* n1, const TrigSiSpacePointBase* n2, float p1, float p2, float p3) : m_n1(n1), m_n2(n2), m_level(1), m_next(1), m_nNei(0) {
    m_p[0] = p1;
    m_p[1] = p2;
    m_p[2] = p3;
  }
  
  TrigFTF_GNN_Edge() : m_n1(nullptr), m_n2(nullptr), m_level(-1), m_next(-1), m_nNei(0) {};

  const TrigSiSpacePointBase* m_n1{nullptr};
  const TrigSiSpacePointBase* m_n2{nullptr};

  signed char m_level{-1}, m_next{-1};

  unsigned char m_nNei{0};
  float m_p[3]{};
  
  unsigned int m_vNei[N_SEG_CONNS]{};//global indices of the connected edges

};

#endif
