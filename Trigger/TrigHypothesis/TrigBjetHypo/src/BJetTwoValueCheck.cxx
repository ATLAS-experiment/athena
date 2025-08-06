/*
   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#include "BJetTwoValueCheck.h"
#include "TrigBjetHypo/safeLogRatio.h"
#include "AthenaMonitoringKernel/Monitored.h"

BJetTwoValueCheck::BJetTwoValueCheck(
  const std::string& type,
  const std::string& name,
  const IInterface* parent ) :
  AthAlgTool( type, name, parent ),
  m_acc(nullptr)
{
}

StatusCode BJetTwoValueCheck::initialize() {
  m_acc.reset(new Accessors {
      std::string(m_numName),
      std::string(m_denomName)});
  if ( not m_monTool.name().empty() ) {
    ATH_CHECK( m_monTool.retrieve() );
    ATH_MSG_DEBUG("MonTool name: " << m_monTool);
  }
  return StatusCode::SUCCESS;
}

bool BJetTwoValueCheck::passThreshold(const SG::AuxElement& btag) const
{
  float d = -1.0;
  float n = -1.0;
  auto jet = getJetFromBTagLink(btag);
  if (m_acc->n.isAvailable(*jet)) {
    n = m_acc->n(*jet);
    d = m_acc->d(*jet);
  } else {
    n = m_acc->n(btag);
    d = m_acc->d(btag);
  }
  float llr = safeLogRatio(n, d);
  Monitored::Group(m_monTool, Monitored::Scalar(m_llrName, llr));
  return llr > m_threshold;
}
