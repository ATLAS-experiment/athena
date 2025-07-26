/*
   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#include "BJetThreeValueCheck.h"
#include "TrigBjetHypo/safeLogRatio.h"
#include "AthenaMonitoringKernel/Monitored.h"

BJetThreeValueCheck::BJetThreeValueCheck(
  const std::string& type,
  const std::string& name,
  const IInterface* parent ) :
  AthAlgTool( type, name, parent ),
  m_acc(nullptr)
{
}

StatusCode BJetThreeValueCheck::initialize() {
  if (m_cFraction < 0 || 1 < m_cFraction) {
    ATH_MSG_ERROR("c-jet fraction for b-tagger likelihood ratio calculation should be between 0 and 1!");
    ATH_MSG_ERROR("c-jet fraction requested: " << m_cFraction);
    return StatusCode::FAILURE;
  }
  if ( not m_monTool.name().empty() ) {
    ATH_CHECK( m_monTool.retrieve() );
    ATH_MSG_DEBUG("MonTool name: " << m_monTool);
  }
  m_acc.reset(new Accessors {
      std::string(m_bName),
      std::string(m_cName),
      std::string(m_uName)});
  return StatusCode::SUCCESS;
}

bool BJetThreeValueCheck::passThreshold(const SG::AuxElement& btag) const
{
  float b = -1.0;
  float c = -1.0;
  float u = -1.0;
  auto jet = getJetFromBTagLink(btag);
  if (m_acc->b.isAvailable(*jet)) {
    b = m_acc->b(*jet);
    c = m_acc->c(*jet);
    u = m_acc->u(*jet);
  } else {
    b = m_acc->b(btag);
    c = m_acc->c(btag);
    u = m_acc->u(btag);
  }
  float f = m_cFraction;
  float llr = safeLogRatio(b, f*c + (1-f)*u);
  Monitored::Group(m_monTool, Monitored::Scalar(m_llrName, llr));
  return llr > m_threshold;
}
