/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonByteStream/NrpcRawDataProvider.h"

// --------------------------------------------------------------------
// Constructor

Muon::NrpcRawDataProvider::NrpcRawDataProvider(const std::string& name, ISvcLocator* pSvcLocator) :
    AthReentrantAlgorithm(name, pSvcLocator) {}

StatusCode Muon::NrpcRawDataProvider::initialize() {
    ATH_MSG_INFO("NrpcRawDataProvider::initialize");

    ATH_CHECK(m_rawDataTool.retrieve());

    return StatusCode::SUCCESS;
}

// --------------------------------------------------------------------
// Execute

StatusCode Muon::NrpcRawDataProvider::execute(const EventContext& ctx) const {
    ATH_MSG_VERBOSE("NrpcRawDataProvider::execute");

    ATH_CHECK(m_rawDataTool->convert(ctx));

    return StatusCode::SUCCESS;
}
