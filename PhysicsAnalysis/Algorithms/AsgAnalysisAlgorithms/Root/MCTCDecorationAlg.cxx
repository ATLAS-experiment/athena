/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Tadej Novak

#include <TruthUtils/TruthClassifiers.h>
#include <xAODTruth/TruthParticle.h>
#include <xAODTruth/xAODTruthHelpers.h>

#include <AsgAnalysisAlgorithms/MCTCDecorationAlg.h>


namespace CP
{

  MCTCDecorationAlg ::
  MCTCDecorationAlg (const std::string& name, 
                                  ISvcLocator* pSvcLocator)
    : AnaAlgorithm (name, pSvcLocator)
  {
  }


  StatusCode MCTCDecorationAlg ::
  initialize ()
  {
    if (m_classificationDecoration.empty())
    {
      ANA_MSG_ERROR("Classification decoration name needs to be set");
      return StatusCode::FAILURE;
    }

    m_classificationAccessor = std::make_unique<SG::AuxElement::ConstAccessor<unsigned int> > (m_classificationDecoration);

    if (!m_isPromptDecoration.empty())
    {
      m_isPromptDecorator = std::make_unique<SG::AuxElement::Decorator<int> > (m_isPromptDecoration);
    }
    if (!m_fromHadronDecoration.empty())
    {
      m_fromHadronDecorator = std::make_unique<SG::AuxElement::Decorator<int> > (m_fromHadronDecoration);
    }
    if (!m_fromBSMDecoration.empty())
    {
      m_fromBSMDecorator = std::make_unique<SG::AuxElement::Decorator<int> > (m_fromBSMDecoration);
    }
    if (!m_fromTauDecoration.empty())
    {
      m_fromTauDecorator = std::make_unique<SG::AuxElement::Decorator<int> > (m_fromTauDecoration);
    }

    ANA_CHECK (m_particlesHandle.initialize (m_systematicsList));
    ANA_CHECK (m_preselection.initialize (m_systematicsList, m_particlesHandle, SG::AllowEmpty));
    ANA_CHECK (m_systematicsList.initialize());

    return StatusCode::SUCCESS;
  }


  StatusCode MCTCDecorationAlg ::
  execute ()
  {
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      const xAOD::IParticleContainer *particles{};
      ANA_CHECK (m_particlesHandle.retrieve (particles, sys));
      for (const xAOD::IParticle *particle : *particles)
      {
        if (m_preselection.getBool (*particle, sys))
        {
          // Check if xAOD::TruthParticle or if not if it has the TruthParticleLink
          const xAOD::TruthParticle *truthParticle
            = dynamic_cast<const xAOD::TruthParticle *> (particle);
          if (truthParticle == nullptr)
          {
            // need to find the truth particle
            truthParticle = xAOD::TruthHelpers::getTruthParticle(*particle);
          }

          // run only on leptons
          if (truthParticle != nullptr && truthParticle->isLepton() && (truthParticle->status() == 1 || truthParticle->status() == 2))
          {
            unsigned int result{};
            if (m_classificationAccessor->isAvailable(*truthParticle))
            {
              result = (*m_classificationAccessor)(*truthParticle);
            }
            else
            {
              ANA_MSG_ERROR ("MCTC Classification decoration not available.");
              return StatusCode::FAILURE;
            }

            std::bitset<MCTruthPartClassifier::MCTC_bits::totalBits> bitset(result);
            if (m_isPromptDecorator != nullptr)
            {
              (*m_isPromptDecorator)(*particle) = MCTruthPartClassifier::isPrompt(result, true) ? 1 : 0;  // also accept 'unknown' with -1
            }
            if (m_fromHadronDecorator != nullptr)
            {
              (*m_fromHadronDecorator)(*particle) = bitset.test(MCTruthPartClassifier::MCTC_bits::hadron);
            }
            if (m_fromBSMDecorator != nullptr)
            {
              (*m_fromBSMDecorator)(*particle) = bitset.test(MCTruthPartClassifier::MCTC_bits::frombsm);
            }
            if (m_fromTauDecorator != nullptr)
            {
              (*m_fromTauDecorator)(*particle) = bitset.test(MCTruthPartClassifier::MCTC_bits::Tau);
            }
            continue;   
          }
        }

        // defaults
        if (m_isPromptDecorator != nullptr)
        {
          (*m_isPromptDecorator)(*particle) = -1;
        }
        if (m_fromHadronDecorator != nullptr)
        {
          (*m_fromHadronDecorator)(*particle) = -1;
        }
        if (m_fromBSMDecorator != nullptr)
        {
          (*m_fromBSMDecorator)(*particle) = -1;
        }
        if (m_fromTauDecorator != nullptr)
        {
          (*m_fromTauDecorator)(*particle) = -1;
        }
      }
    }

    return StatusCode::SUCCESS;
  }

} // namespace CP
