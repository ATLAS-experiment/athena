/*
    Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina

#include <AsgAnalysisAlgorithms/BootstrapGeneratorAlg.h>

std::uint64_t CP::BootstrapGenerator::fnv1a_64(const void *buffer, size_t size, std::uint64_t offset_basis) {
  std::uint64_t h = offset_basis;
  const unsigned char *p = static_cast<const unsigned char *>(buffer);
  for (size_t i = 0; i < size; i++) {
    h ^= p[i];
    h *= m_prime;
  }
  return h;
}

std::uint64_t CP::BootstrapGenerator::generateSeed(std::uint64_t eventNumber, std::uint32_t runNumber, std::uint32_t mcChannelNumber)
{
  std::uint64_t hash = fnv1a_64(&runNumber, sizeof(runNumber), m_offset);
  hash = fnv1a_64(&eventNumber, sizeof(eventNumber), hash);
  hash = fnv1a_64(&mcChannelNumber, sizeof(mcChannelNumber), hash);
  return hash;
}

void CP::BootstrapGenerator::setSeed(std::uint64_t eventNumber, std::uint32_t runNumber, std::uint32_t mcChannelNumber)
{
  std::uint64_t seed = generateSeed(eventNumber, runNumber, mcChannelNumber);
  m_rng.SetSeed(seed);
}

StatusCode CP::BootstrapGeneratorAlg::initialize()
{
  if (m_nReplicas < 0)
    {
      ANA_MSG_ERROR("The number of bootstrapped weights (toys) cannot be negative!");
      return StatusCode::FAILURE;
    }

  ANA_CHECK(m_eventInfoHandle.initialize(m_systematicsList));
  ANA_CHECK(m_decoration.initialize(m_systematicsList, m_eventInfoHandle));
  ANA_CHECK(m_systematicsList.initialize());

  return StatusCode::SUCCESS;
}

StatusCode CP::BootstrapGeneratorAlg::execute(const EventContext& ctx)
{
  for (const auto &sys : m_systematicsList.systematicsVector())
    {
      // retrieve the EventInfo
      const xAOD::EventInfo *evtInfo = nullptr;
      ANA_CHECK(m_eventInfoHandle.retrieve(evtInfo, sys, ctx));

      // generate a unique seed from runNumber, eventNumber and DSID!
      m_bootstrap.setSeed(evtInfo->eventNumber(), evtInfo->runNumber(), m_data ? 0 : evtInfo->mcChannelNumber());

      m_weights.resize(m_nReplicas);
      // and fill it with Poisson(1)
      for (int i = 0; i < m_nReplicas; i++)
	{
	  m_weights.at(i) = m_bootstrap.getBootstrap();
	}

      // decorate weights onto EventInfo
      m_decoration.set(*evtInfo, m_weights, sys);
    }

  return StatusCode::SUCCESS;
}
