/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Tadej Novak

#include <AsgAnalysisAlgorithms/EventStatusSelectionAlg.h>
#include <EventBookkeeperTools/FilterReporter.h>
#include <xAODEventInfo/EventInfo.h>

CP::EventStatusSelectionAlg::EventStatusSelectionAlg(const std::string &name,
                                                     ISvcLocator *svcLoc)
  : EL::AnaAlgorithm(name, svcLoc)
{
}

StatusCode CP::EventStatusSelectionAlg::initialize()
{
  ANA_CHECK(m_filterParams.initialize());
  return StatusCode::SUCCESS;
}

StatusCode CP::EventStatusSelectionAlg::execute(const EventContext& /*ctx*/)
{
  FilterReporter filter (m_filterParams, false);

  const xAOD::EventInfo *eventInfo = 0;
  ANA_CHECK(evtStore()->retrieve(eventInfo, "EventInfo"));

  // Reject bad events due to problems in Tile calorimeter
  if (eventInfo->errorState(xAOD::EventInfo::Tile) == xAOD::EventInfo::Error)
  {
    ATH_MSG_VERBOSE("Rejecting event due to problems in Tile calorimeter");
    filter.setPassed(false);
    return StatusCode::SUCCESS;
  }

  // Reject bad events due to problems in LAr calorimeter
  if (eventInfo->errorState(xAOD::EventInfo::LAr) == xAOD::EventInfo::Error)
  {
    ATH_MSG_VERBOSE("Rejecting event due to problems in LAr calorimeter");
    filter.setPassed(false);
    return StatusCode::SUCCESS;
  }

  // Reject bad events due to problems in SCT
  if (eventInfo->errorState(xAOD::EventInfo::SCT) == xAOD::EventInfo::Error)
  {
    ATH_MSG_VERBOSE("Rejecting event due to problems in SCT");
    filter.setPassed(false);
    return StatusCode::SUCCESS;
  }

  // Reject incomplete events
  if (eventInfo->isEventFlagBitSet(xAOD::EventInfo::Core, 18))
  {
    ATH_MSG_VERBOSE("Rejecting incomplete events");
    filter.setPassed(false);
    return StatusCode::SUCCESS;
  }

  ATH_MSG_VERBOSE("Event passed all status checks.");
  filter.setPassed(true);

  return StatusCode::SUCCESS;
}

StatusCode CP::EventStatusSelectionAlg::finalize()
{
  ANA_MSG_INFO (m_filterParams.summary());

  return StatusCode::SUCCESS;
}
