/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IPFUNIFIEDBASETOOL_H
#define IPFUNIFIEDBASETOOL_H

#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/EventContext.h"

struct PFData;

class IPFUnifiedBaseTool : virtual public IAlgTool {

 public:

  /** Process the shared PFData payload */
  virtual StatusCode processPFlowData(const EventContext& ctx, PFData&) const = 0;

  DeclareInterfaceID(IPFUnifiedBaseTool,1,0);

};
#endif
