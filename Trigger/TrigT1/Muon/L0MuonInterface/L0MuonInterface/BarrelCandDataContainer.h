/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L0MuonInterface_BARRELCANDDATACONTAINER_H
#define L0MuonInterface_BARRELCANDDATACONTAINER_H

#include "L0MuonInterface/BarrelCandData.h"
#include "AthContainers/DataVector.h"
#include "AthenaKernel/CLASS_DEF.h"

namespace L0Muon
{
  class BarrelCandDataContainer : public DataVector<BarrelCandData>
  {
  public:
    // default constructor
    BarrelCandDataContainer() = default;
    ~BarrelCandDataContainer() = default;

  private:
    
  };
} // namespace L0Muon

CLASS_DEF( L0Muon::BarrelCandDataContainer , 1278850067 , 1 )

#endif // L0MuonInterface_BARRELCANDDATACONTAINER_H
