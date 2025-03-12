/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ITRIGGERPROCESSORTOOL_H
#define ITRIGGERPROCESSORTOOL_H 1

#include "GaudiKernel/IAlgTool.h"
#include "MuonRDO/NSW_PadTriggerDataContainer.h"
#include "MuonRDO/NSW_TrigRawDataContainer.h"

namespace NSWL1 {

  class ITriggerProcessorTool: virtual public IAlgTool {

    public:
      DeclareInterfaceID(ITriggerProcessorTool, 1 ,0);
      virtual ~ITriggerProcessorTool() = default;

      virtual StatusCode mergeRDO(const Muon::NSW_PadTriggerDataContainer* padTriggerContainer, const Muon::NSW_TrigRawDataContainer* stripTriggerContainer,
                                  const Muon::NSW_TrigRawDataContainer* MMTriggerContainer, Muon::NSW_TrigRawDataContainer* trigRdoContainer) const = 0;
  };
}
#endif
