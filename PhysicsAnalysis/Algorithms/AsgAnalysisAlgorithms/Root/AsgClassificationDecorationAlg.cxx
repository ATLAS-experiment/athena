/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Tadej Novak <tadej@cern.ch>


#include <AsgAnalysisAlgorithms/AsgClassificationDecorationAlg.h>
#include <xAODBase/IParticleHelpers.h>

#include <unordered_map>

namespace CP
{

StatusCode AsgClassificationDecorationAlg::initialize()
{

  ANA_CHECK (m_particlesHandle.initialize (m_systematicsList));
  ANA_CHECK (m_classificationDecorator.initialize (m_systematicsList, m_particlesHandle));
  ANA_CHECK (m_systematicsList.initialize());

  ANA_CHECK(m_tool->initialize());

  return StatusCode::SUCCESS;
}



StatusCode AsgClassificationDecorationAlg::execute(const EventContext& ctx)
{

  std::unordered_map<const xAOD::IParticle *, unsigned int> classifications;

  for (const auto& sys : m_systematicsList.systematicsVector())
  {
    const xAOD::IParticleContainer *particles = nullptr;
    ANA_CHECK(m_particlesHandle.retrieve(particles, sys, ctx));

    for (const xAOD::IParticle *particle : *particles)
      {
        const xAOD::IParticle *key = xAOD::getOriginalObject (*particle);
        if (key == nullptr)
          key = particle;

        auto iter = classifications.find (key);
        if (iter == classifications.end())
          {
            unsigned int classification = 0;
            ANA_CHECK (m_tool->classify (*particle, classification));
            iter = classifications.emplace (key, classification).first;
          }
        m_classificationDecorator.set (*particle, iter->second, sys);
      }
  }

  return StatusCode::SUCCESS;
}

} // namespace CP
