/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Miha Muskinja

#ifndef ASG_ANALYSIS_ALGORITHMS__SYS_TRUTH_WEIGHT_ALG_H
#define ASG_ANALYSIS_ALGORITHMS__SYS_TRUTH_WEIGHT_ALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <AsgDataHandles/ReadHandleKey.h>
#include <PMGAnalysisInterfaces/ISysTruthWeightTool.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <xAODEventInfo/EventInfo.h>
#include <xAODTruth/TruthParticleContainer.h>

namespace CP
{
  /// \brief an algorithm for calling \ref ISysTruthWeightTool
  class SysTruthWeightAlg final : public EL::AnaAlgorithm
  {
    /// \brief the standard constructor
  public:
    using EL::AnaAlgorithm::AnaAlgorithm;

  public:
    StatusCode initialize() override;

  public:
    StatusCode execute(const EventContext& ctx) override;

    /// \brief the truth particle container to use for the calculation
  private:
    SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthParticleContainer{
      this, "TruthParticleContainer", "TruthParticles", "the truth particle container to use for the calculation"};

    /// \brief the tool
  private:
    ToolHandle<PMGTools::ISysTruthWeightTool> m_sysTruthWeightTool{
      this, "sysTruthWeightTool", "PMGTools::PMGHFProductionFractionTool", "the systematic truth weight tool"};

    /// \brief the systematics list we run
  private:
    SysListHandle m_systematicsList{this};

    /// \brief the event collection we run on
  private:
    SysReadHandle<xAOD::EventInfo> m_eventInfoHandle{
      this, "eventInfo", "EventInfo", "the event info object to run on"};

    /// \brief the decoration for the truth weights
  private:
    SysWriteDecorHandle<float> m_decoration{
      this, "decoration", "", "the decoration for the truth weights"};
  };
}  // namespace CP

#endif
