/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Tadej Novak

#ifndef ASG_ANALYSIS_ALGORITHMS__EVENT_STATUS_SELECTION_ALG_H
#define ASG_ANALYSIS_ALGORITHMS__EVENT_STATUS_SELECTION_ALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <EventBookkeeperTools/FilterReporterParams.h>

namespace CP
{
  /// \brief an algorithm for selecting events based on their error status
  class EventStatusSelectionAlg final : public EL::AnaAlgorithm
  {
  public:
    EventStatusSelectionAlg(const std::string &name,
                            ISvcLocator *svcLoc = nullptr);

    virtual StatusCode initialize() final;
    virtual StatusCode execute(const EventContext& ctx) final;
    virtual StatusCode finalize() final;

  private:
    /// \brief the filter reporter parameters
    FilterReporterParams m_filterParams {this, "EventErrorState", "selecting events without any error state set"};
  };
}

#endif
