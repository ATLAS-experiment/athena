/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Marco Rimoldi

#ifndef TRIGGER_ANALYSIS_ALGORITHMS__TRIG_MATCHING_ALG_H
#define TRIGGER_ANALYSIS_ALGORITHMS__TRIG_MATCHING_ALG_H

// Algorithm includes
#include <AnaAlgorithm/AnaAlgorithm.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysWriteHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SelectionHelpers/SysReadSelectionHandle.h>
#include <SystematicsHandles/SysFilterReporterParams.h>
#include <AsgTools/PropertyWrapper.h>

// Framework includes
#include <AsgMessaging/AsgMessaging.h>
#include <xAODBase/IParticleContainer.h>
#include <xAODEventInfo/EventInfo.h>
#include <AsgTools/AsgTool.h>
#include <AsgTools/ToolHandle.h>
#include <AsgTools/AnaToolHandle.h>

// Trigger Include
#include <TriggerMatchingTool/IMatchingTool.h>

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
    virtual StatusCode execute() final override;

    /// \brief trigger decision tool handle
  private:
    ToolHandle<Trig::IMatchingTool> m_trigMatchingTool;

    /// \brief the systematics list we run
    SysListHandle m_systematicsList {this};

    /// \brief the decoration for trigger matching
    Gaudi::Property<std::string> m_matchingDecoration {this, "matchingDecoration", {}, "The decoration for trigger matching"};

    /// \brief the decorators for \ref m_matchingDecoration and triggers combination
    std::unordered_map<std::string, SG::AuxElement::Decorator<char>> m_matchingDecorators;

    /// \brief list of triggers
    Gaudi::Property<std::vector<std::string>> m_trigSingleMatchingList {this, "trigSingleMatchingList", {}, "List of triggers for Matching"};
  
    /// \brief list of triggers for dummy matching decorations
    Gaudi::Property<std::vector<std::string>> m_trigSingleMatchingListDummy {this, "trigSingleMatchingListDummy", {}, "List of triggers for dummy matching decorations"};

    /// \brief input particle collection
    SysReadHandle<xAOD::IParticleContainer> m_particlesHandle { this, "particles", "", "the particle container to use"};

    /// \brief input particle selection
    SysReadSelectionHandle m_particleSelection {this, "particleSelection", "", "the selection on the input particles"};

  };

} // namespace CP

#endif /*  TRIGGER_ANALYSIS_ALGORITHMS__TRIG_MATCHING_ALG_H */

