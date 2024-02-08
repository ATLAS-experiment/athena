/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <AsgAnalysisAlgorithms/ObjectCutFlowHistAlg.h>

#include <PATCore/AcceptInfo.h>
#include <RootCoreUtils/StringUtil.h>
#include <TH1.h>

//
// method implementations
//

namespace CP
{
  ObjectCutFlowHistAlg ::
  ObjectCutFlowHistAlg (const std::string& name, 
                          ISvcLocator* pSvcLocator)
    : AnaAlgorithm (name, pSvcLocator)
  {
    declareProperty ("histPattern", m_histPattern, "the pattern for histogram names");

    declareProperty ("selection", m_selection, "the list of selection decorations");
  }



  StatusCode ObjectCutFlowHistAlg ::
  initialize ()
  {
    ANA_CHECK (m_inputHandle.initialize (m_systematicsList));
    ANA_CHECK (m_preselection.initialize (m_systematicsList, m_inputHandle, SG::AllowEmpty));
    ANA_CHECK (m_systematicsList.initialize());
    ANA_CHECK (m_selectionNameSvc.retrieve());

    // Total label
    m_labels.push_back ("total");

    for (std::size_t iter = 0, end = m_selection.size(); iter != end; ++ iter)
    {
      unsigned ncuts = 0u;
      std::unique_ptr<ISelectionReadAccessor> accessor;
      ANA_CHECK (makeSelectionReadAccessor (m_selection[iter], accessor));

      if (accessor->isBool())
      {
        ANA_MSG_DEBUG ("selection " << m_selection[iter] << " is a bool, using 1 cut");
        ncuts = 1;
        m_labels.push_back (accessor->label());
      } else if (const asg::AcceptInfo *acceptInfo = m_selectionNameSvc->getAcceptInfo (m_inputHandle.getNamePattern(), accessor->label());
          acceptInfo != nullptr)
      {
        ANA_MSG_DEBUG ("found accept info for " << m_inputHandle.getNamePattern() << " " << accessor->label());
        ncuts = acceptInfo->getNCuts();
        for (unsigned i = 0; i != ncuts; i++)
        {
          ANA_MSG_DEBUG ("using cut name from accept info: " << acceptInfo->getCutName (i));
          m_labels.push_back (acceptInfo->getCutName (i));
        }
      } else
      {
        ANA_MSG_ERROR ("could not find accept info for " << m_inputHandle.getNamePattern() << " " << accessor->label());
        return StatusCode::FAILURE;
      }

      m_accessors.push_back (std::make_pair (std::move (accessor), ncuts));
      m_allCutsNum += ncuts;
      assert (m_allCutsNum+1 == m_labels.size());
    }

    return StatusCode::SUCCESS;
  }



  StatusCode ObjectCutFlowHistAlg ::
  execute ()
  {
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      const xAOD::IParticleContainer *input = nullptr;
      ANA_CHECK (m_inputHandle.retrieve (input, sys));

      auto histIter = m_hist.find (sys);
      if (histIter == m_hist.end())
      {
        std::string name;
        ANA_CHECK (m_systematicsList.service().makeSystematicsName (name, m_histPattern, sys));

        std::string title = m_histTitle.value();
        if (!sys.empty())
          title += " (" + sys.name() + ")";
        ANA_CHECK (book (TH1F (name.c_str(), title.c_str(), m_allCutsNum+1, 0, m_allCutsNum+1)));

        m_hist.insert (std::make_pair (sys, hist (name)));
        histIter = m_hist.find (sys);
        assert (histIter != m_hist.end());

        for (unsigned i = 0; i < m_allCutsNum+1; i++)
        {
          histIter->second->GetXaxis()->SetBinLabel(i + 1, m_labels[i].c_str());
        }
      }

      for (const xAOD::IParticle *particle : *input)
      {
        if (m_preselection.getBool (*particle, sys))
        {
          bool keep = true;
          unsigned cutIndex = 1;
          histIter->second->Fill (0);
          for (const auto& accessor : m_accessors)
          {
            const auto selection = accessor.first->getBits (*particle);
            for (unsigned index = 0, end = accessor.second;
                 index != end; ++ index, ++ cutIndex)
            {
              if (selection & (1 << index))
              {
                histIter->second->Fill (cutIndex);
              } else
              {
                keep = false;
                break;
              }
            }
            if (!keep)
              break;
          }
        }
      }
    }

    return StatusCode::SUCCESS;
  }
}
