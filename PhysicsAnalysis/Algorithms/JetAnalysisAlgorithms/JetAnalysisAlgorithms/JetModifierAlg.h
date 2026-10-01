/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef JET_ANALYSIS_ALGORITHMS__JET_MODIFIER_ALG_H
#define JET_ANALYSIS_ALGORITHMS__JET_MODIFIER_ALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <JetInterface/IJetModifier.h>
#include <SelectionHelpers/OutOfValidityHelper.h>
#include <SystematicsHandles/SysCopyHandle.h>
#include <SystematicsHandles/SysListHandle.h>

namespace CP
{
  /// @brief an algorithm for calling @ref IJetModifier

  class JetModifierAlg final : public EL::AnaAlgorithm
  {
    /// @brief the standard constructor
  public:
    using EL::AnaAlgorithm::AnaAlgorithm;
    StatusCode initialize () override;
    StatusCode execute (const EventContext& ctx) override;



    /// @brief the modifier tool
  private:
    ToolHandle<IJetModifier> m_modifierTool {this, "modifierTool", "JetForwardJvtTool", "the modifier tool we apply"};

    /// @brief the systematics list we run
  private:
    SysListHandle m_systematicsList {this};

    /// @brief the jet collection we run on
  private:
    SysCopyHandle<xAOD::JetContainer> m_jetHandle {
      this, "jets", "", "the jet collection to run on"};

    /// @brief the helper for OutOfValidity results
  private:
    OutOfValidityHelper m_outOfValidity {this};
  };
}

#endif
