/*
 *   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 *   */


#ifndef L1TopoSimulation_IControlHistSvc
#define L1TopoSimulation_IControlHistSvc


#include "GaudiKernel/IAlgTool.h"
#include "L1TopoCoreSim/TopoSteering.h"

namespace LVL1 {


  class IControlHistSvc : virtual public extend_interfaces<IAlgTool> {


    public:
  
      DeclareInterfaceID(IControlHistSvc, 0, 1);

      virtual StatusCode SetHistSvc(const std::unique_ptr<TCS::TopoSteering> &topoSteering, std::string histBaseDir) = 0;
  };

}

#endif

