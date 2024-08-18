/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Marco Rimoldi

#include <TriggerAnalysisAlgorithms/TrigMatchingAlg.h>
#include <xAODEventInfo/EventInfo.h>
#include <RootCoreUtils/StringUtil.h>
#include "PATCore/PATCoreEnums.h"

namespace CP
{

  TrigMatchingAlg::TrigMatchingAlg(const std::string& name, 
                                         ISvcLocator *svcLoc)
      : EL::AnaAlgorithm(name, svcLoc)
  {
    declareProperty("matchingTool", m_trigMatchingTool, "trigger matching tool");
  }

  StatusCode TrigMatchingAlg::initialize()
  {
    if (m_matchingDecoration.empty())
    {
      ATH_MSG_ERROR("The decoration name needs to be defined");
      return StatusCode::FAILURE;
    }

    if (m_trigSingleMatchingList.empty())
    {
      ATH_MSG_ERROR("At least one trigger needs to be provided in the list");
      return StatusCode::FAILURE;
    }

    // retrieve the trigger matching tool
    ANA_CHECK(m_trigMatchingTool.retrieve());

    for (const std::string &chain : m_trigSingleMatchingList)
    {
      m_matchingDecorators.emplace(chain, m_matchingDecoration + "_" + RCU::substitute (chain, "-", "_"));
    }
    ANA_CHECK(m_particleSelection.initialize(m_systematicsList, m_particlesHandle, SG::AllowEmpty));

    if (m_particlesHandle) ANA_CHECK (m_particlesHandle.initialize (m_systematicsList));

    ANA_CHECK (m_systematicsList.initialize());

    return StatusCode::SUCCESS;
  }



  StatusCode TrigMatchingAlg::execute()
  {

  for (const auto & syst : m_systematicsList.systematicsVector())
    {
    const xAOD::IParticleContainer* particles(nullptr);

      if (m_particlesHandle)  ANA_CHECK(m_particlesHandle.retrieve(particles, syst));

      if (particles != nullptr)
      {
        for (const xAOD::IParticle *particle : *particles)
        {
          for (const std::string &chain : m_trigSingleMatchingList)
          {
            float dR = chain.starts_with("HLT_tau")? 0.2:0.1;
            (m_matchingDecorators.at(chain))(*particle) = m_trigMatchingTool->match(*particle, chain, dR, false);
          }
        }
      }
    }
    return StatusCode::SUCCESS;
  }
} // namespace CP

