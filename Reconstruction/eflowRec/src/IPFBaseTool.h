/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EFLOWREC_IPFBASETOOL_H
#define EFLOWREC_IPFBASETOOL_H

#include "GaudiKernel/IAlgTool.h"

class EventContext;
class eflowCaloObjectContainer;

class IPFBaseTool : virtual public IAlgTool {

 public:

  /** Execute method to use eflowCaloObjectContainer */
  virtual StatusCode execute(const EventContext& ctx, eflowCaloObjectContainer&) = 0;

  DeclareInterfaceID(IPFBaseTool,1,0);

};
#endif
