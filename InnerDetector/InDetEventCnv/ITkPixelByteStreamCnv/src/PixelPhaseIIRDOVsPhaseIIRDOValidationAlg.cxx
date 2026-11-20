/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "PixelPhaseIIRDOVsPhaseIIRDOValidationAlg.h"
#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "StoreGate/ReadHandle.h"
#include <cstddef>

StatusCode PixelPhaseIIRDOVsPhaseIIRDOValidationAlg::initialize()
{
  ATH_CHECK(PixelRDOValidationCommons::initialize());
  ATH_CHECK(m_referenceRDOKey.initialize());
  ATH_CHECK(m_monitoredRDOKey.initialize());
  return StatusCode::SUCCESS;
}

StatusCode PixelPhaseIIRDOVsPhaseIIRDOValidationAlg::makeSortedReferenceRDOs(
  const EventContext & ctx,
  TSortedRDOContainer & reference_rdos, size_t & nb_reference_rdos)
  const
{
  auto ref = SG::makeHandle(m_referenceRDOKey, ctx);
  ATH_CHECK(ref.isValid());

  if (ref->size() == 0) {
    ATH_MSG_ERROR("empty reference container");
    return StatusCode::FAILURE;
  }

  return makeSortedPhaseIIRDOs(ref, reference_rdos, nb_reference_rdos);
}

StatusCode PixelPhaseIIRDOVsPhaseIIRDOValidationAlg::makeSortedMonitoredRDOs(
  const EventContext & ctx,
  TSortedRDOContainer & monitored_rdos, size_t & nb_monitored_rdos)
  const
{
  auto mon = SG::makeHandle(m_monitoredRDOKey, ctx);
  ATH_CHECK(mon.isValid());

  if (mon->size() == 0) {
    ATH_MSG_ERROR("empty monitored container");
    return StatusCode::FAILURE;
  }

  return makeSortedPhaseIIRDOs(mon, monitored_rdos, nb_monitored_rdos);
}