/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Tadej Novak


//
// includes
//

#include <TriggerAnalysisAlgorithms/TrigPrescalesAlg.h>

#include <RootCoreUtils/StringUtil.h>
#include <xAODEventInfo/EventInfo.h>

//
// method implementations
//

namespace CP
{
  TrigPrescalesAlg ::
  TrigPrescalesAlg (const std::string& name, 
                    ISvcLocator* pSvcLocator)
    : AnaAlgorithm (name, pSvcLocator)
    , m_pileupReweightingTool ("CP::PileupReweightingTool", this)
  {
    declareProperty ("pileupReweightingTool", m_pileupReweightingTool, "the pileup reweighting tool to be used");
    declareProperty ("triggers", m_trigList, "trigger list");
    declareProperty ("triggersAll", m_trigListAll, "all trigger list");
    declareProperty ("triggersFormula", m_trigFormula, "produce prescale based on formula instead of per trigger, "
                                                       "e.g. (trigA||trigB)");
    declareProperty ("prescaleDecoration", m_prescaleDecoration, "decoration to store prescales");
  }



  StatusCode TrigPrescalesAlg ::
  initialize ()
  {
    if (m_prescaleDecoration.empty())
    {
      ANA_MSG_ERROR ("The decoration should not be empty");
      return StatusCode::FAILURE;
    }

    if (m_trigList.empty() && m_trigFormula.empty())
    {
      ANA_MSG_ERROR ("Either a list of triggers or trigger formula need to be provided");
      return StatusCode::FAILURE;
    }
    
    if (!m_trigList.empty() && !m_trigFormula.empty())
    {
      ANA_MSG_ERROR ("Provide either only a list of triggers or only a trigger formula");
      return StatusCode::FAILURE;
    }
    
    ANA_CHECK (m_pileupReweightingTool.retrieve());

    if (!m_trigFormula.empty())
    {
      m_prescaleAccessors.emplace_back(m_prescaleDecoration);
      m_prescaleFunctions.emplace_back([this](const xAOD::EventInfo *evtInfo, const std::string &trigger)
      {
        return m_pileupReweightingTool->getDataWeight (*evtInfo, trigger, true);
      });
      // By putting the formula into` m_trigListAll` 
      // the logic in `execute` does not have to change 
      // depending on if `m_trigFormula` or `m_trigList` is used
      m_trigListAll = {m_trigFormula}; 
      return StatusCode::SUCCESS;
    }

    if (m_trigListAll.empty())
    {
      m_trigListAll = m_trigList;
    }

    for (const std::string &chain : m_trigListAll)
    {
      m_prescaleAccessors.emplace_back(m_prescaleDecoration + "_" + RCU::substitute (chain, "-", "_"));

      // Generate helper functions
      if (std::find(m_trigList.begin(), m_trigList.end(), chain) != m_trigList.end())
      {
        m_prescaleFunctions.emplace_back([this](const xAOD::EventInfo *evtInfo, const std::string &trigger)
        {
          return m_pileupReweightingTool->getDataWeight (*evtInfo, trigger, true);
        });
      }
      else
      {
        m_prescaleFunctions.emplace_back([](const xAOD::EventInfo *, const std::string &)
        {
          return invalidTriggerPrescale();
        });
      }
    }


    return StatusCode::SUCCESS;
  }



  StatusCode TrigPrescalesAlg ::
  execute ()
  {
    const xAOD::EventInfo *evtInfo{};
    ANA_CHECK (evtStore()->retrieve(evtInfo, "EventInfo"));

    for (size_t i = 0; i < m_trigListAll.size(); i++)
    {
      (m_prescaleAccessors[i]) (*evtInfo) = (m_prescaleFunctions[i]) (evtInfo, m_trigListAll[i]);
    }

    return StatusCode::SUCCESS;
  }
}
