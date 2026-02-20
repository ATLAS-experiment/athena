/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack
/// @author Baptiste Ravina


//
// includes
//

#include <AsgAnalysisAlgorithms/ObjectCutFlowHistAlg.h>

#include <RootCoreUtils/StringUtil.h>
#include <TH1.h>

//
// method implementations
//

namespace CP
{

  StatusCode ObjectCutFlowHistAlg ::
  initialize ()
  {
    ANA_CHECK (m_inputHandle.initialize (m_systematicsList));
    ANA_CHECK (m_preselection.initialize (m_systematicsList, m_inputHandle, SG::AllowEmpty));
    ANA_CHECK (m_selections.initialize (m_systematicsList, m_inputHandle));
    ANA_CHECK (m_systematicsList.initialize());
    ANA_CHECK (m_selectionNameSvc.retrieve());

    // Total label
    m_labels.push_back ("total");
    // Individual labels
    for (size_t i{}; i < m_selections.size(); i++) {
      std::string label = m_selections.at(i).getSelectionName();
      // Check if the string ends with "_%SYS%"
      if (label.size() >= 6 && label.substr(label.size() - 6) == "_%SYS%") {
        // Remove "_%SYS%" from the end of the string
        label.erase(label.size() - 6);
      }
      m_labels.push_back (std::move(label));
      m_allCutsNum ++;
    }
    assert (m_allCutsNum+1 == m_labels.size());

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

      for (const xAOD::IParticle *particle : *input) {
        if (m_preselection.getBool (*particle, sys)) {
          unsigned cutIndex = 1;
          histIter->second->Fill (0);
          bool selected{true};
          for (size_t i{}; i < m_selections.size(); i++) {
            if ((!m_forceCutSequence.value() || selected) && m_selections.at(i).getBool (*particle, sys) > 0) {
              histIter->second->Fill (cutIndex);
            } else {
              selected = false;
            }
            cutIndex++;
          }
        }
      }

    }
    return StatusCode::SUCCESS;
  }
}
