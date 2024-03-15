/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Tadej Novak <tadej@cern.ch>


#include <AsgAnalysisAlgorithms/AsgClassificationDecorationAlg.h>


namespace CP
{

AsgClassificationDecorationAlg::AsgClassificationDecorationAlg(const std::string &name,
                                               ISvcLocator *pSvcLocator)
    : AnaAlgorithm(name, pSvcLocator), m_tool("",this)
{
  declareProperty ("tool", m_tool, "classification tool");
}



StatusCode AsgClassificationDecorationAlg::initialize()
{

  ANA_CHECK (m_particlesHandle.initialize (m_systematicsList));
  ANA_CHECK (m_classificationDecorator.initialize (m_systematicsList, m_particlesHandle));
  ANA_CHECK (m_systematicsList.initialize());

  ANA_CHECK(m_tool->initialize());

  return StatusCode::SUCCESS;
}



StatusCode AsgClassificationDecorationAlg::execute()
{

  std::vector<unsigned int> classifications;

  for (const auto& sys : m_systematicsList.systematicsVector())
  {
    const xAOD::IParticleContainer *particles = nullptr;
    ANA_CHECK(m_particlesHandle.retrieve(particles, sys));

    if (sys.empty()) {
      // we only run the IFF tool on the nominal calibration
      for (const xAOD::IParticle *particle : *particles)
	{
	  unsigned int classification(0);
	  ANA_CHECK(m_tool->classify(*particle, classification));
	  m_classificationDecorator.set(*particle, classification, sys);
	  classifications.push_back(classification);
	}
    }
    else {
      // and for all other systematics, just propagate the decoration
      unsigned int index = 0;
      for (const xAOD::IParticle *particle : *particles)
	{
	  m_classificationDecorator.set(*particle, classifications.at(index), sys);
	  index ++;
	}
    }
  }

  return StatusCode::SUCCESS;
}

} // namespace CP
