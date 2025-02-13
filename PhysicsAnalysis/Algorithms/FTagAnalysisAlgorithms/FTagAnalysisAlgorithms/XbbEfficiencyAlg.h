/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Binbin Dong

#ifndef F_TAG_ANALYSIS_ALGORITHMS__Xbb_EFFICIENCY_ALG_H
#define F_TAG_ANALYSIS_ALGORITHMS__Xbb_EFFICIENCY_ALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <FTagAnalysisInterfaces/IBTaggingEfficiencyJsonTool.h>
#include <SelectionHelpers/SysReadSelectionHandle.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <xAODJet/JetContainer.h>
#include <AsgTools/PropertyWrapper.h>
#include <memory>

namespace CP
{
  /// \brief an algorithm for calling \ref IBTaggingEfficiencyJsonTool
  class XbbEfficiencyAlg final : public EL::AnaAlgorithm
  {
  /// \brief the standard constructor
    public:
    using EL::AnaAlgorithm::AnaAlgorithm;
    virtual StatusCode initialize () override;
    virtual StatusCode execute () override;

    /// \brief the efficiency json tool
    private:
    ToolHandle<IBTaggingEfficiencyJsonTool> m_efficiencyTool {this, "efficiencyTool", "BTaggingEfficiencyJsonTool", "the calibration tool we apply"};
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

    /// \brief the decoration for the Xbb SF
    private:
    SysWriteDecorHandle<float> m_scaleFactorDecoration {
      this, "scaleFactorDecoration", "", "the decoration for the Xbb SF"};
  };
}
#endif
