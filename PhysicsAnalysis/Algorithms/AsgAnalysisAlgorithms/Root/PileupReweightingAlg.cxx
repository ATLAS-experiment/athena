/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//
#include "AsgAnalysisAlgorithms/PileupReweightingAlg.h"

/// Anonymous namespace for helpers
namespace {
  const static SG::ConstAuxElement::ConstAccessor<unsigned int> accRRN("RandomRunNumber");
  const static SG::ConstAuxElement::Decorator<unsigned int> decRRN("RandomRunNumber");
  const static SG::ConstAuxElement::Decorator<unsigned int> decRLBN("RandomLumiBlockNumber");
  const static SG::ConstAuxElement::Decorator<uint64_t> decHash("PRWHash");
}

//
// method implementations
//

namespace CP
{

  StatusCode PileupReweightingAlg ::
  initialize ()
  {
    if (!m_correctedScaledAverageMuDecoration.empty())
    {
      m_correctedScaledAverageMuDecorator = std::make_unique<SG::AuxElement::Decorator<float> > (m_correctedScaledAverageMuDecoration);
    }
    if (!m_correctedActualMuDecoration.empty())
    {
      m_correctedActualMuDecorator = std::make_unique<SG::AuxElement::Decorator<float> > (m_correctedActualMuDecoration);
    }
    if (!m_correctedScaledActualMuDecoration.empty())
    {
      m_correctedScaledActualMuDecorator = std::make_unique<SG::AuxElement::Decorator<float> > (m_correctedScaledActualMuDecoration);
    }

    ANA_CHECK (m_eventInfoHandle.initialize(m_systematicsList));
    ANA_CHECK (m_weightDecorator.initialize(m_systematicsList, m_eventInfoHandle, SG::AllowEmpty));
    ANA_CHECK (m_pileupReweightingTool.retrieve());
    ANA_CHECK (m_systematicsList.addSystematics (*m_pileupReweightingTool));
    ANA_CHECK (m_systematicsList.initialize());
    ANA_CHECK (m_outOfValidity.initialize());
    ANA_CHECK (m_baseEventInfoName.initialize());
    return StatusCode::SUCCESS;
  }



  StatusCode PileupReweightingAlg ::
  execute ()
  {

    SG::ReadHandle<xAOD::EventInfo> evtInfo(m_baseEventInfoName);

    // Add additional decorations - these apply to data (and on MC just redecorate the same value as
    // before)
    if (m_correctedScaledAverageMuDecorator)
    {
      (*m_correctedScaledAverageMuDecorator) (*evtInfo)
        = m_pileupReweightingTool->getCorrectedAverageInteractionsPerCrossing (*evtInfo, true);
    }

    if (m_correctedActualMuDecorator)
    {
      (*m_correctedActualMuDecorator) (*evtInfo)
        = m_pileupReweightingTool->getCorrectedActualInteractionsPerCrossing (*evtInfo);
    }

    if (m_correctedScaledActualMuDecorator)
    {
      (*m_correctedScaledActualMuDecorator) (*evtInfo)
        = m_pileupReweightingTool->getCorrectedActualInteractionsPerCrossing (*evtInfo, true);
    }

    if (!evtInfo->eventType(xAOD::EventInfo::IS_SIMULATION))
      // The rest of the PRW tool only applies to MC
      return StatusCode::SUCCESS;

    // Deal with the parts that aren't related to systematics
    // Get random run and lumi block numbers
    unsigned int rrn = 0;
    if(decRRN.isAvailable(*evtInfo))
      rrn = accRRN(*evtInfo);
    else{
      rrn = m_pileupReweightingTool->getRandomRunNumber(*evtInfo, true);
      // If it returns 0, try again without the mu dependence
      if(rrn == 0)
        rrn = m_pileupReweightingTool->getRandomRunNumber(*evtInfo, false);
      decRRN(*evtInfo) = rrn;
    }
    if(!decRLBN.isAvailable(*evtInfo))
      decRLBN(*evtInfo) = (rrn == 0) ? 0 : m_pileupReweightingTool->GetRandomLumiBlockNumber(rrn);
    // Also decorate with the hash, this can be used for rerunning PRW (but usually isn't)
    if(!decHash.isAvailable(*evtInfo))
      decHash(*evtInfo) = m_pileupReweightingTool->getPRWHash(*evtInfo);

    // Take care of the weight (which is the only thing depending on systematics)
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      const xAOD::EventInfo* systEvtInfo = nullptr;
      ANA_CHECK( m_eventInfoHandle.retrieve(systEvtInfo, sys));
      ANA_CHECK (m_pileupReweightingTool->applySystematicVariation (sys));
      if (m_weightDecorator)
        // calculate and set the weight. The 'true' argument makes the tool treat unrepresented data
        // correctly if the corresponding property is set
        m_weightDecorator.set(*systEvtInfo, m_pileupReweightingTool->getCombinedWeight(*evtInfo, true), sys);

    };
    return StatusCode::SUCCESS;
  }
}
