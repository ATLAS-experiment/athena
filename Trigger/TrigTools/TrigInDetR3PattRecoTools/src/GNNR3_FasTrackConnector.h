/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGINDETPATTRECOTOOLS_FASTRACK_CONNECTOR_H
#define TRIGINDETPATTRECOTOOLS_FASTRACK_CONNECTOR_H

#include<fstream>
#include<vector>
#include<map>

typedef struct GNNR3_FasTrackConnection {
  
  GNNR3_FasTrackConnection(unsigned int, unsigned int);
  ~GNNR3_FasTrackConnection() {};

  unsigned int m_src, m_dst;
  std::vector<int> m_binTable;

} GNNR3_FASTRACK_CONNECTION;


typedef class GNNR3_FasTrackConnector {

 public:

  struct LayerGroup {
  LayerGroup(unsigned int l1Key, const std::vector<const GNNR3_FASTRACK_CONNECTION*>& v) : m_dst(l1Key), m_sources(v) {};

    unsigned int m_dst;//the target layer of the group
    std::vector<const GNNR3_FASTRACK_CONNECTION*> m_sources;//the source layers of the group
  };

 public:

  GNNR3_FasTrackConnector(std::ifstream&, bool LRTmode);
  ~GNNR3_FasTrackConnector();

  float m_etaBin;

  std::map<int, std::vector<struct LayerGroup> > m_layerGroups;
  std::map<int, std::vector<GNNR3_FASTRACK_CONNECTION*> > m_connMap;

} GNNR3_FASTRACK_CONNECTOR;

#endif
