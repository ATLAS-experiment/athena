/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Binbin Dong

#ifndef EVENT_SELECTOR_DILEPTONOSSFINVARIANTMASSWINDOWSELECTORALG_H
#define EVENT_SELECTOR_DILEPTONOSSFINVARIANTMASSWINDOWSELECTORALG_H

// Algorithm includes
#include <AnaAlgorithm/AnaAlgorithm.h>
#include <AsgTools/PropertyWrapper.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SelectionHelpers/SysReadSelectionHandle.h>
#include <SelectionHelpers/SysWriteSelectionHandle.h>

// Framework includes
#include <xAODEgamma/ElectronContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODTruth/TruthParticleContainer.h>
#include <xAODEventInfo/EventInfo.h>

namespace CP {
  /// \brief an algorithm that vetos an event with two OS electrons or muons mass in a specific mass window

  class DileptonOSSFInvariantMassWindowSelectorAlg final : public EL::AnaAlgorithm {

    /// \brief the standard constructor
    public:
      DileptonOSSFInvariantMassWindowSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator);
      virtual StatusCode initialize() override;
      virtual StatusCode execute() override;

    private:
      Gaudi::Property<float> m_mll_lower {this, "lowMll", 0., "MLL > LOW (in MeV)"};
      Gaudi::Property<float> m_mll_upper {this, "highMll", 0., "MLL < High (in MeV)"};
      /// whether to veto events instead of selecting them
      Gaudi::Property<bool> m_veto {this, "vetoMode", false, "switch to veto-mode"};

      /// \brief use dressed kinematics
      Gaudi::Property<bool> m_useDressedProperties{this, "useDressedProperties", false,
        "whether to use dressed electron and muon kinematics rather than simple "
        "P4 kinematics"};

      CP::SysListHandle m_systematicsList {this};
      CP::SysReadHandle<xAOD::ElectronContainer> m_electronsHandle {
        this, "electrons", "", "the electron container to use"
      };
      CP::SysReadSelectionHandle m_electronSelection {
        this, "electronSelection", "", "the selection on the input electrons"
      };
      CP::SysReadHandle<xAOD::MuonContainer> m_muonsHandle {
        this, "muons", "", "the muon container to use"
      };
      CP::SysReadSelectionHandle m_muonSelection {
        this, "muonSelection", "", "the selection on the input muons"
      };
      CP::SysReadHandle<xAOD::TruthParticleContainer> m_electronsTruthHandle {
        this, "truthElectrons", "", "the truth electron container to use"
      };
      CP::SysReadSelectionHandle m_electronTruthSelection {
        this, "truthElectronSelection", "", "the selection on the input truth electrons"
      };
      CP::SysReadHandle<xAOD::TruthParticleContainer> m_muonsTruthHandle {
        this, "truthMuons", "", "the truth muon container to use"
      };
      CP::SysReadSelectionHandle m_muonTruthSelection {
        this, "truthMuonSelection", "", "the selection on the input muons"
      };
      CP::SysReadHandle<xAOD::EventInfo> m_eventInfoHandle {
        this, "eventInfo", "EventInfo", "the EventInfo container to read selection decisions from"
      };
      CP::SysReadSelectionHandle m_preselection {
        this, "eventPreselection", "SetMe", "name of the preselection to check before applying this one"
      };
      CP::SysWriteSelectionHandle m_decoration {
        this, "decorationName", "SetMe", "decoration name for the MLL selector"
      };

    }; // class
} // namespace CP

#endif // EVENT_SELECTOR_DILEPTONOSSFINVARIANTMASSWINDOWSELECTORALG_H
