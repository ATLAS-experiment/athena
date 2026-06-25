/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file SCTSiLorentzAngleTestAlg.cxx
 * @author Susumu.Oda@cern.ch
 **/

// Local include
#include "SCTSiLorentzAngleTestAlg.h"

// Athena include
#include "InDetIdentifier/SCT_ID.h"

// Gaudi include

// STL include

SCTSiLorentzAngleTestAlg::SCTSiLorentzAngleTestAlg(const std::string& name, ISvcLocator* pSvcLocator):
  ::AthReentrantAlgorithm(name, pSvcLocator),
  m_id{nullptr}
{
}

StatusCode SCTSiLorentzAngleTestAlg::initialize()
{
  ATH_CHECK(m_tool.retrieve());
  ATH_CHECK(detStore()->retrieve(m_id, "SCT_ID"));

  return StatusCode::SUCCESS;
}

StatusCode SCTSiLorentzAngleTestAlg::execute(const EventContext& ctx) const
{
  unsigned int maxHash{static_cast<unsigned int>(m_id->wafer_hash_max())};
  for (unsigned int hash{0}; hash<maxHash; hash++) {
    const IdentifierHash elementHash{hash};
    const double lorentzShift{m_tool->getLorentzShift(elementHash, ctx)};
    const double lorentzShiftEta{m_tool->getLorentzShiftEta(elementHash, ctx)};
    const double tanLorentzAngle{m_tool->getTanLorentzAngle(elementHash, ctx)};
    const double tanLorentzAngleEta{m_tool->getTanLorentzAngleEta(elementHash, ctx)};
    const double biasVoltage{m_tool->getBiasVoltage(elementHash, ctx)};
    const double depletionVoltage{m_tool->getDepletionVoltage(elementHash, ctx)};
    const double temperature{m_tool->getTemperature(elementHash, ctx)};
    ATH_MSG_INFO("hash " << hash <<
                 " lorentzShift " << lorentzShift <<
                 " lorentzShiftEta " << lorentzShiftEta <<
                 " tanLorentzAngle " << tanLorentzAngle <<
                 " tanLorentzAngleEta " << tanLorentzAngleEta <<
                 " biasVoltage " << biasVoltage <<
                 " depletionVoltage " << depletionVoltage <<
                 " temperature " << temperature);
  }

  return StatusCode::SUCCESS;
}

StatusCode SCTSiLorentzAngleTestAlg::finalize()
{
  return StatusCode::SUCCESS;
}
