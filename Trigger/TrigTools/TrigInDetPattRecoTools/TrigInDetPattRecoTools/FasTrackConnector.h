/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGINDETPATTRECOTOOLS_FASTRACK_CONNECTOR_H
#define TRIGINDETPATTRECOTOOLS_FASTRACK_CONNECTOR_H

#include<fstream>
#include<vector>
#include<map>

typedef struct FasTrackConnection {
  
  FasTrackConnection(unsigned int, unsigned int);
  ~FasTrackConnection() {};

  unsigned int m_src, m_dst;
  std::vector<int> m_binTable;

} FASTRACK_CONNECTION;


typedef class FasTrackConnector {

 public:

  struct LayerGroup {
  LayerGroup(unsigned int l1Key, const std::vector<const FASTRACK_CONNECTION*>& v) : m_dst(l1Key), m_sources(v) {};

    unsigned int m_dst;//the target layer of the group
    std::vector<const FASTRACK_CONNECTION*> m_sources;//the source layers of the group
  };

 public:

  FasTrackConnector(std::ifstream&, bool LRTmode);
  ~FasTrackConnector();

  float m_etaBin;

  std::map<int, std::vector<struct LayerGroup> > m_layerGroups;
  std::map<int, std::vector<FASTRACK_CONNECTION*> > m_connMap;

} FASTRACK_CONNECTOR;

#endif
