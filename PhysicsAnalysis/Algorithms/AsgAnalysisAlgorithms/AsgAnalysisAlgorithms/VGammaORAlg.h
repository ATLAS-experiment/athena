/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Lucas Cremer

#ifndef ASG_VGAMMAORALG_H
#define ASG_VGAMMAORALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <AsgTools/PropertyWrapper.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysFilterReporterParams.h>

#include <xAODEventInfo/EventInfo.h>

#include "GammaORTools/IVGammaORTool.h"

namespace CP {

  class VGammaORAlg final : public EL::AnaAlgorithm {

  public:
    using EL::AnaAlgorithm::AnaAlgorithm;
    virtual StatusCode initialize() override;
    virtual StatusCode execute() override;
    virtual StatusCode finalize() override;

  private:
    /// \brief the systematics
    CP::SysListHandle m_systematicsList {this};

    /// \brief the overlap removal tool
    ToolHandle<IVGammaORTool> m_vgammaORTool {
      this, "VGammaORTool", "", "the VGammaORTool"
    };

    /// \brief the event info handle
    CP::SysReadHandle<xAOD::EventInfo> m_eventInfoHandle {
      this, "eventInfoContainer", "EventInfo", "the input EventInfo container"
    };

    /// \brief the decoration for the tool decision
    CP::SysWriteDecorHandle<bool> m_inOverlapHandle {
      this, "in_vgamma_overlap", "in_vgamma_overlap_%SYS%", "decoration name for the VGammaORTool overlap flag"
    };

    /// \brief the event filter for the tool decision
    CP::SysFilterReporterParams m_filterParams {
      this, "VGamma overlap filter"
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
