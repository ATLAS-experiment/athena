/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Marco Rimoldi

#include <TrigCompositeUtils/ChainNameParser.h>
#include <TriggerAnalysisAlgorithms/TrigMatchingAlg.h>
#include <xAODEventInfo/EventInfo.h>
#include <RootCoreUtils/StringUtil.h>


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

    if (m_trigSingleMatchingList.empty() && m_trigSingleMatchingListDummy.empty())
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
    for (const std::string &chain : m_trigSingleMatchingListDummy)
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
            // A string-based signature-identifier per leg, may contain duplicated return values for asymmetric chains.
            const std::vector<std::string> signatures = ChainNameParser::signatures(chain);
            if (signatures.size() != 1) {
              ANA_MSG_ERROR("The decoration-based TrigMatchingAlg only supports single-legged triggers." << chain << " has " << signatures.size() << " legs.");
              return StatusCode::FAILURE;
            }

            const float dR = (signatures.at(0) == "tau" ? 0.2 : 0.1);
            (m_matchingDecorators.at(chain))(*particle) = m_trigMatchingTool->match(*particle, chain, dR, false);
          }

          for (const std::string &chain : m_trigSingleMatchingListDummy)
          {
            (m_matchingDecorators.at(chain))(*particle) = 0;
          }
        }
      }
    }
    return StatusCode::SUCCESS;
  }
} // namespace CP

