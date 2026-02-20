/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "GNNR3_FasTrackConnector.h"
#include <iostream>
#include <cstring>

#include <list>
#include <set>
#include <unordered_map>

GNNR3_FasTrackConnection::GNNR3_FasTrackConnection(unsigned int s, unsigned int d) : m_src(s), m_dst(d) { 

}

GNNR3_FasTrackConnector::GNNR3_FasTrackConnector(std::ifstream& inFile, bool LRTmode) {

  m_connMap.clear();
  m_layerGroups.clear();

  int nLinks{};

  inFile >> nLinks >> m_etaBin;
  
  //nLinks _should_ be checked against sensible bounds
  
  unsigned int stage{}, lIdx{}, src{}, dst{}, nEntries{};
  int height{}, width{};
  int dummy{};

  //coverity[TAINTED_SCALAR]
  for(int l=0;l<nLinks;l++) {
    inFile >> lIdx >> stage >> src >> dst >> height >> width >> nEntries;
    GNNR3_FASTRACK_CONNECTION* pC = new GNNR3_FASTRACK_CONNECTION(src, dst);
    

    //coverity[TAINTED_SCALAR]
    for(int i=0;i<height;i++) {
      //coverity[TAINTED_SCALAR]
      for(int j=0;j<width;j++) inFile >> dummy;//pC->m_binTable[j+i*width];
    }

    int srcvol_id = src / 1000;
    int dstvol_id = dst / 1000;

    bool srcIsStrip = (srcvol_id == 13 || srcvol_id == 12 || srcvol_id == 14);
    bool dstIsStrip = (dstvol_id == 13 || dstvol_id == 12 || dstvol_id == 14);
    if (LRTmode) {
      if ( !srcIsStrip || !dstIsStrip) {
        delete pC;
        continue;
      }
    } else {
      if ( srcIsStrip || dstIsStrip) {
        delete pC;
        continue;
      }
    }

    std::map<int, std::vector<GNNR3_FASTRACK_CONNECTION*> >::iterator it = m_connMap.find(stage);
    
    if(it == m_connMap.end()) {
      std::vector<GNNR3_FASTRACK_CONNECTION*> v = {pC};
      m_connMap.insert(std::make_pair(stage, v));
    } else (*it).second.push_back(pC);
  }

  //re-arrange the connection stages

  std::list<const GNNR3_FASTRACK_CONNECTION*> lConns;

  std::map<int, std::vector<const GNNR3_FASTRACK_CONNECTION*> > newConnMap;
  
  for(const auto& conn : m_connMap) {
    std::copy(conn.second.begin(), conn.second.end(), std::back_inserter(lConns));
  }

  int stageCounter = 0;

  while(!lConns.empty()) {

    std::unordered_map<unsigned int, std::pair<int, int> > mCounter;//layerKey, nDst, nSrc

    for(const auto& conn : lConns) {
      auto entryIt = mCounter.find(conn->m_dst);
      if(entryIt != mCounter.end()) {
	(*entryIt).second.first++;
      }
      else {
	int nDst = 1;
	int nSrc = 0;
	mCounter.insert(std::make_pair(conn->m_dst, std::make_pair(nDst, nSrc)));
      }

      entryIt = mCounter.find(conn->m_src);
      if(entryIt != mCounter.end()) {
	(*entryIt).second.second++;
      }
      else {
	int nDst = 0;
	int nSrc = 1;
	mCounter.insert(std::make_pair(conn->m_src, std::make_pair(nDst, nSrc)));
      }
    }

    //find layers with nSrc = 0

    std::set<unsigned int> zeroLayers;

    for(const auto& layerCounts : mCounter) {
      
      if(layerCounts.second.second!=0) continue;

      zeroLayers.insert(layerCounts.first);
    }

    //remove connections which use zeroLayer as destination

    std::vector<const GNNR3_FASTRACK_CONNECTION*> theStage;

    std::list<const GNNR3_FASTRACK_CONNECTION*>::iterator cIt = lConns.begin();

    while(cIt!=lConns.end()) {
      if(zeroLayers.find((*cIt)->m_dst) != zeroLayers.end()) {//check if contains
	theStage.push_back(*cIt);
	cIt = lConns.erase(cIt);
	continue;
      }
      ++cIt;
    }
    newConnMap.insert(std::make_pair(stageCounter, theStage));
    stageCounter++;
  }
  
  //create layer groups

  int currentStage = 0;

  //the doublet making is done using "outside-in" approach hence the reverse iterations

  for(std::map<int, std::vector<const GNNR3_FASTRACK_CONNECTION*> >::reverse_iterator it = newConnMap.rbegin();it!=newConnMap.rend();++it, currentStage++) {

    const std::vector<const GNNR3_FASTRACK_CONNECTION*> & vConn = (*it).second;
    
    //loop over links, extract all connections for the stage, group sources by L1 (dst) index
    
    std::map<unsigned int, std::vector<const GNNR3_FASTRACK_CONNECTION*> > l1ConnMap;

    for(const auto* conn : vConn) {

      unsigned int dst = conn->m_dst;

      std::map<unsigned int, std::vector<const GNNR3_FASTRACK_CONNECTION*> >::iterator l1MapIt = l1ConnMap.find(dst);
      if(l1MapIt != l1ConnMap.end()) 
	(*l1MapIt).second.push_back(conn);
      else {
	std::vector<const GNNR3_FASTRACK_CONNECTION*> v = {conn};
	l1ConnMap.insert(std::make_pair(dst, v));
      } 
    }

    std::vector<LayerGroup> lgv;

    lgv.reserve(l1ConnMap.size());

    for(const auto& l1Group : l1ConnMap) {
      lgv.push_back(LayerGroup(l1Group.first, l1Group.second));
    }
   
    m_layerGroups.insert(std::make_pair(currentStage, lgv));
  }

  newConnMap.clear();

}

GNNR3_FasTrackConnector::~GNNR3_FasTrackConnector() {

  m_layerGroups.clear();

  for(auto& conn : m_connMap) {
    for(auto& link : conn.second) delete link;
    conn.second.clear();
  }

  m_connMap.clear();

}
