/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
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
#include <PATInterfaces/CorrectionCode.h>
#include <PATInterfaces/SystematicSet.h>

namespace CP
{
  /// \brief an algorithm for calling \ref IBTaggingEfficiencyTool including b-jet trigger SF

  class BTaggingTriggerEfficiencyAlg final : public EL::AnaAlgorithm
  {
    /// \brief the standard constructor
  public:
    using EL::AnaAlgorithm::AnaAlgorithm;

    StatusCode initialize () override;
    StatusCode execute (const EventContext& ctx) override;
    StatusCode finalize () override;

    /// \brief compute the b-jet trigger scale factor for one jet
    ///
    /// The returned code is the worst code of the calibration lookups that
    /// entered the value: Ok when the scale factor is valid,
    /// OutOfValidityRange when no scale factor can be given for this jet,
    /// Error on a tool failure.  Jets the trigger did not look at, and jets
    /// outside the trigger calibration, receive the offline scale factor.
  private:
    CP::CorrectionCode triggerScaleFactor (const xAOD::Jet& jet, const CP::SystematicSet& sys, float& sf);

    /// \brief give the offline scale factor to a trigger-matched b-jet that
    /// the trigger scale factor inputs do not cover, and report it
    ///
    /// The first such jet is reported with a warning; the number of such
    /// jets in the nominal pass is reported in finalize().
    CP::CorrectionCode outsideTriggerCalibration (const xAOD::Jet& jet, const CP::SystematicSet& sys, float& sf);

    /// \brief number of nominal jets that received the offline scale factor
    /// because they are outside the trigger calibration
    std::size_t m_nOutsideTriggerCalibration = 0;
    /// \brief whether the first such jet has been reported
    bool m_reportedOutsideTriggerCalibration = false;

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
