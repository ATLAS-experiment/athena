/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGINDETPATTRECOTOOLS_FASTRACK_CONNECTOR_H
#define TRIGINDETPATTRECOTOOLS_FASTRACK_CONNECTOR_H

#include<fstream>
#include<vector>
#include<map>

typedef struct GNN_FasTrackConnection {
  
  GNN_FasTrackConnection(unsigned int, unsigned int);
  ~GNN_FasTrackConnection() {};

  unsigned int m_src, m_dst;
  std::vector<int> m_binTable;

} GNN_FASTRACK_CONNECTION;


typedef class GNN_FasTrackConnector {

 public:

  struct LayerGroup {
  LayerGroup(unsigned int l1Key, const std::vector<const GNN_FASTRACK_CONNECTION*>& v) : m_dst(l1Key), m_sources(v) {};

    unsigned int m_dst;//the target layer of the group
    std::vector<const GNN_FASTRACK_CONNECTION*> m_sources;//the source layers of the group
  };

 public:

  GNN_FasTrackConnector(std::ifstream&, bool LRTmode);
  ~GNN_FasTrackConnector();

  float m_etaBin;

  std::map<int, std::vector<struct LayerGroup> > m_layerGroups;
  std::map<int, std::vector<GNN_FASTRACK_CONNECTION*> > m_connMap;

} GNN_FASTRACK_CONNECTOR;

#endif
