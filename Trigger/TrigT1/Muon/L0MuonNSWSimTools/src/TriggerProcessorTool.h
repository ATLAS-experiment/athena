/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGGERPROCESSORTOOL_H
#define TRIGGERPROCESSORTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "L0MuonNSWSimTools/ITriggerProcessorTool.h"

namespace L0Muon {

  class TriggerProcessorTool : public extends<AthAlgTool,ITriggerProcessorTool> {

  public:

    TriggerProcessorTool(const std::string& type, const std::string& name, const IInterface* parent);
    virtual ~TriggerProcessorTool() override = default;

    virtual StatusCode initialize() override;

    StatusCode mergeRDO(const Muon::NSW_PadTriggerDataContainer* padTriggerContainer,
                        const Muon::NSW_TrigRawDataContainer* stripTriggerContainer,
                        const Muon::NSW_TrigRawDataContainer* MMTriggerContainer,
                        Muon::NSW_TrigRawDataContainer* trigRdoContainer) const override;

  private:

  };
}
#endif
