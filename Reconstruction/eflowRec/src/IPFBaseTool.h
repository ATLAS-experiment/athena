/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IPFBASETOOL_H
#define IPFBASETOOL_H

#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/EventContext.h"

struct PFData;

class IPFBaseTool : virtual public IAlgTool {

 public:

  /** Process the shared PFData payload */
  virtual StatusCode processPFlowData(const EventContext& ctx, PFData&) const = 0;

  DeclareInterfaceID(IPFBaseTool,1,0);

};
#endif
