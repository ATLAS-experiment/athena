/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Tadej Novak

#ifndef TRIGGER_ANALYSIS_ALGORITHMS__TRIG_EVENT_SELECTION_ALG_H
#define TRIGGER_ANALYSIS_ALGORITHMS__TRIG_EVENT_SELECTION_ALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <AsgTools/PropertyWrapper.h>
#include <AsgTools/ToolHandle.h>
#include <EventBookkeeperTools/FilterReporterParams.h>
#include <TrigDecisionInterface/ITrigDecisionTool.h>
#include <TrigDecisionInterface/Conditions.h>
#include <AsgTools/PropertyWrapper.h>

namespace CP
{
  class TrigEventSelectionAlg : public EL::AnaAlgorithm
  {
  public:
    TrigEventSelectionAlg(const std::string &name,
                          ISvcLocator *svcLoc = nullptr);

    virtual StatusCode initialize() final;
    virtual StatusCode execute() final;
    virtual StatusCode finalize() final;

  private:
    /// \brief trigger decision tool handle
    ToolHandle<Trig::ITrigDecisionTool> m_trigDecisionTool;

    /// \brief list of triggers or trigger chains
    Gaudi::Property<std::vector<std::string>> m_trigList {this, "triggers", {}, "trigger selection list"};

    /// \brief the decoration for trigger selection
    Gaudi::Property<std::string> m_selectionDecoration {this, "selectionDecoration", "", "the decoration the trigger pass status"};

    /// \brief whether to not apply an event filter
    Gaudi::Property<bool> m_noFilter {this, "noFilter", false, "whether to not apply an event filter"};

    /// \brief whether to not require L1 decision
    Gaudi::Property<bool> m_noL1 {this, "noL1", false, "whether to not require L1 decision"};

    /// \brief the accessors for \ref m_selectionDecoration and \ref m_trigList combination
    std::vector<SG::Decorator<bool>> m_selectionAccessors;

    /// \brief the filter reporter params
    FilterReporterParams m_filterParams {this, "TriggerEventSelection", "trigger event selection"};
  };
}

#endif
