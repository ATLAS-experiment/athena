/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Marco Rimoldi

#ifndef TRIGGER_ANALYSIS_ALGORITHMS__TRIG_MATCHING_ALG_H
#define TRIGGER_ANALYSIS_ALGORITHMS__TRIG_MATCHING_ALG_H

// Algorithm includes
#include <AnaAlgorithm/AnaAlgorithm.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <AsgTools/PropertyWrapper.h>

// Framework includes
#include <AthContainers/Decorator.h>
#include <xAODBase/IParticleContainer.h>
#include <AsgTools/ToolHandle.h>

// Trigger Include
#include <TriggerMatchingTool/IMatchingTool.h>

#include <string>
#include <vector>

namespace CP
{
  /// \brief an algorithm to provide and decorate trigger matching for leptons
  /// Currently only single leg triggers are supported.

  class TrigMatchingAlg final : public EL::AnaAlgorithm
  {
    /// \brief the standard constructor
  public:
    TrigMatchingAlg(const std::string& name, ISvcLocator* pSvcLocator);

  public:
    virtual StatusCode initialize() final override;
    virtual StatusCode execute(const EventContext& ctx) final override;

    /// \brief trigger decision tool handle
  private:
    ToolHandle<Trig::IMatchingTool> m_trigMatchingTool;

    /// \brief the systematics list we run
    SysListHandle m_systematicsList {this};

    /// \brief the decoration for trigger matching
    Gaudi::Property<std::string> m_matchingDecoration {this, "matchingDecoration", {}, "The decoration for trigger matching"};

    /// \brief list of triggers
    Gaudi::Property<std::vector<std::string>> m_trigSingleMatchingList {this, "trigSingleMatchingList", {}, "List of triggers for Matching"};
  
    /// \brief list of triggers for dummy matching decorations
    Gaudi::Property<std::vector<std::string>> m_trigSingleMatchingListDummy {this, "trigSingleMatchingListDummy", {}, "List of triggers for dummy matching decorations"};

    /// \brief input particle collection
    SysReadHandle<xAOD::IParticleContainer> m_particlesHandle { this, "particles", "", "the particle container to use"};

    /// \brief per-chain matching configuration, built in initialize()
    struct MatchingChain
    {
      std::string chain;
      float dR{};
      SG::Decorator<char> decorator;
    };
    std::vector<MatchingChain> m_matchingChains;

    /// \brief per-chain dummy matching decorations, built in initialize()
    struct DummyChain
    {
      std::string chain;
      SG::Decorator<char> decorator;
    };
    std::vector<DummyChain> m_dummyChains;

  };

} // namespace CP

#endif /*  TRIGGER_ANALYSIS_ALGORITHMS__TRIG_MATCHING_ALG_H */
