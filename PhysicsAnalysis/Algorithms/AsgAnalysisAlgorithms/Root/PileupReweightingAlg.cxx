/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//
#include "AsgAnalysisAlgorithms/PileupReweightingAlg.h"
#include "AsgDataHandles/WriteDecorHandle.h"
#include "AthContainers/ConstAccessor.h"
#include "AsgTools/CurrentContext.h"

/// Anonymous namespace for helpers
namespace {
  const static SG::ConstAccessor<unsigned int> accRRN("RandomRunNumber");
}

//
// method implementations
//

namespace CP
{

  StatusCode PileupReweightingAlg ::
  initialize ()
  {
    ANA_CHECK( m_correctedScaledAverageMuDecorator.initialize(SG::AllowEmpty) );
    ANA_CHECK( m_correctedActualMuDecorator.initialize(SG::AllowEmpty) );
    ANA_CHECK( m_correctedScaledActualMuDecorator.initialize(SG::AllowEmpty) );

    ANA_CHECK (m_eventInfoHandle.initialize(m_systematicsList));
    ANA_CHECK (m_weightDecorator.initialize(m_systematicsList, m_eventInfoHandle, SG::AllowEmpty));
    ANA_CHECK (m_pileupReweightingTool.retrieve());
    ANA_CHECK (m_systematicsList.addSystematics (*m_pileupReweightingTool));
    ANA_CHECK (m_systematicsList.initialize());
    ANA_CHECK (m_outOfValidity.initialize());
    ANA_CHECK (m_baseEventInfoName.initialize());
    ANA_CHECK (m_decRRNKey.initialize());
    ANA_CHECK (m_decRLBNKey.initialize());
    ANA_CHECK (m_decHashKey.initialize());
    return StatusCode::SUCCESS;
  }



  StatusCode PileupReweightingAlg ::
  execute ()
  {

    const EventContext& ctx = Gaudi::Hive::currentContext();
    SG::ReadHandle<xAOD::EventInfo> evtInfo(m_baseEventInfoName, ctx);

    // Add additional decorations - these apply to data (and on MC just redecorate the same value as
    // before)
    if (!m_correctedScaledAverageMuDecorator.empty())
    {
      SG::WriteDecorHandle<xAOD::EventInfo, float> dec (m_correctedScaledAverageMuDecorator, ctx);
      dec (*evtInfo)
        = m_pileupReweightingTool->getCorrectedAverageInteractionsPerCrossing (*evtInfo, true);
    }

    if (!m_correctedActualMuDecorator.empty())
    {
      SG::WriteDecorHandle<xAOD::EventInfo, float> dec (m_correctedActualMuDecorator, ctx);
      dec (*evtInfo)
        = m_pileupReweightingTool->getCorrectedActualInteractionsPerCrossing (*evtInfo);
    }

    if (!m_correctedScaledActualMuDecorator.empty())
    {
      SG::WriteDecorHandle<xAOD::EventInfo, float> dec (m_correctedScaledActualMuDecorator, ctx);
      dec (*evtInfo)
        = m_pileupReweightingTool->getCorrectedActualInteractionsPerCrossing (*evtInfo, true);
    }

    if (!evtInfo->eventType(xAOD::EventInfo::IS_SIMULATION))
      // The rest of the PRW tool only applies to MC
      return StatusCode::SUCCESS;

    // Deal with the parts that aren't related to systematics
    // Get random run and lumi block numbers
    SG::WriteDecorHandle<xAOD::EventInfo, unsigned int> decRRN (m_decRRNKey, ctx);
    unsigned int rrn = 0;
    if(decRRN.isAvailable())
      rrn = accRRN(*evtInfo);
    else{
      rrn = m_pileupReweightingTool->getRandomRunNumber(*evtInfo, true);
      // If it returns 0, try again without the mu dependence
      if(rrn == 0)
        rrn = m_pileupReweightingTool->getRandomRunNumber(*evtInfo, false);
      decRRN(*evtInfo) = rrn;
    }

    SG::WriteDecorHandle<xAOD::EventInfo, unsigned int> decRLBN (m_decRLBNKey, ctx);
    if(!decRLBN.isAvailable())
      decRLBN(*evtInfo) = (rrn == 0) ? 0 : m_pileupReweightingTool->GetRandomLumiBlockNumber(rrn);

    // Also decorate with the hash, this can be used for rerunning PRW (but usually isn't)
    SG::WriteDecorHandle<xAOD::EventInfo, uint64_t> decHash (m_decHashKey, ctx);
    if(!decHash.isAvailable())
      decHash(*evtInfo) = m_pileupReweightingTool->getPRWHash(*evtInfo);

    // Take care of the weight (which is the only thing depending on systematics)
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      const xAOD::EventInfo* systEvtInfo = nullptr;
      ANA_CHECK( m_eventInfoHandle.retrieve(systEvtInfo, sys));
      ANA_CHECK (m_pileupReweightingTool->applySystematicVariation (sys));
      if (m_weightDecorator) {
        // calculate and set the weight. The 'true' argument makes the tool treat unrepresented data
        // correctly if the corresponding property is set
        m_weightDecorator.set(*systEvtInfo, m_pileupReweightingTool->getCombinedWeight(*evtInfo, true), sys);
        m_weightDecorator.lock(*systEvtInfo, sys);
      }

    };
    return StatusCode::SUCCESS;
  }
}
