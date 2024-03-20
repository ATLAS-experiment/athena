/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Tadej Novak


#ifndef MUON_ANALYSIS_ALGORITHMS__MUON_TRIGGER_EFFICIENCY_SCALE_FACTOR_ALG_H
#define MUON_ANALYSIS_ALGORITHMS__MUON_TRIGGER_EFFICIENCY_SCALE_FACTOR_ALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <MuonAnalysisInterfaces/IMuonTriggerScaleFactors.h>
#include <SelectionHelpers/OutOfValidityHelper.h>
#include <SelectionHelpers/SysReadSelectionHandle.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <xAODEventInfo/EventInfo.h>
#include <xAODMuon/MuonContainer.h>
#include <AsgTools/PropertyWrapper.h>

namespace CP
{
  /// \brief an algorithm for calling \ref IMuonTriggerScaleFactors

  class MuonTriggerEfficiencyScaleFactorAlg final : public EL::AnaAlgorithm
  {
    /// \brief the standard constructor
  public:
    using EL::AnaAlgorithm::AnaAlgorithm;
    StatusCode initialize () override;
    StatusCode execute () override;
    


    /// \brief the smearing tool
  private:
    ToolHandle<IMuonTriggerScaleFactors> m_efficiencyScaleFactorTool;

    /// \brief the systematics list we run
  private:
    SysListHandle m_systematicsList {this};

    /// \brief the muon collection we run on
  private:
    SysReadHandle<xAOD::MuonContainer> m_muonHandle {
      this, "muons", "Muons", "the muon collection to run on"};

    /// \brief the EventInfo collection we use
  private:
    SysReadHandle<xAOD::EventInfo> m_eventInfoHandle {
      this, "eventInfo", "EventInfo", "the EventInfo we use"};

    /// \brief the preselection we apply to our input
  private:
    SysReadSelectionHandle m_preselection {
      this, "preselection", "", "the preselection to apply"};

    /// \brief the helper for OutOfValidity results
  private:
    OutOfValidityHelper m_outOfValidity {this};

    /// \brief trigger to run efficiency for
  private:
    Gaudi::Property<std::string> m_trigger {this, "trigger", "", "trigger or trigger leg to calculate efficiency for"};
    
    /// \brief minimum run number this trigger is valid for
  private:
    Gaudi::Property<uint32_t> m_minRunNumber {this, "minRunNumber", 0, "minimum run number for the trigger or trigger leg to calculate efficiency for"};

    /// \brief maximum run number this trigger is valid for
  private:
    Gaudi::Property<uint32_t> m_maxRunNumber {this, "maxRunNumber", 999999, "maximum run number for the trigger or trigger leg to calculate efficiency for"};

    /// \brief the decoration for the muon scale factor
  private:
    SysWriteDecorHandle<float> m_scaleFactorDecoration {
      this, "scaleFactorDecoration", "", "the decoration for the muon efficiency scale factor"};

    /// \brief the decoration for the muon mc efficiency
  private:
    SysWriteDecorHandle<float> m_mcEfficiencyDecoration {
      this, "mcEfficiencyDecoration", "", "the decoration for the muon MC efficiency"};

    /// \brief the decoration for the muon data efficiency
  private:
    SysWriteDecorHandle<float> m_dataEfficiencyDecoration {
      this, "dataEfficiencyDecoration", "", "the decoration for the muon data efficiency"};
  };
}

#endif
