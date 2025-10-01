/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <JetAnalysisAlgorithms/JvtUpdateAlg.h>

//
// method implementations
//

namespace CP
{

  StatusCode JvtUpdateAlg ::
  initialize ()
  {
    if (m_decorationName.empty())
    {
      ANA_MSG_ERROR ("decoration name set to empty string, not allowed");
      return StatusCode::FAILURE;
    }
    m_decorationAccessor = std::make_unique
      <SG::Accessor<float> > (m_decorationName);

    ANA_CHECK (m_jvtTool.retrieve());
    ANA_CHECK (m_jetHandle.initialize (m_systematicsList));
    ANA_CHECK (m_preselection.initialize (m_systematicsList, m_jetHandle, SG::AllowEmpty));
    ANA_CHECK (m_systematicsList.initialize());
    return StatusCode::SUCCESS;
  }



  StatusCode JvtUpdateAlg ::
  execute ()
  {
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_jetHandle.getCopy (jets, sys));
      for (xAOD::Jet *jet : *jets)
      {
        // Set default value for jets that fail preselection
        (*m_decorationAccessor) (*jet) = -1.;
        if (m_preselection.getBool (*jet, sys))
        {
          // manually update jvt decoration using the tool
          const float jvt = m_jvtTool->updateJvt (*jet);
          (*m_decorationAccessor) (*jet) = jvt;
        }
      }
    }
    return StatusCode::SUCCESS;
  }
}
