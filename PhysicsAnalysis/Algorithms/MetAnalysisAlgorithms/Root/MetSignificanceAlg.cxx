/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include <MetAnalysisAlgorithms/MetSignificanceAlg.h>
#include <AsgDataHandles/ReadHandle.h>

//
// method implementations
//
namespace CP
{

  StatusCode MetSignificanceAlg::initialize()
{
  ANA_CHECK(m_significanceTool.retrieve());
  ANA_CHECK(m_metHandle.initialize(m_systematicsList));
  ANA_CHECK(m_significanceDecorHandle.initialize(m_systematicsList, m_metHandle));

  ANA_CHECK(m_sigDirectionalDecorHandle.initialize(m_systematicsList, m_metHandle, SG::AllowEmpty));
  ANA_CHECK(m_metOverSqrtSumETDecorHandle.initialize(m_systematicsList, m_metHandle, SG::AllowEmpty));
  ANA_CHECK(m_metOverSqrtHTDecorHandle.initialize(m_systematicsList, m_metHandle, SG::AllowEmpty));
  ANA_CHECK(m_eventInfoKey.initialize());

  ANA_CHECK(m_systematicsList.initialize());
  return StatusCode::SUCCESS;
}



  StatusCode MetSignificanceAlg ::
  execute (const EventContext& ctx)
  {
    SG::ReadHandle<xAOD::EventInfo> evtInfo(m_eventInfoKey, ctx);

    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      // I'm not sure why this can't be const, but the interface
      // requires a non-const object
      xAOD::MissingETContainer *met {};
      ANA_CHECK (m_metHandle.getCopy (met, sys, ctx));

      const xAOD::MissingET *totalMET = (*met)[m_totalMETName];
      if (totalMET == nullptr)
      {
        ANA_MSG_ERROR ("could not find total MET term: " << m_totalMETName);
        return StatusCode::FAILURE;
      }

      ANA_CHECK (m_significanceTool->varianceMET (met, evtInfo->averageInteractionsPerCrossing(), m_jetTermName, m_softTermName, m_totalMETName));

      m_significanceDecorHandle.set(*totalMET, m_significanceTool->GetSignificance(), sys);

      if (!m_sigDirectionalDecorHandle.empty())
        m_sigDirectionalDecorHandle.set(*totalMET, m_significanceTool->GetSigDirectional(), sys);

      if (!m_metOverSqrtSumETDecorHandle.empty())
        m_metOverSqrtSumETDecorHandle.set(*totalMET, m_significanceTool->GetMETOverSqrtSumET(), sys);

      if (!m_metOverSqrtHTDecorHandle.empty())
        m_metOverSqrtHTDecorHandle.set(*totalMET, m_significanceTool->GetMETOverSqrtHT(), sys);
    }

    return StatusCode::SUCCESS;
  }
}
