/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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
  execute ()
  {
    for (const auto& sys : m_systematicsVector)
      {
	ANA_CHECK (m_uncertaintiesTool->applySystematicVariation (sys));
	xAOD::JetContainer *jets = nullptr;
	ANA_CHECK (m_jetHandle.getCopy (jets, sys));
	for (xAOD::Jet *jet : *jets)
	  {
	    if (m_preselection.getBool (*jet, sys))
	      {
		ANA_CHECK_CORRECTION (m_outOfValidity, *jet, m_uncertaintiesTool->applyCorrection (*jet));
	      }
	  }
      }
    for (const auto& sys : m_systematicsVectorOnlyJERPseudoData)
    {
      ANA_CHECK (m_uncertaintiesToolPD->applySystematicVariation (sys));
      xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_jetHandle.getCopy (jets, sys));
      for (xAOD::Jet *jet : *jets)
	{
	  if (m_preselection.getBool (*jet, sys))
	    {
	      ANA_CHECK_CORRECTION (m_outOfValidity, *jet, m_uncertaintiesToolPD->applyCorrection (*jet));
	    }
	}
    }

    return StatusCode::SUCCESS;
  }
}
