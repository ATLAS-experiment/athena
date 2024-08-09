/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nello Bruscino

#ifndef ASG_FAKEBKGCALCULATORALG_H
#define ASG_FAKEBKGCALCULATORALG_H

// Algorithm includes
#include <AnaAlgorithm/AnaAlgorithm.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SelectionHelpers/SysReadSelectionHandle.h>
#include <AsgTools/PropertyWrapper.h>

// Framework includes
#include <xAODEgamma/ElectronContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODEventInfo/EventInfo.h>

#include "AsgAnalysisInterfaces/ILinearFakeBkgTool.h"


namespace CP {

  class FakeBkgCalculatorAlg final : public EL::AnaAlgorithm {

  public:
    using EL::AnaAlgorithm::AnaAlgorithm;
    virtual StatusCode initialize() override;
    virtual StatusCode execute() override;

  private:
    CP::SysListHandle m_systematicsList {this};

    // configurable properties
    Gaudi::Property<std::string> m_definition {this, "Definition", "", "lepton definition used by FakeBkgTools from IFF"};
    Gaudi::Property<std::string> m_process {this, "Process", "", "target process used by FakeBkgTools from IFF"};

    // inputs needed for calculation
    CP::SysReadHandle<xAOD::ElectronContainer> m_electronsHandle {
      this, "electrons", "", "the electron container to use"
    };
    CP::SysReadSelectionHandle m_electronSelection {
      this, "electronSelection", "", "the selection on the input electrons"
    };
    CP::SysReadSelectionHandle m_electronSelectionTarget {
      this, "electronSelectionTarget", "", "the tight selection on the input electrons"
    };

    CP::SysReadHandle<xAOD::MuonContainer> m_muonsHandle {
      this, "muons", "", "the muon container to use"
    };
    CP::SysReadSelectionHandle m_muonSelection {
      this, "muonSelection", "", "the selection on the input muons"
    };
    CP::SysReadSelectionHandle m_muonSelectionTarget {
      this, "muonSelectionTarget", "", "the tight selection on the input muons"
    };

    CP::SysReadHandle<xAOD::EventInfo> m_eventInfoHandle {
      this, "eventInfo", "EventInfo", "the EventInfo container to read selection deciosions from"
    };

    CP::SysReadSelectionHandle m_preselection {
      this, "eventPreselection", "", "event preselection to check before running this algorithm"
    };

    ToolHandle<ILinearFakeBkgTool> m_fakeTool {
      this, "FakeTool", "", "the tool for fake lepton estimate among the different [FakeBkgTools](https://gitlab.cern.ch/atlas/athena/-/tree/main/PhysicsAnalysis/AnalysisCommon/FakeBkgTools). The currently available one is `CP::AsymptMatrixTool`"
    };

    // output container
    CP::SysWriteDecorHandle<float> m_fakeToolOutput {
      this, "FakeToolOutput", "FakeToolOutput_%SYS%", "decoration name for the Fake Tool output"
    };

  };

} // namespace

#endif
