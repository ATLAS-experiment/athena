/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include <AsgAnalysisAlgorithms/AsgObjectScaleFactorAlg.h>

namespace CP
{

  StatusCode AsgObjectScaleFactorAlg ::
  initialize ()
  {

    ANA_CHECK (m_particlesHandle.initialize (m_systematicsList));

    for(const auto& sf : m_inScaleFactors)
      m_inSFHandles.emplace_back(sf, this);
    for(auto& handle : m_inSFHandles)
      ANA_CHECK(handle.initialize(m_systematicsList, m_particlesHandle));

    ANA_CHECK(m_outSFHandle.initialize(m_systematicsList, m_particlesHandle));

    ANA_CHECK (m_systematicsList.initialize());
    
    return StatusCode::SUCCESS;
  }

  StatusCode AsgObjectScaleFactorAlg ::
  execute ()
  {

    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      const xAOD::IParticleContainer *particles = nullptr;
      ANA_CHECK (m_particlesHandle.retrieve (particles, sys));
      for (const xAOD::IParticle *particle : *particles)
      {
	float scaleFactor = 1.;
	for(const auto& sf : m_inSFHandles)
	  scaleFactor *= sf.get(*particle, sys);
	
	m_outSFHandle.set(*particle, scaleFactor, sys);
      }
    }

    return StatusCode::SUCCESS;
  }

} // namespace
