/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Tadej Novak



//
// includes
//

#include <FTagAnalysisAlgorithms/BTaggingInformationDecoratorAlg.h>

//
// method implementations
//

namespace CP
{

  StatusCode BTaggingInformationDecoratorAlg ::
  initialize ()
  {
    if (m_taggerWeightDecoration.empty() && m_quantileDecoration.empty())
    {
      ANA_MSG_ERROR ("at least one of taggerWeightDecoration and quantileDecoration must be set");
      return StatusCode::FAILURE;
    }

    if (!m_taggerWeightDecoration.empty())
    {
      m_taggerWeightDecorator = std::make_unique<SG::Decorator<float> > (m_taggerWeightDecoration);
    }
    if (!m_quantileDecoration.empty())
    {
      m_quantileDecorator = std::make_unique<SG::Decorator<int> > (m_quantileDecoration);
    }

    ANA_CHECK (m_selectionTool.retrieve());
    ANA_CHECK (m_jetHandle.initialize (m_systematicsList));
    ANA_CHECK (m_preselection.initialize (m_systematicsList, m_jetHandle, SG::AllowEmpty));
    ANA_CHECK (m_systematicsList.initialize());
    ANA_CHECK (m_outOfValidity.initialize());

    return StatusCode::SUCCESS;
  }



  StatusCode BTaggingInformationDecoratorAlg ::
  execute (const EventContext& ctx)
  {
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      const xAOD::JetContainer *jets{};
      ANA_CHECK (m_jetHandle.retrieve (jets, sys, ctx));
      for (const xAOD::Jet *jet : *jets)
      {
        if (m_preselection.getBool (*jet, sys))
        {
          if (m_taggerWeightDecorator != nullptr)
          {
            double weight{-1.};
            ANA_CHECK_CORRECTION (m_outOfValidity, *jet, m_selectionTool->getTaggerWeight (*jet, weight));
            (*m_taggerWeightDecorator)(*jet) = weight;
          }

          if (m_quantileDecorator != nullptr)
          {
            const int quantile = m_selectionTool->getQuantile(*jet);
            (*m_quantileDecorator)(*jet) = quantile;
          }
        } else {
          if (m_taggerWeightDecorator != nullptr)
          {
            (*m_taggerWeightDecorator)(*jet) = -100.;
          }

          if (m_quantileDecorator != nullptr)
          {
            (*m_quantileDecorator)(*jet) = -1;
          }
        }
      }
    }
    return StatusCode::SUCCESS;
  }
}
