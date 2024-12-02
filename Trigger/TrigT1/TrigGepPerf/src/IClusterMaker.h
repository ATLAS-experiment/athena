/*
 *   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

#ifndef TRIGL0GEPPERF_ICLUSTERMAKER_H
#define TRIGL0GEPPERF_ICLUSTERMAKER_H

#include <map> 
#include <string>

#include "./Cluster.h"
#include "./GepCaloCell.h"

#include <memory>

typedef std::map<unsigned int,Gep::GepCaloCell> GepCellMap;
typedef std::unique_ptr<GepCellMap> pGepCellMap;

namespace Gep{
  class IClusterMaker
  {
  public:
    
    virtual std::vector<Gep::Cluster>
    makeClusters(const pGepCellMap&) const = 0;
    
    virtual std::string getName() const = 0;
    
    virtual ~IClusterMaker() {}
    
  };
}


#endif
