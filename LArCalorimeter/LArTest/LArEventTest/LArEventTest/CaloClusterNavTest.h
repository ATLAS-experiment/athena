/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CALOCLUSTERNAVTEST_H
#define CALOCLUSTERNAVTEST_H

#include "AthenaBaseComps/AthAlgorithm.h"

class CaloClusterNavTest: public AthAlgorithm
{

 public:    
  
  using AthAlgorithm::AthAlgorithm;
  virtual ~CaloClusterNavTest();
  
  virtual StatusCode execute(const EventContext& ctx) ; 
  virtual StatusCode initialize();
  virtual StatusCode finalize();
  
 private: 
 Gaudi::Property<std::vector<std::string>> m_clusterContainerNames{this, "ClusterContainerNames", {}};
};

#endif 
