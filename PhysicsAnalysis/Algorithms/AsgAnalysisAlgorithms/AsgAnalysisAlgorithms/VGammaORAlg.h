/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Lucas Cremer

#ifndef ASG_VGAMMAORALG_H
#define ASG_VGAMMAORALG_H

#include <AnaAlgorithm/AnaReentrantAlgorithm.h>
#include <AsgDataHandles/ReadHandleKey.h>
#include <AsgDataHandles/WriteDecorHandleKey.h>
#include <AsgTools/PropertyWrapper.h>
#include <EventBookkeeperTools/FilterReporterParams.h>

#include <xAODEventInfo/EventInfo.h>

#include "GammaORTools/IVGammaORTool.h"

namespace CP {

  class VGammaORAlg final : public EL::AnaReentrantAlgorithm {

  public:
    using EL::AnaReentrantAlgorithm::AnaReentrantAlgorithm;
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;
    virtual StatusCode finalize() override;

  private:
    /// \brief the overlap removal tool
    ToolHandle<IVGammaORTool> m_vgammaORTool {
      this, "VGammaORTool", "", "the VGammaORTool"
    };

    /// \brief the event info key
    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey {
      this, "eventInfoContainer", "EventInfo", "the input EventInfo container"
    };

    /// \brief the decoration for the tool decision
    SG::WriteDecorHandleKey<xAOD::EventInfo> m_inOverlapKey {
      this, "in_vgamma_overlap", m_eventInfoKey, "in_vgamma_overlap", "decoration name for the VGammaORTool overlap flag"
    };

    /// \brief the event filter for the tool decision
    FilterReporterParams m_filterParams {
      this, "VGammaORFilter", "VGamma overlap filter"
    };

    /// \brief whether to not apply an event filter
    Gaudi::Property<bool> m_noFilter {
      this, "noFilter", false, "whether to disable the event filter"
    };

    /// \brief which way to run the event filter
    Gaudi::Property<bool> m_keepOverlap {
      this, "keepOverlap", false, "whether to keep events in the overlap region"
    };

  };

} // namespace

#endif
