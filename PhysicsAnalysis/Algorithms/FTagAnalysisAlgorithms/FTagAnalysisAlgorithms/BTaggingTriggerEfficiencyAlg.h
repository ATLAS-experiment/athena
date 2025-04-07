/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Thomas Strebler


#ifndef F_TAG_ANALYSIS_ALGORITHMS__B_TAGGING_TRIGGER_EFFICIENCY_ALG_H
#define F_TAG_ANALYSIS_ALGORITHMS__B_TAGGING_TRIGGER_EFFICIENCY_ALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <FTagAnalysisInterfaces/IBTaggingEfficiencyTool.h>
#include <SelectionHelpers/OutOfValidityHelper.h>
#include <SelectionHelpers/SysReadSelectionHandle.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <xAODJet/JetContainer.h>
#include <TrigDecisionTool/TrigDecisionTool.h>
#include <AsgTools/PropertyWrapper.h>
#include <memory>

namespace CP
{
  /// \brief an algorithm for calling \ref IBTaggingEfficiencyTool including b-jet trigger SF

  class BTaggingTriggerEfficiencyAlg final : public EL::AnaAlgorithm
  {
    /// \brief the standard constructor
  public:
    using EL::AnaAlgorithm::AnaAlgorithm;

    BTaggingTriggerEfficiencyAlg(const std::string &name,
				 ISvcLocator *svcLoc = nullptr);
    StatusCode initialize () override;
    StatusCode execute () override;

    /// \brief the smearing tool
  private:
    ToolHandle<IBTaggingEfficiencyTool> m_offlineEfficiencyTool
      {this, "offlineEfficiencyTool", "", "the efficiency tool we use for offline"};
    ToolHandle<IBTaggingEfficiencyTool> m_triggerEfficiencyTool
      {this, "triggerEfficiencyTool", "", "the efficiency tool we use for trigger"};
    ToolHandle<IBTaggingEfficiencyTool> m_conditionalEfficiencyTool
      {this, "conditionalEfficiencyTool", "",
	  "the efficiency tool we apply for conditional probabilities p(off | trig)"};

    ToolHandle<Trig::TrigDecisionTool> m_trigDecTool;
    Gaudi::Property<std::string> m_trigger {this, "trigger", "",
	"the trigger path to consider for the SF computation"};
    Gaudi::Property<bool> m_useRun3TriggerEDM {this, "useRun3TriggerEDM", true,
	"is the Run-3 trigger EDM available" };
    Gaudi::Property<float> m_btagThreshold {this, "btagThreshold", -1.,
	"b-tag trigger cut, only used with Run 2 trigger EDM"};

    /// \brief the systematics list we run
  private:
    SysListHandle m_systematicsList {this};

    /// \brief the jet collection we run on
  private:
    SysReadHandle<xAOD::JetContainer> m_jetHandle {
      this, "jets", "Jets", "the jet collection to run on"};

    /// \brief the preselection we apply to our input
  private:
    SysReadSelectionHandle m_preselection {
      this, "preselection", "", "the preselection to apply"};

    /// \brief the helper for OutOfValidity results
  private:
    OutOfValidityHelper m_outOfValidity {this};

    /// \brief the decoration for the b-tagging scale factor
  private:
    SysWriteDecorHandle<float> m_scaleFactorDecoration {
      this, "scaleFactorDecoration", "", "the decoration for the b-tagging efficiency scale factor"};

    /// \brief the decoration for the b-tagging selection
  private:
    SysReadSelectionHandle m_selectionHandle {
      this, "selectionDecoration", "", "the decoration for the asg selection"};

  private:
    StatusCode passTriggerBtag(const xAOD::Jet* jet, bool& pass) const;

    bool isSameJet(const xAOD::IParticle *jet1, const xAOD::IParticle *jet2) const;

    StatusCode getBtagScore(const xAOD::IParticle* jet, double& hlt_bscore) const;

  };
}

#endif
