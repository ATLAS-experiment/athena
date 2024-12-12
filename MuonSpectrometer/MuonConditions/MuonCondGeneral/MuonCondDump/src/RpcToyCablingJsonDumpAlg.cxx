/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "RpcToyCablingJsonDumpAlg.h"

#include "MuonCablingData/RpcCablingData.h"
#include "MuonCablingData/RpcFlatCableTranslator.h"
#include "GeoModelKernel/throwExcept.h"
#include "nlohmann/json.hpp"
#include <fstream>

namespace Muon {
    StatusCode RpcToyCablingJsonDumpAlg::initialize() {
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(detStore()->retrieve(m_detMgr));
        if (!m_idHelperSvc->hasRPC()) {
            ATH_MSG_FATAL("You can't write rpc cablings without rpc detectors? ");
            return StatusCode::FAILURE;
        }

        m_BIL_stIdx = m_idHelperSvc->rpcIdHelper().stationNameIndex("BIL");
        m_BIS_stIdx = m_idHelperSvc->rpcIdHelper().stationNameIndex("BIS");
        return StatusCode::SUCCESS;
    }
    StatusCode RpcToyCablingJsonDumpAlg::execute() {
        std::ofstream outStream{m_cablingJSON};
        if (!outStream.good()) {
            ATH_MSG_FATAL("Failed to create JSON file " << m_cablingJSON);
            return StatusCode::FAILURE;
        }

        std::vector<const MuonGMR4::RpcReadoutElement *> reEles{m_detMgr->getAllRpcReadoutElements()};

        /// Subdetector identifiers (0x65 for chambers on the posivie side and 0x66 for chambers on the other side
        constexpr int subDetA{0x65}, subDetB{0x66};
        constexpr unsigned nStripsPerTdc = RpcFlatCableTranslator::readStrips;
       
        unsigned int tdcSecA{0}, tdcSecC{0};

        nlohmann::json chamberJSON{}, flatCableJSON{};
        std::vector<std::unique_ptr<RpcFlatCableTranslator>> flatTranslators{};

        auto connectFlatCable = [&flatCableJSON, &flatTranslators, this](const unsigned coveredStrips) -> unsigned{
            for (std::unique_ptr<RpcFlatCableTranslator>& translator : flatTranslators){
                if (static_cast<unsigned>(translator->connectedChannels()) == coveredStrips){
                    return translator->id();
                }
            }
            auto& translator = flatTranslators.emplace_back(std::make_unique<RpcFlatCableTranslator>(flatTranslators.size()));
            for (unsigned int s = RpcFlatCableTranslator::firstStrip; s <= coveredStrips; ++s) {
                if (!translator->mapChannels(s, RpcFlatCableTranslator::readStrips -s, msgStream())){
                    THROW_EXCEPTION("Channel mapping failed");
                }
            }
            nlohmann::json chipJSON{};
            chipJSON["flatCableId"] = translator->id();
            std::vector<std::pair<unsigned,unsigned>> pins{};
            for (unsigned int s = RpcFlatCableTranslator::firstStrip; s <= coveredStrips; ++s) {
                pins.emplace_back(s, translator->tdcChannel(s, msgStream()).value_or(RpcFlatCableTranslator::notSet));
            }
            chipJSON["pinAssignment"] = pins;

            flatCableJSON.push_back(chipJSON);
            return translator->id();
        };

        constexpr int16_t measPhiBit = RpcCablingData::measPhiBit;
        constexpr int16_t stripSideBit = RpcCablingData::stripSideBit;
        for (const MuonGMR4::RpcReadoutElement *reEle : reEles) {
            const unsigned int subDet = reEle->stationEta() > 0 ? subDetA : subDetB;
            const unsigned int tdcSec = reEle->stationEta() > 0 ? (++tdcSecA) : (++tdcSecC);
            unsigned int tdc{1};
            for (unsigned int gasGap = 1; gasGap <= reEle->nGasGaps(); ++gasGap) {
                for (int doubletPhi = reEle->doubletPhi(); doubletPhi <= reEle->doubletPhiMax(); ++doubletPhi) {
                    for (bool measPhi : {false, true}) {
                        const IdentifierHash layHash = reEle->createHash(1, gasGap, doubletPhi, measPhi);
                        if (!reEle->nStrips(layHash))
                            continue;
                        const unsigned int nStrips = reEle->nStrips(layHash);
                        const unsigned int nTdcChips = (nStrips % nStripsPerTdc > 0) +
                                                       (nStrips - (nStrips % nStripsPerTdc)) / nStripsPerTdc;

                        for (bool side : {false, true}) {
                            bool run4_BIS = ((reEle->stationName() == m_BIS_stIdx) && (std::abs(reEle->stationEta()) < 7));
                            if (side && reEle->stationName() != m_BIL_stIdx && !(run4_BIS)) {
                                /// Do not create side cablings for non BIL and non BIS 1-6 stations
                                continue;
                            }
                            unsigned int measPhiSide = (side * stripSideBit) | (measPhi * measPhiBit);
                            for (unsigned int chip = 0; chip < nTdcChips; ++chip) {
                                const unsigned firstStrip = (RpcFlatCableTranslator::firstStrip + nStripsPerTdc * chip);
                                const unsigned coveredStrips = std::min(nStripsPerTdc, nStrips - (firstStrip - RpcFlatCableTranslator::firstStrip));
                                nlohmann::json cablingChannel{};
                                cablingChannel["station"] = m_idHelperSvc->stationNameString(reEle->identify());
                                cablingChannel["eta"] = reEle->stationEta();
                                cablingChannel["phi"] = reEle->stationPhi();
                                cablingChannel["doubletR"] = reEle->doubletR();
                                cablingChannel["doubletZ"] = reEle->doubletZ();
                                cablingChannel["doubletPhi"] = doubletPhi;
                                cablingChannel["gasGap"] = gasGap;
                                cablingChannel["measPhi"] = measPhiSide;

                                cablingChannel["subDetector"] = subDet;
                                cablingChannel["tdcSector"] = tdcSec;
                                cablingChannel["firstStrip"] = firstStrip;
                                cablingChannel["tdc"] = (++tdc);
                                cablingChannel["flatCableId"] = connectFlatCable(coveredStrips);
                                ATH_MSG_VERBOSE("Install new cabling "<<m_idHelperSvc->toString(reEle->measurementId(layHash))
                                            <<"nStrips: "<<nStrips<<", stripsPerTdc"
                                    <<nStripsPerTdc<<", nChips: "<<nTdcChips<<", covered: "<<coveredStrips);
                                chamberJSON.push_back(cablingChannel);
                            }
                        }
                    }
                }
            }
        }
        nlohmann::json finalJSON{};
        finalJSON["chamberMap"] = chamberJSON;
        finalJSON["readoutCards"] = flatCableJSON;
        outStream << finalJSON.dump(2) << std::endl;
        return StatusCode::SUCCESS;
    }
}
