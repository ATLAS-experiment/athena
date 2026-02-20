/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONTGC_CNVTOOL_ITGCPREPDATAREPLICATIONTOOL_H
#define MUONTGC_CNVTOOL_ITGCPREPDATAREPLICATIONTOOL_H

#include "CxxUtils/checker_macros.h"
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/IAlgTool.h"

namespace Muon {

class ITgcPrepDataReplicationTool : virtual public IAlgTool {

   public:
    DeclareInterfaceID(ITgcPrepDataReplicationTool, 1, 0);

    virtual StatusCode replicate(const EventContext& ctx) const = 0;
};

}  // namespace Muon

#endif  // MUONTGC_CNVTOOL_ITGCPREPDATAREPLICATIONTOOL_H
