/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CAVERNINFRAGEOMODEL_CAVERNINFRADETECTORMANAGER_H
#define CAVERNINFRAGEOMODEL_CAVERNINFRADETECTORMANAGER_H

#include "GeoModelKernel/GeoVPhysVol.h"
#include "GeoModelKernel/GeoVDetectorManager.h"
#include <vector>

class CavernInfraDetectorManager : public GeoVDetectorManager  
{
 public:
  CavernInfraDetectorManager();

  const CavernInfraDetectorManager & operator=(const CavernInfraDetectorManager &right) = delete;
  CavernInfraDetectorManager(const CavernInfraDetectorManager &right) = delete;
  
  ~CavernInfraDetectorManager();

  // Access to raw geometry:
  virtual unsigned int getNumTreeTops() const override;
  virtual PVConstLink getTreeTop(unsigned int i) const override;
 
  void addTreeTop(PVLink);      // Add a Tree top:

 private:  
  // Tree Tops
  std::vector<PVLink> m_treeTops;
};

#ifndef GAUDI_NEUTRAL
#include "AthenaKernel/CLASS_DEF.h" 
CLASS_DEF(CavernInfraDetectorManager, 1500000300, 1)
#endif

#endif


