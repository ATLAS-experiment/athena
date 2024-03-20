/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina


//
// includes
//

#include <AsgAnalysisAlgorithms/EventCutFlowHistAlg.h>

#include <RootCoreUtils/StringUtil.h>
#include <TH1.h>

//
// method implementations
//

namespace CP
{

  StatusCode EventCutFlowHistAlg ::
  initialize ()
  {
    ANA_CHECK (m_eventInfoHandle.initialize (m_systematicsList));
    ANA_CHECK (m_preselection.initialize (m_systematicsList, m_eventInfoHandle, SG::AllowEmpty));
    ANA_CHECK (m_selections.initialize (m_systematicsList, m_eventInfoHandle));
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
      m_labels.push_back (label);
      m_allCutsNum ++;
    }
    assert (m_allCutsNum+1 == m_labels.size());

    return StatusCode::SUCCESS;
  }

  StatusCode EventCutFlowHistAlg ::
  execute ()
  {
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      const xAOD::EventInfo *evtInfo = nullptr;
      ANA_CHECK (m_eventInfoHandle.retrieve (evtInfo, sys));

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

      if (m_preselection.getBool (*evtInfo, sys)) {
	unsigned cutIndex = 1;
	histIter->second->Fill (0);
	for (size_t i{}; i < m_selections.size(); i++) {
	  if (m_selections.at(i).getBool (*evtInfo, sys) > 0) {
	    histIter->second->Fill (cutIndex);
	  }
	  cutIndex++;
	}
      }
    }
    return StatusCode::SUCCESS;
  }
}
