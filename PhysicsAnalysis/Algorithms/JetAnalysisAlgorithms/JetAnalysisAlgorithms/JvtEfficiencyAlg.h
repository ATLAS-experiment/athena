/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef JET_ANALYSIS_ALGORITHMS__JVT_EFFICIENCY_ALG_H
#define JET_ANALYSIS_ALGORITHMS__JVT_EFFICIENCY_ALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <AsgTools/PropertyWrapper.h>
#include <JetAnalysisInterfaces/IJvtEfficiencyTool.h>
#include <SelectionHelpers/OutOfValidityHelper.h>
#include <SelectionHelpers/SysReadSelectionHandle.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <xAODJet/JetContainer.h>

namespace CP
{
  /// \brief an algorithm for calling \ref IJEREfficiencyTool

  class JvtEfficiencyAlg final : public EL::AnaAlgorithm
  {
    /// \brief the standard constructor
  public:
    using EL::AnaAlgorithm::AnaAlgorithm;
    StatusCode initialize () override;
    StatusCode execute () override;
    


    /// \brief the efficiency tool
  private:
    ToolHandle<CP::IJvtEfficiencyTool> m_efficiencyTool{
      this, "efficiencyTool", "", "the JVT efficiency tool to apply"};

    /// \brief the systematics list we run
  private:
    SysListHandle m_systematicsList {this};

    /// \brief the jet collection we run on
  private:
    SysReadHandle<xAOD::JetContainer> m_jetHandle {
      this, "jets", "", "the jet collection to run on"};

    /// \brief the preselection we apply to our input
  private:
    SysReadSelectionHandle m_preselection {
      this, "preselection", "", "the preselection to apply"};

    /// \brief the decoration for the JVT selection
  private:
    SysReadSelectionHandle m_selectionHandle {
      this, "selection", "", "the input decoration for the JVT selection"};

    /// \brief the decoration for the JVT scale factor
  private:
    SysWriteDecorHandle<float> m_scaleFactorDecoration {
      this, "scaleFactorDecoration", "", "the decoration for the JVT efficiency scale factor"};

    /// \brief whether to skip efficiency calculation if the selection failed
  private:
    Gaudi::Property<bool> m_skipBadEfficiency{
      this,
      "skipBadEfficiency",
      false,
      "Whether to skip calculating scale factors for objects that failed the JVT selection"};

    /// \brief the helper for OutOfValidity results
  private:
    OutOfValidityHelper m_outOfValidity {this};
  };
}

#endif
