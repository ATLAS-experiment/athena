/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file MCEventInfoByteStreamTool_test.cxx
 * @brief Unit test for MCEventInfoByteStreamTool encode/decode round-trip.
 */

#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

#include "TestTools/initGaudi.h"
#include "GaudiKernel/IToolSvc.h"
#include "Gaudi/Interfaces/IOptionsSvc.h"
#include "GaudiKernel/ServiceHandle.h"
#include "StoreGate/StoreGateSvc.h"
#include "AthenaKernel/ExtendedEventContext.h"

#include "ByteStreamData/RawEvent.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODEventInfo/EventAuxInfo.h"
#include "eformat/Status.h"
#include "AthContainers/AuxElement.h"

#include "../src/MCEventInfoByteStreamTool.h"

#include <cassert>
#include <iostream>

namespace {
  const SG::AuxElement::Accessor<uint64_t> acc_pileUpMixtureLow("pileUpMixtureIDLowBits");
  const SG::AuxElement::Accessor<uint64_t> acc_pileUpMixtureHigh("pileUpMixtureIDHighBits");
}

int main() {
    std::cout << "MCEventInfoByteStreamTool_test\n";

    // Initialize Gaudi
    ISvcLocator* svcLoc = nullptr;
    assert(Athena_test::initGaudi(svcLoc));

    // Configure tool property via IOptionsSvc
    ServiceHandle<Gaudi::Interfaces::IOptionsSvc> joSvc("JobOptionsSvc", "test");
    assert(joSvc.retrieve().isSuccess());
    joSvc->set("ToolSvc.EncodeTool.EventInfoReadKey", "EventInfo");

    // Get services
    IToolSvc* toolSvc = nullptr;
    assert(svcLoc->service("ToolSvc", toolSvc).isSuccess());

    ServiceHandle<StoreGateSvc> evtStore("StoreGateSvc", "test");
    assert(evtStore.retrieve().isSuccess());

    // Create encode tool (property already configured via JobOptionsSvc)
    IAlgTool* encodeToolBase = nullptr;
    assert(toolSvc->retrieveTool("MCEventInfoByteStreamTool/EncodeTool", encodeToolBase).isSuccess());
    auto* encodeTool = dynamic_cast<MCEventInfoByteStreamTool*>(encodeToolBase);
    assert(encodeTool != nullptr);

    // Create decode tool
    IAlgTool* decodeToolBase = nullptr;
    assert(toolSvc->retrieveTool("MCEventInfoByteStreamTool/DecodeTool", decodeToolBase).isSuccess());
    auto* decodeTool = dynamic_cast<MCEventInfoByteStreamTool*>(decodeToolBase);
    assert(decodeTool != nullptr);

    // Create source EventInfo
    auto srcAux = std::make_unique<xAOD::EventAuxInfo>();
    auto srcInfo = std::make_unique<xAOD::EventInfo>();
    srcInfo->setStore(srcAux.get());

    srcInfo->setMCChannelNumber(410470);
    srcInfo->setMCEventNumber(123456789012345ULL);
    srcInfo->setActualInteractionsPerCrossing(32.5f);
    srcInfo->setAverageInteractionsPerCrossing(35.0f);
    srcInfo->setEventTypeBitmask(xAOD::EventInfo::IS_SIMULATION);
    srcInfo->setExtendedLevel1ID(42);
    srcInfo->setEventFlags(xAOD::EventInfo::Background, 0x100);
    srcInfo->setMCEventWeights({1.0f, 0.5f, -0.25f});
    acc_pileUpMixtureLow(*srcInfo) = 0xDEADBEEF12345678ULL;
    acc_pileUpMixtureHigh(*srcInfo) = 0xCAFEBABE87654321ULL;

    // Save expected values
    const uint32_t expChannel = srcInfo->mcChannelNumber();
    const uint64_t expEvtNum = srcInfo->mcEventNumber();
    const float expActualMu = srcInfo->actualInteractionsPerCrossing();
    const float expAvgMu = srcInfo->averageInteractionsPerCrossing();
    const uint32_t expTypeBitmask = srcInfo->eventTypeBitmask();
    const uint32_t expExtL1ID = srcInfo->extendedLevel1ID();
    const uint32_t expBgFlags = srcInfo->eventFlags(xAOD::EventInfo::Background);
    const uint64_t expPileUpLow = acc_pileUpMixtureLow(*srcInfo);
    const uint64_t expPileUpHigh = acc_pileUpMixtureHigh(*srcInfo);
    const std::vector<float> expWeights = srcInfo->mcEventWeights();

    // Record to StoreGate
    assert(evtStore->record(std::move(srcAux), "EventInfoAux.").isSuccess());
    assert(evtStore->record(std::move(srcInfo), "EventInfo").isSuccess());

    // Create EventContext
    EventContext ctx(0, 0);
    ctx.setExtension(Atlas::ExtendedEventContext(evtStore->hiveProxyDict()));

    // Encode
    std::vector<OFFLINE_FRAGMENTS_NAMESPACE_WRITE::ROBFragment*> robFragments;
    assert(encodeTool->convertToBS(robFragments, ctx).isSuccess());
    assert(robFragments.size() == 1);
    std::cout << "Encoded to " << robFragments.size() << " ROB fragment(s)\n";

    // Serialize ROB
    auto* wrobf = robFragments[0];
    std::vector<uint32_t> buffer(wrobf->size_word());
    uint32_t* ptr = buffer.data();
    eformat::write::copy(*wrobf->bind(), ptr, wrobf->size_word());

    // Parse as read ROB
    OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment rob(buffer.data());

    // Decode
    xAOD::EventInfo dstInfo;
    xAOD::EventAuxInfo dstAux;
    dstInfo.setStore(&dstAux);

    assert(decodeTool->convertFromBS(&rob, dstInfo).isSuccess());
    std::cout << "Decoded successfully\n";

    // Verify
    assert(dstInfo.mcChannelNumber() == expChannel);
    assert(dstInfo.mcEventNumber() == expEvtNum);
    assert(dstInfo.actualInteractionsPerCrossing() == expActualMu);
    assert(dstInfo.averageInteractionsPerCrossing() == expAvgMu);
    assert(dstInfo.eventTypeBitmask() == expTypeBitmask);
    assert(dstInfo.extendedLevel1ID() == expExtL1ID);
    assert(dstInfo.eventFlags(xAOD::EventInfo::Background) == expBgFlags);
    assert(acc_pileUpMixtureLow(dstInfo) == expPileUpLow);
    assert(acc_pileUpMixtureHigh(dstInfo) == expPileUpHigh);

    const auto& dstWeights = dstInfo.mcEventWeights();
    assert(dstWeights.size() == expWeights.size());
    for (size_t i = 0; i < expWeights.size(); ++i) {
        assert(dstWeights[i] == expWeights[i]);
    }

    std::cout << "All fields verified - PASSED\n";
    return 0;
}
