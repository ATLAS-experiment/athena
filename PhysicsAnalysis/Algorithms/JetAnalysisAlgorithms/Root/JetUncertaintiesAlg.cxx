/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <JetAnalysisAlgorithms/JetUncertaintiesAlg.h>

//
// method implementations
//

namespace CP
{

  StatusCode JetUncertaintiesAlg ::
  initialize ()
  {
    ANA_CHECK (m_uncertaintiesTool.retrieve());
    ANA_CHECK (m_jetHandle.initialize (m_systematicsList));
    ANA_CHECK (m_preselection.initialize (m_systematicsList, m_jetHandle, SG::AllowEmpty));
    ANA_CHECK (m_systematicsList.addSystematics (*m_uncertaintiesTool));
    if (!m_uncertaintiesToolPD.empty()) {
      ANA_CHECK (m_uncertaintiesToolPD.retrieve());
      ANA_CHECK (m_systematicsList.addSystematics (*m_uncertaintiesToolPD));
    }
    ANA_CHECK (m_systematicsList.initialize());
    ANA_CHECK (m_outOfValidity.initialize());

    if (!m_isJESbtag.empty()) m_decIsJESbtag.emplace(m_isJESbtag);
    const std::string labelB = "PartonTruthLabelID";
    m_accTruthLabel.emplace(labelB);

    // CPU-optimisation: differentiate the systematics for the two tools
    // in initialisation rather than execution
    for (const auto&sys : m_systematicsList.systematicsVector())
      {
	if (sys.name().find("PseudoData") != std::string::npos) {
	  m_systematicsVectorOnlyJERPseudoData.push_back(sys);
	}
	else {
	  m_systematicsVector.push_back(sys);
	}
      }
    return StatusCode::SUCCESS;
  }



  StatusCode JetUncertaintiesAlg ::
  execute (const EventContext& ctx)
  {
    ANA_CHECK (applyTool (m_uncertaintiesTool, m_systematicsVector, true, ctx));
    ANA_CHECK (applyTool (m_uncertaintiesToolPD, m_systematicsVectorOnlyJERPseudoData, false, ctx));

    return StatusCode::SUCCESS;
  }



  StatusCode JetUncertaintiesAlg ::
  applyTool (ToolHandle<ICPJetUncertaintiesTool>& tool,
             const std::vector<CP::SystematicSet>& systematics,
             bool decorateBtag, const EventContext& ctx)
  {
    for (const auto& sys : systematics)
    {
      ANA_CHECK (tool->applySystematicVariation (sys));
      xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_jetHandle.getCopy (jets, sys, ctx));
      for (xAOD::Jet *jet : *jets)
      {
        // we need to tell the JES flavour uncertainty tool whether each jet is b-tagged at truth-level
        if (decorateBtag && m_decIsJESbtag) {
          (*m_decIsJESbtag)(*jet) = m_accTruthLabel->isAvailable (*jet) && (*m_accTruthLabel)(*jet) == 5;
        }
        if (m_preselection.getBool (*jet, sys))
        {
          ANA_CHECK_CORRECTION (m_outOfValidity, *jet, tool->applyCorrection (*jet));
        }
      }
    }

    return StatusCode::SUCCESS;
  }
}
