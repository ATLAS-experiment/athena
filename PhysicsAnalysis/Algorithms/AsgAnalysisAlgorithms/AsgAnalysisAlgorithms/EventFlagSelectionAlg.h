/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Tadej Novak

#ifndef ASG_ANALYSIS_ALGORITHMS__EVENT_FLAG_SELECTION_ALG_H
#define ASG_ANALYSIS_ALGORITHMS__EVENT_FLAG_SELECTION_ALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <EventBookkeeperTools/FilterReporterParams.h>
#include <SelectionHelpers/ISelectionReadAccessor.h>
#include <AsgTools/PropertyWrapper.h>

namespace CP
{
  /// \brief an algorithm for selecting events by flag
  class EventFlagSelectionAlg final : public EL::AnaAlgorithm
  {
  public:
    using EL::AnaAlgorithm::AnaAlgorithm;
    virtual StatusCode initialize() final;
    virtual StatusCode execute() final;
    virtual StatusCode finalize() final;

  private:
    /// \brief flags that we want to select events with
    Gaudi::Property<std::vector<std::string>> m_selFlags {this, "selectionFlags", {}, "list of flags to use as selection criteria"};
    
    /// \brief invert flags
    Gaudi::Property<std::vector<bool>> m_invertFlags {this, "invertFlags", {}, "toggles for inverting the selection (index-parallel to selectionFlags)"};
    
    /// \brief a vector of accessors to read the flags
    std::vector<std::unique_ptr<ISelectionReadAccessor>> m_accessors;

    /// \brief the filter reporter params
    FilterReporterParams m_filterParams {this, "EventFlagSelection", "event flag selection"};
  };
}

#endif
