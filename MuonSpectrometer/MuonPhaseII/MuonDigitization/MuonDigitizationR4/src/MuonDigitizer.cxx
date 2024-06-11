/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonDigitizer.h"


MuonDigitizer::MuonDigitizer(const std::string& name, ISvcLocator* pSvcLocator) : 
    AthAlgorithm(name, pSvcLocator) {}

StatusCode MuonDigitizer::initialize() {
    ATH_CHECK(m_digTool.retrieve());
    ATH_MSG_DEBUG("Retrieved MuonDigitzationTool (" << m_digTool->name() << ").");
    return StatusCode::SUCCESS;
}

StatusCode MuonDigitizer::execute() {
    ATH_MSG_DEBUG("in execute()");
    return m_digTool->processAllSubEvents(Gaudi::Hive::currentContext());
}
