/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGINDETPATTRECOTOOLS_GNNR3_DATA_STORAGE_H
#define TRIGINDETPATTRECOTOOLS_GNNR3_DATA_STORAGE_H

#include<vector>
#include<map>
#include<algorithm>
#include<array>
#include<limits>
#define MAX_SEG_PER_NODE 1000 //was 30
#define N_SEG_CONNS  6 //was 6

class TrigFTF_GNNR3_Geometry;

struct TrigFTF_GNNR3_Node {

  TrigFTF_GNNR3_Node(unsigned short l) : m_x(0), m_y(0), m_z(0), m_r(0), m_phi(0), m_layer(l), m_pcw(0) {};

  inline float x() const {return m_x;}
  inline float y() const {return m_y;}
  
  inline float phi() const {return m_phi;}
  inline float z() const {return m_z;}
  inline float r() const {return m_r;}
  inline unsigned short layer() const {return m_layer;}
  inline float pixelClusterWidth() const {return m_pcw;}
  
  inline int sp_idx() const {return m_idx;}
  
  float m_x, m_y, m_z, m_r, m_phi;
  unsigned short m_layer{10000};
  unsigned int m_idx{std::numeric_limits<unsigned int>::max()};
  float m_pcw;
};


class TrigFTF_GNNR3_EtaBin {
public:

   struct CompareNodesByPhi {

    bool operator()(const TrigFTF_GNNR3_Node* n1, const TrigFTF_GNNR3_Node* n2) {
      return n1->phi() < n2->phi();
    }

  };
  
  TrigFTF_GNNR3_EtaBin();
  ~TrigFTF_GNNR3_EtaBin();

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
  
  std::vector<const TrigFTF_GNNR3_Node*> m_vn;//nodes of the graph
  std::vector<std::pair<float, unsigned int> > m_vPhiNodes;
  std::vector<std::vector<unsigned int> > m_in;//vectors of incoming edges, stores indicies of edges in the edge vector
  std::vector<std::array<float,5> > m_params;//node attributes: m_minCutOnTau, m_maxCutOnTau, m_phi, m_r, m_z;

  float m_minRadius, m_maxRadius;
  
};

class TrigFTF_GNNR3_DataStorage {
public:
  TrigFTF_GNNR3_DataStorage(const TrigFTF_GNNR3_Geometry&);
  ~TrigFTF_GNNR3_DataStorage();

  int loadPixelGraphNodes(short, const std::vector<TrigFTF_GNNR3_Node>&, bool);
  int loadStripGraphNodes(short, const std::vector<TrigFTF_GNNR3_Node>&);
  
  unsigned int numberOfNodes() const;
  void sortByPhi();
  void initializeNodes(bool);
  void generatePhiIndexing(float);


  TrigFTF_GNNR3_EtaBin& getEtaBin(int idx) {
    if(idx >= static_cast<int>(m_etaBins.size())) idx = idx-1;
    return m_etaBins.at(idx);
  }

protected:

  const TrigFTF_GNNR3_Geometry& m_geo;

  std::vector<TrigFTF_GNNR3_EtaBin> m_etaBins; 

};

class TrigFTF_GNNR3_Edge {
public:

  struct CompareLevel {
  public:
    bool operator()(const TrigFTF_GNNR3_Edge* pE1, const TrigFTF_GNNR3_Edge* pE2) {
      return pE1->m_level > pE2->m_level;
    }
  };
  
  TrigFTF_GNNR3_Edge(const TrigFTF_GNNR3_Node* n1, const TrigFTF_GNNR3_Node* n2, float p1, float p2, float p3) : m_n1(n1), m_n2(n2), m_level(1), m_next(1), m_nNei(0) {
    m_p[0] = p1;
    m_p[1] = p2;
    m_p[2] = p3;
  }
  
  TrigFTF_GNNR3_Edge() : m_n1(nullptr), m_n2(nullptr), m_level(-1), m_next(-1), m_nNei(0) {};
  
  
  const TrigFTF_GNNR3_Node* m_n1{nullptr};
  const TrigFTF_GNNR3_Node* m_n2{nullptr};

  signed char m_level{-1}, m_next{-1};

  unsigned char m_nNei{0};
  float m_p[3]{};
  
  unsigned int m_vNei[N_SEG_CONNS]{};//global indices of the connected edges

};

#endif

