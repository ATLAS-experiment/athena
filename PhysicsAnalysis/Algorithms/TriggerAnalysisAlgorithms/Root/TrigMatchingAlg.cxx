/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Marco Rimoldi

#include <TrigCompositeUtils/ChainNameParser.h>
#include <TriggerAnalysisAlgorithms/TrigMatchingAlg.h>
#include <TriggerAnalysisAlgorithms/TrigChainNameHelpers.h>


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
    const std::string prefix = m_matchingDecoration + "_";
    for (const std::string &chain : m_trigSingleMatchingList)
    {
      // A string-based signature-identifier per leg, may contain duplicated return values for asymmetric chains.
      const std::vector<std::string> signatures = ChainNameParser::signatures(chain);
      if (signatures.size() != 1)
      {
        ATH_MSG_ERROR("The decoration-based TrigMatchingAlg only supports single-legged triggers. " << chain << " has " << signatures.size() << " legs.");
        return StatusCode::FAILURE;
      }
      const float dR = (signatures.front() == "tau" ? 0.2 : 0.1);
      m_matchingChains.push_back({chain, dR, SG::Decorator<char>(prefix + sanitizeTriggerChainName(chain))});
    }
    for (const std::string &chain : m_trigSingleMatchingListDummy)
    {
      m_dummyChains.push_back({chain, SG::Decorator<char>(prefix + sanitizeTriggerChainName(chain))});
    }

    ANA_CHECK (m_particlesHandle.initialize (m_systematicsList));
    ANA_CHECK (m_systematicsList.initialize());

    return StatusCode::SUCCESS;
  }



  StatusCode TrigMatchingAlg::execute(const EventContext& ctx)
  {
    for (const auto & syst : m_systematicsList.systematicsVector())
    {
      const xAOD::IParticleContainer* particles(nullptr);

      ANA_CHECK(m_particlesHandle.retrieve(particles, syst, ctx));

      ATH_MSG_DEBUG("Retrieving " << m_particlesHandle.getName(syst));

      for (const xAOD::IParticle *particle : *particles)
      {
        ATH_MSG_DEBUG("-- Considering offline eta:" << particle->eta() << " phi:" << particle->phi() << " (pT:" << particle->pt() << ")");
        for (const MatchingChain &mc : m_matchingChains)
        {
          const bool match = m_trigMatchingTool->match(*particle, mc.chain, mc.dR, false);
          mc.decorator(*particle) = match;
          ATH_MSG_DEBUG("-- -- Considering for " << mc.chain << ", match = " << match);
        }

        for (const DummyChain &dc : m_dummyChains)
        {
          ATH_MSG_DEBUG("Applying dummy match=0 decoration for " << dc.chain);
          dc.decorator(*particle) = 0;
        }
      }
    }
    return StatusCode::SUCCESS;
  }
} // namespace CP
