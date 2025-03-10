/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina

#ifndef JET_ANALYSIS_ALGORITHMS__JET_FFSMEARING_ALG_H
#define JET_ANALYSIS_ALGORITHMS__JET_FFSMEARING_ALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <JetCPInterfaces/ICPJetCorrectionTool.h>
#include <SelectionHelpers/OutOfValidityHelper.h>
#include <SelectionHelpers/SysReadSelectionHandle.h>
#include <SystematicsHandles/SysCopyHandle.h>
#include <SystematicsHandles/SysListHandle.h>

namespace CP {
/// \brief an algorithm for calling \ref FFJetSmearingTool

class JetFFSmearingAlg final : public EL::AnaAlgorithm {
  /// \brief the standard constructor
 public:
  using EL::AnaAlgorithm::AnaAlgorithm;
  virtual StatusCode initialize() override;
  virtual StatusCode execute() override;

  /// \brief the large-R jet FF smearing tool
 private:
  ToolHandle<ICPJetCorrectionTool> m_FFSmearingTool{
      this, "FFSmearingTool", "", "the large-R jet FF smearing tool we apply"};

  /// \brief the systematics list we run
 private:
  SysListHandle m_systematicsList{this};

  /// \brief FIXME: vector of FFJetSmearingTool systematics, while the tool is unable to handle external systematics
  private:
   std::vector<CP::SystematicSet> m_systematicsVector;

  /// \brief the large-R jet collection we run on
 private:
  SysCopyHandle<xAOD::JetContainer> m_jetHandle{
      this, "jets", "", "the large-R jet collection to run on"};

  /// \brief the preselection we apply to our input
 private:
  SysReadSelectionHandle m_preselection{this, "preselection", "",
                                        "the preselection to apply"};

  /// \brief the helper for OutOfValidity results
 private:
  OutOfValidityHelper m_outOfValidity{this};
};
}  // namespace CP

#endif
