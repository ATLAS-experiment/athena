/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina

#ifndef EVENT_SELECTOR_SAVEFILTERALG_H
#define EVENT_SELECTOR_SAVEFILTERALG_H

// Algorithm includes
#include <AnaAlgorithm/AnaAlgorithm.h>
#include <AsgTools/PropertyWrapper.h>
#include <SystematicsHandles/SysFilterReporterParams.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SelectionHelpers/SysReadSelectionHandle.h>
#include <SelectionHelpers/SysWriteSelectionHandle.h>

// Framework includes
#include <xAODEventInfo/EventInfo.h>

namespace CP {

  /// \brief an algorithm to turn event selections into filters for cutflow saving

  class SaveFilterAlg final : public EL::AnaAlgorithm {

    /// \brief the standard constructor
    public:
      SaveFilterAlg(const std::string &name, ISvcLocator *pSvcLocator);
      virtual StatusCode initialize() override;
      virtual StatusCode execute() override;
      virtual StatusCode finalize() override;

    private:

      /// \brief the systematics
      CP::SysListHandle m_systematicsList {this};

      /// \brief the input selections
      CP::SysReadSelectionHandle m_inputselection {
        this, "selection", "SetMe", "names of the selections to check"
      };

      /// \brief the event info handle
      CP::SysReadHandle<xAOD::EventInfo> m_eventInfoHandle {
        this, "eventInfoContainer", "EventInfo", "the EventInfo container to read selection decisions from"
      };

      /// \brief the save filter
      CP::SysFilterReporterParams m_filterParams {
        this, "event selector SAVE filter"
      };

      /// \brief whether to not apply an event filter
      Gaudi::Property<bool> m_noFilter {this, "noFilter", false, "whether to disable the event filter"};

      /// \brief the output selection decoration
      CP::SysWriteSelectionHandle m_outputselection {
	this, "selectionName", "SetMe", "name of the output selection decision"
      };

      /// \brief the output selection decoration for the event filter
      CP::SysWriteDecorHandle<char> m_decoration {
	this, "decorationName", "SetMe", "additional decoration name for the event filter (stores the decision even when the filter is off)"
      };

  }; // class
} // namespace CP

#endif // EVENT_SELECTOR_SAVEFILTERALG_H
