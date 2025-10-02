/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////// 
// IIOHepMcTool.h 
// Header file for class IIOHepMcTool
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 
#ifndef MCPARTICLEKERNEL_IIOHEPMCTOOL_H 
#define MCPARTICLEKERNEL_IIOHEPMCTOOL_H 

// FrameWork includes
#include "GaudiKernel/IAlgTool.h"


class IIOHepMcTool : virtual public extend_interfaces<IAlgTool>
{ 

 public:
  DeclareInterfaceID(IIOHepMcTool, 1, 0);

  virtual ~IIOHepMcTool() {}

  virtual StatusCode execute() = 0;

}; 


#endif //> MCPARTICLEKERNEL_IIOHEPMCTOOL_H
