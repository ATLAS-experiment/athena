/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef JET_ANALYSIS_ALGORITHMS__JET_UNCERTAINTIES_ALG_H
#define JET_ANALYSIS_ALGORITHMS__JET_UNCERTAINTIES_ALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <JetCPInterfaces/ICPJetUncertaintiesTool.h>
#include <SelectionHelpers/OutOfValidityHelper.h>
#include <SelectionHelpers/SysReadSelectionHandle.h>
#include <SystematicsHandles/SysCopyHandle.h>
#include <SystematicsHandles/SysListHandle.h>

#include <optional>

namespace CP
{
  /// \brief an algorithm for calling \ref ICPJetUncertaintiesTool

  class JetUncertaintiesAlg final : public EL::AnaAlgorithm
  {
    /// \brief the standard constructor
  public:
    using EL::AnaAlgorithm::AnaAlgorithm;
    StatusCode initialize () override;
    StatusCode execute () override;



    /// \brief the main jet uncertainties tool
  private:
    ToolHandle<ICPJetUncertaintiesTool> m_uncertaintiesTool {this, "uncertaintiesTool", "JetUncertaintiesTool", "the uncertainties tool we apply"};

    /// \brief the secondary jet uncertainties tool, for pseudo-data JER smearing
  private:
    ToolHandle<ICPJetUncertaintiesTool> m_uncertaintiesToolPD {this, "uncertaintiesToolPD", "", "the uncertainties tool we apply specifically for the 'Full'/'All' JER systematic models"};

    /// \brief the systematics list we run
  private:
    SysListHandle m_systematicsList {this};

    /// \brief the jet collection we run on
  private:
    SysCopyHandle<xAOD::JetContainer> m_jetHandle {
      this, "jets", "AntiKt4EMTopoJets", "the jet collection to run on"};

    /// \brief the preselection we apply to our input
  private:
    SysReadSelectionHandle m_preselection {
      this, "preselection", "", "the preselection to apply"};

    /// \brief the helper for OutOfValidity results
  private:
    OutOfValidityHelper m_outOfValidity {this};

    /// \brief the vector of systematics (for CPU-optimisation)
  private:
    std::vector<CP::SystematicSet> m_systematicsVector;

    /// \brief the vector of pseudo-data JER systematics (for CPU-optimisation)
  private:
    std::vector<CP::SystematicSet> m_systematicsVectorOnlyJERPseudoData;

  private:
    Gaudi::Property<std::string> m_isJESbtag {this, "isJESbtagLabel", "IsBjet", "The label to apply to truth b-tagged jets, for JES flavour uncertainties"};
    std::optional<SG::Decorator<char>> m_decIsJESbtag;
    std::optional<SG::Accessor<int>> m_accTruthLabel;

  };
}

#endif
