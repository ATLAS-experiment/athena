/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonRdoToPrepData/MuonRdoToPrepDataAlg.h"

#include "Identifier/IdentifierHash.h"

MuonRdoToPrepDataAlg::MuonRdoToPrepDataAlg(const std::string& name, ISvcLocator* pSvcLocator) :
    AthReentrantAlgorithm(name, pSvcLocator) {}

StatusCode MuonRdoToPrepDataAlg::initialize() {
    ATH_MSG_DEBUG(" in initialize()");

    // verify that our tool handle is pointing to an accessible tool
    ATH_CHECK(m_tool.retrieve());
    ATH_CHECK(m_roiCollectionKey.initialize(m_seededDecoding));
    ATH_CHECK(m_regsel.retrieve(EnableTool{m_seededDecoding}));
    return StatusCode::SUCCESS;
}

StatusCode MuonRdoToPrepDataAlg::execute(const EventContext& ctx)  const {
    ATH_MSG_DEBUG("**************** in MuonRdoToPrepDataAlg::execute() ***********************************************");
    ATH_MSG_DEBUG("in execute()");

    std::vector<IdentifierHash> toDecode{}, toDecodeWithData{};
    std::vector<uint32_t> robs{};
            

    if (m_seededDecoding) {  // decoding from trigger roi
        SG::ReadHandle<TrigRoiDescriptorCollection> muonRoI(m_roiCollectionKey, ctx);
        ATH_CHECK(muonRoI.isPresent());
        for (const auto *roi : *muonRoI) {
            if (m_robDecoding) {
                    m_regsel->ROBIDList(*roi, robs);
            } else {
                m_regsel->HashIDList(*roi, toDecode);
            }               
            if (!robs.empty()) {
                ATH_CHECK(m_tool->decode(ctx, robs));
                robs.clear();
            } else if (!toDecode.empty()) {
                ATH_CHECK(m_tool->decode(ctx, toDecode, toDecodeWithData));
            } else {
                ATH_CHECK(m_tool->provideEmptyContainer(ctx));
            }
        }
              
    } else
        ATH_CHECK(m_tool->decode(ctx, toDecode, toDecodeWithData));

    if (m_print_inputRdo) m_tool->printInputRdo(ctx);
    if (m_print_prepData) m_tool->printPrepData(ctx);

    return StatusCode::SUCCESS;
}
