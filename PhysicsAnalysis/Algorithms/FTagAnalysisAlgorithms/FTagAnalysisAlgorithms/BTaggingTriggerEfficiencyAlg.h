/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Thomas Strebler


#ifndef F_TAG_ANALYSIS_ALGORITHMS__B_TAGGING_TRIGGER_EFFICIENCY_ALG_H
#define F_TAG_ANALYSIS_ALGORITHMS__B_TAGGING_TRIGGER_EFFICIENCY_ALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <AsgTools/PropertyWrapper.h>
#include <SelectionHelpers/OutOfValidityHelper.h>
#include <SelectionHelpers/SysReadSelectionHandle.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysListHandle.h>

#include <xAODJet/JetContainer.h>
#include <FTagAnalysisInterfaces/IBTaggingEfficiencyTool.h>

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

    /// \brief the systematics list we run
    SysListHandle m_systematicsList {this};

    /// \brief the jet collection we run on
    SysReadHandle<xAOD::JetContainer> m_jetHandle {
      this, "jets", "Jets", "the jet collection to run on"};

    SysReadDecorHandle<int> m_truthFlav{"HadronConeExclTruthLabelID", this};

    /// \brief the preselection we apply to our input
    SysReadSelectionHandle m_preselection {
      this, "preselection", "", "the preselection to apply"};

    /// \brief the helper for OutOfValidity results
    OutOfValidityHelper m_outOfValidity {this};

    /// \brief the decoration for the b-tagging scale factor
    SysWriteDecorHandle<float> m_scaleFactorDecoration {
      this, "scaleFactorDecoration", "", "the decoration for the b-tagging efficiency scale factor"};

    SysReadDecorHandle<char> m_matchingDecoration {
      this, "matchingDecoration", "", "the decoration for offline jet matched to HLT"};
    SysReadDecorHandle<char> m_bTagMatchingDecoration {
      this, "bTagMatchingDecoration", "", "the decoration for offline jet  matched to HLT b-tag"};

  };
}

#endif
