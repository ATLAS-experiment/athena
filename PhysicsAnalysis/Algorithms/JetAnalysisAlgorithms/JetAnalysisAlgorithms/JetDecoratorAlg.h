/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Teng Jian Khoo


#ifndef JET_ANALYSIS_ALGORITHMS__JET_DECORATOR_ALG_H
#define JET_ANALYSIS_ALGORITHMS__JET_DECORATOR_ALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <JetInterface/IJetDecorator.h>
#include <SystematicsHandles/SysCopyHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <xAODJet/JetContainer.h>

namespace CP
{
  /// @brief an algorithm for calling @ref IJetDecorator

  class JetDecoratorAlg final : public EL::AnaAlgorithm
  {
    /// @brief the standard constructor
  public:
    using EL::AnaAlgorithm::AnaAlgorithm;


  public:
    StatusCode initialize () override;

  public:
    StatusCode execute (const EventContext& ctx) override;

    /// @brief the update tool
  private:
    ToolHandle<IJetDecorator> m_decorator{this, "decorator", "", "the decorator tool we apply to the jet collection"};

    /// @brief the systematics list we run
  private:
    SysListHandle m_systematicsList {this};

    /// @brief the jet collection we run on
  private:
    SysCopyHandle<const xAOD::JetContainer> m_jetHandle {
      this, "jets", "", "the jet collection to run on"};

  };
}

#endif
