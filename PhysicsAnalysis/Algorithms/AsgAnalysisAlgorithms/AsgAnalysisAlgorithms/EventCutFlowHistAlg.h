/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina


#ifndef ASG_ANALYSIS_ALGORITHMS__EVENT_CUT_FLOW_HIST_ALG_H
#define ASG_ANALYSIS_ALGORITHMS__EVENT_CUT_FLOW_HIST_ALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <AsgTools/PropertyWrapper.h>
#include <SelectionHelpers/ISelectionNameSvc.h>
#include <SelectionHelpers/SysReadSelectionHandle.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <xAODEventInfo/EventInfo.h>
#include <AsgTools/PropertyWrapper.h>

namespace CP
{
  /// \brief an algorithm for dumping the an event-level cutflow

  class EventCutFlowHistAlg final : public EL::AnaAlgorithm
  {
    /// \brief the standard constructor
  public:
    using EL::AnaAlgorithm::AnaAlgorithm;
    StatusCode initialize () override;
    StatusCode execute () override;


    /// \brief the systematics list we run
  private:
    SysListHandle m_systematicsList {this};

    /// \brief the jet collection we run on
  private:
    SysReadHandle<xAOD::EventInfo> m_eventInfoHandle {
      this, "eventInfo", "EventInfo", "the EventInfo container to run on"};

    /// \brief the preselection we apply to our input
  private:
    SysReadSelectionHandle m_preselection {
      this, "preselection", "", "the preselection to apply"};

    /// \brief the pattern for histogram names
  private:
    Gaudi::Property<std::string> m_histPattern {this, "histPattern", "cutflow_%SYS%", "the pattern for histogram names"};

    /// \brief the selection name service
  private:
    ServiceHandle<ISelectionNameSvc> m_selectionNameSvc {"SelectionNameSvc", "EventCutFlowHistAlg"};

    /// \brief the histogram title to use
  private:
    Gaudi::Property<std::string> m_histTitle {this, "histTitle", "event cut flow", "title for the created histograms"};

    /// \brief the input object selections for which to create a cutflow
  private:
    SysReadSelectionHandleArray m_selections {
      this, "selections", {}, "the inputs to the event cutflow"};

    /// \brief the total number of cuts configured (needed to
    /// configure histograms)
  private:
    unsigned m_allCutsNum = 0;

    /// \brief the created histograms
  private:
    std::unordered_map<CP::SystematicSet,TH1*> m_hist;

    /// \brief histogram bin labels
  private:
    std::vector<std::string> m_labels;
  };
}

#endif
