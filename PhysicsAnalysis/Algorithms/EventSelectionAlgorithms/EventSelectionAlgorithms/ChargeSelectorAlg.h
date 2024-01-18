/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina

#ifndef EVENT_SELECTOR_CHARGESELECTORALG_H
#define EVENT_SELECTOR_CHARGESELECTORALG_H

// Algorithm includes
#include <AnaAlgorithm/AnaAlgorithm.h>
#include <AsgTools/PropertyWrapper.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SelectionHelpers/SysReadSelectionHandle.h>
#include <SelectionHelpers/SysWriteSelectionHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>

// Framework includes
#include <xAODEgamma/ElectronContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODEventInfo/EventInfo.h>

namespace CP {

  /// \brief an algorithm that selects an event if the event has leptons
  /// with opposite sign or same sign 

  class ChargeSelectorAlg final : public EL::AnaAlgorithm {
    
    public:
     /// \brief the standard constructor
      ChargeSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator);
      virtual StatusCode initialize() override;
      virtual StatusCode execute() override;

    private:
    
      /// \brief whether or not to select 2 opposite-sign leptons
      Gaudi::Property<bool> m_OSmode {this, "OS", true, "whether to request 2 opposite-sign leptons"};

      /// \brief the systematics list
      CP::SysListHandle m_systematicsList {this};

      /// \brief the electron input handle
      CP::SysReadHandle<xAOD::ElectronContainer> m_electronsHandle {
        this, "electrons", "", "the electron container to use"
      };

      /// \brief the electron selection handle
      CP::SysReadSelectionHandle m_electronSelection {
        this, "electronSelection", "", "the selection on the input electrons"
      };

      /// \brief the muon input handle
      CP::SysReadHandle<xAOD::MuonContainer> m_muonsHandle {
        this, "muons", "", "the muon container to use"
      };

      /// \brief the muon selection handle
      CP::SysReadSelectionHandle m_muonSelection {
        this, "muonSelection", "", "the selection on the input muons"
      };

      /// \brief the event info handle
      CP::SysReadHandle<xAOD::EventInfo> m_eventInfoHandle {
        this, "eventInfo", "EventInfo", "the EventInfo container to read selection decisions from"
      };

      /// \brief the preselection
      CP::SysReadSelectionHandle m_preselection {
        this, "eventPreselection", "SetMe", "name of the preselection to check before applying this one"
      };

      /// \brief the output decoration handle
      CP::SysWriteSelectionHandle m_decoration {
        this, "decorationName", "SetMe", "decoration name for the MLL selector"
      };

  }; // class
} // namespace CP

#endif // EVENT_SELECTOR_CHARGESELECTORALG_H
