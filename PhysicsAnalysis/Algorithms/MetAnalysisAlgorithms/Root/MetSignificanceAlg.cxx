/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include <MetAnalysisAlgorithms/MetSignificanceAlg.h>

#include <xAODMissingET/MissingETAuxContainer.h>
#include "xAODEventInfo/EventInfo.h"

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

  if (!m_sigDirectionalDecorHandle.empty())
    ANA_CHECK(m_sigDirectionalDecorHandle.initialize(m_systematicsList, m_metHandle));

  if (!m_metOverSqrtSumETDecorHandle.empty())
    ANA_CHECK(m_metOverSqrtSumETDecorHandle.initialize(m_systematicsList, m_metHandle));

  if (!m_metOverSqrtHTDecorHandle.empty())
    ANA_CHECK(m_metOverSqrtHTDecorHandle.initialize(m_systematicsList, m_metHandle));

  ANA_CHECK(m_systematicsList.initialize());
  return StatusCode::SUCCESS;
}



  StatusCode MetSignificanceAlg ::
  execute ()
  {
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      // I'm not sure why this can't be const, but the interface
      // requires a non-const object
      xAOD::MissingETContainer *met {};
      ANA_CHECK (m_metHandle.getCopy (met, sys));

      const xAOD::EventInfo* evtInfo = 0;
      ANA_CHECK( evtStore()->retrieve( evtInfo, "EventInfo" ) );

      ANA_CHECK (m_significanceTool->varianceMET (met, evtInfo->averageInteractionsPerCrossing(), m_jetTermName, m_softTermName, m_totalMETName));

      m_significanceDecorHandle.set(*(*met)[m_totalMETName], m_significanceTool->GetSignificance(), sys);

      if (!m_sigDirectionalDecorHandle.empty())
        m_sigDirectionalDecorHandle.set(*(*met)[m_totalMETName], m_significanceTool->GetSigDirectional(), sys);

      if (!m_metOverSqrtSumETDecorHandle.empty())
        m_metOverSqrtSumETDecorHandle.set(*(*met)[m_totalMETName], m_significanceTool->GetMETOverSqrtSumET(), sys);

      if (!m_metOverSqrtHTDecorHandle.empty())
        m_metOverSqrtHTDecorHandle.set(*(*met)[m_totalMETName], m_significanceTool->GetMETOverSqrtHT(), sys);
    }

    return StatusCode::SUCCESS;
  }
}
