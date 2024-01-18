/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina

#ifndef EVENT_SELECTOR_DILEPTONINVARIANTMASSWINDOWSELECTORALG_H
#define EVENT_SELECTOR_DILEPTONINVARIANTMASSWINDOWSELECTORALG_H

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

  /// \brief an algorithm to select an event with 2-lepton mass compared to a
  /// specified window of values "lowMLL" and "highMLL". Use "veto" to change
  /// the behaviour and instead veto these events.

  class DileptonInvariantMassWindowSelectorAlg final : public EL::AnaAlgorithm {

    /// \brief the standard constructor
    public:
      DileptonInvariantMassWindowSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator);
      virtual StatusCode initialize() override;
      virtual StatusCode execute() override;

    private:
    
      /// \brief the upper limit of the MLL window
      Gaudi::Property<float> m_mllupper {this, "highMLL", 0., "MLL < HIGH (in MeV)"};

      /// \brief the lower limit of the MLL window
      Gaudi::Property<float> m_mlllower {this, "lowMLL", 0., "MLL > LOW (in MeV)"};

      /// \brief whether to veto events instead of selecting them
      Gaudi::Property<bool> m_veto {this, "vetoMode", false, "switch to veto-mode"};

      /// \brief the systematics
      CP::SysListHandle m_systematicsList {this};

      /// \brief the electrons handle
      CP::SysReadHandle<xAOD::ElectronContainer> m_electronsHandle {
        this, "electrons", "", "the electron container to use"
      };

      /// \brief the electrons selection
      CP::SysReadSelectionHandle m_electronSelection {
        this, "electronSelection", "", "the selection on the input electrons"
      };

      /// \brief the muons handle
      CP::SysReadHandle<xAOD::MuonContainer> m_muonsHandle {
        this, "muons", "", "the muon container to use"
      };

      /// \brief the muons selection
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

      /// \brief the output selection decoration
      CP::SysWriteSelectionHandle m_decoration {
        this, "decorationName", "SetMe", "decoration name for the MLL selector"
      };

  }; // class
} // namespace CP

#endif // EVENT_SELECTOR_DILEPTONINVARIANTMASSSELECTORALG_H
