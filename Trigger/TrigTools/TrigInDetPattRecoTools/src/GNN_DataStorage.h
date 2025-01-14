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

#include "TrkSpacePoint/SpacePoint.h"

class TrigFTF_GNN_Geometry;

template<typename externalSP>
struct TrigFTF_GNN_Node {

  TrigFTF_GNN_Node(unsigned short l) : m_x(0), m_y(0), m_z(0), m_r(0), m_phi(0), m_layer(l), m_pSP(nullptr), m_pcw(0) {};

  inline float x() const {return m_x;}
  inline float y() const {return m_y;}
  
  inline float phi() const {return m_phi;}
  inline float z() const {return m_z;}
  inline float r() const {return m_r;}
  inline unsigned short layer() const {return m_layer;}
  inline float pixelClusterWidth() const {return m_pcw;}
  
  inline externalSP sp() const {return m_pSP;}
  
  float m_x, m_y, m_z, m_r, m_phi;
  unsigned short m_layer{10000};
  externalSP m_pSP{nullptr};
  float m_pcw;
};

template<typename externalSP>
class TrigFTF_GNN_EtaBin {
public:

   struct CompareNodesByPhi {

    bool operator()(const TrigFTF_GNN_Node<externalSP>* n1, const TrigFTF_GNN_Node<externalSP>* n2) {
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

  float getMinBinRadius() const {
    return m_minRadius;
  }
  
  float getMaxBinRadius() const {
    return m_maxRadius;
  }
  
  std::vector<const TrigFTF_GNN_Node<externalSP>*> m_vn;//nodes of the graph
  std::vector<std::pair<float, unsigned int> > m_vPhiNodes;
  std::vector<std::vector<unsigned int> > m_in;//vectors of incoming edges
  std::vector<std::array<float,5> > m_params;//node attributes: m_minCutOnTau, m_maxCutOnTau, m_phi, m_r, m_z;

  float m_minRadius, m_maxRadius;
  
};

template<typename externalSP>
class TrigFTF_GNN_DataStorage {
public:
  TrigFTF_GNN_DataStorage(const TrigFTF_GNN_Geometry&);
  ~TrigFTF_GNN_DataStorage();

  int loadPixelGraphNodes(short, const std::vector<TrigFTF_GNN_Node<externalSP>>&, bool);
  int loadStripGraphNodes(short, const std::vector<TrigFTF_GNN_Node<externalSP>>&);
  
  unsigned int numberOfNodes() const;
  void sortByPhi();
  void initializeNodes(bool);
  void generatePhiIndexing(float);


  TrigFTF_GNN_EtaBin<externalSP>& getEtaBin(int idx) {
    if(idx >= static_cast<int>(m_etaBins.size())) idx = idx-1;
    return m_etaBins.at(idx);
  }

protected:

  const TrigFTF_GNN_Geometry& m_geo;

  std::vector<TrigFTF_GNN_EtaBin<externalSP>> m_etaBins; 

};

template<typename externalSP>
class TrigFTF_GNN_Edge {
public:

  struct CompareLevel {
  public:
    bool operator()(const TrigFTF_GNN_Edge<externalSP>* pS1, const TrigFTF_GNN_Edge<externalSP>* pS2) {
      return pS1->m_level > pS2->m_level;
    }
  };
  
  TrigFTF_GNN_Edge(const TrigFTF_GNN_Node<externalSP>* n1, const TrigFTF_GNN_Node<externalSP>* n2, float p1, float p2, float p3) : m_n1(n1), m_n2(n2), m_level(1), m_next(1), m_nNei(0) {
    m_p[0] = p1;
    m_p[1] = p2;
    m_p[2] = p3;
  }
  
  TrigFTF_GNN_Edge() : m_n1(nullptr), m_n2(nullptr), m_level(-1), m_next(-1), m_nNei(0) {};
  
  
  const TrigFTF_GNN_Node<externalSP>* m_n1{nullptr};
  const TrigFTF_GNN_Node<externalSP>* m_n2{nullptr};

  signed char m_level{-1}, m_next{-1};

  unsigned char m_nNei{0};
  float m_p[3]{};
  
  unsigned int m_vNei[N_SEG_CONNS]{};//global indices of the connected edges

};

#endif

#include "GNN_DataStorage.ipp"