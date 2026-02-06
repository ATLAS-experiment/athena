/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonReadoutGeometryR4/ToroidDetectorManager.h"

#include <cassert>
namespace MuonGMR4 {
    ToroidDetectorManager::ToroidDetectorManager(const std::string& name){
        setName(name);
    }       
    unsigned int ToroidDetectorManager::getNumTreeTops () const {
        return m_treeTops.size();
    }    
    PVConstLink ToroidDetectorManager::getTreeTop (unsigned int i) const {
        assert(i < m_treeTops.size()); 
        return m_treeTops.at(i);
    }
    void ToroidDetectorManager::addTreeTop(PVConstLink pvLink) {
        m_treeTops.push_back(pvLink);
    }
}
