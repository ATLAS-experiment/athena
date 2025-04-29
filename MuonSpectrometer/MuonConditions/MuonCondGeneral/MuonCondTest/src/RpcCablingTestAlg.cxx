/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "RpcCablingTestAlg.h"
#include "StoreGate/ReadCondHandle.h"
#include "MuonReadoutGeometry/RpcReadoutElement.h"
#include "MuonCablingData/RpcFlatCableTranslator.h"

namespace Muon {
    StatusCode RpcCablingTestAlg::initialize(){
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(m_DetectorManagerKey.initialize());
        ATH_CHECK(m_cablingKey.initialize());
        for (const std::string &statName : m_considStat)
        {
            m_cabStat.insert(m_idHelperSvc->rpcIdHelper().stationNameIndex(statName));
        }
        return StatusCode::SUCCESS;
    }

    StatusCode RpcCablingTestAlg::execute() {
        const EventContext &ctx = Gaudi::Hive::currentContext();
        ATH_MSG_INFO("Start validation of the Rpc cabling");
        {
            RpcFlatCableTranslator flatCable{0};
            constexpr unsigned halfCard = RpcFlatCableTranslator::readStrips / 2;
            for (size_t n = 1; n <= RpcFlatCableTranslator::readStrips; ++n) {
                if (n <= halfCard && !flatCable.mapChannels(n, RpcFlatCableTranslator::readStrips-n, msgStream())) {
                    return StatusCode::FAILURE;
                } else if (n > halfCard && !flatCable.mapChannels(n, n - halfCard - 1, msgStream())) {
                    return StatusCode::FAILURE;
                }
            }
            ATH_MSG_INFO("Test flat cable map " << flatCable);
            for (size_t n = 1; n <= RpcFlatCableTranslator::readStrips; ++n) {
                const unsigned forthCh = flatCable.tdcChannel(n, msgStream()).value_or(0);
                if (const unsigned  backCh = flatCable.stripChannel(forthCh, msgStream()).value_or(0);
                    backCh != n) {
                    ATH_MSG_FATAL("Forward backward conversion of channel " << n << " failed. Ended up with " << backCh);
                    return StatusCode::FAILURE;
                }
            }
            ATH_MSG_INFO("Back <-> forth test is a great success");
        }

        SG::ReadCondHandle detectorMgr{m_DetectorManagerKey, ctx};
        if (!detectorMgr.isValid()) {
            ATH_MSG_FATAL("Failed to retrieve the Detector manager " << m_DetectorManagerKey.fullKey());
            return StatusCode::FAILURE;
        }

        SG::ReadCondHandle cabling{m_cablingKey, ctx};
        if (!cabling.isValid()) {
            ATH_MSG_ERROR("Failed to retrieve the Rpc cabling " << m_cablingKey.fullKey());
            return StatusCode::FAILURE;
        }
        const RpcIdHelper &idHelper{m_idHelperSvc->rpcIdHelper()};
        unsigned int n_elements{0}, n_success{0};
        for (auto det_itr = idHelper.detectorElement_begin(); det_itr != idHelper.detectorElement_end(); ++det_itr) {

            const MuonGM::RpcReadoutElement *readEle = detectorMgr->getRpcReadoutElement(*det_itr);
            if (!readEle) {
                ATH_MSG_VERBOSE("Detector element does not exist. ");
                continue;
            }
            if (!m_cabStat.empty() && !m_cabStat.count(m_idHelperSvc->stationName(readEle->identify()))) {
                ATH_MSG_VERBOSE("Do not test station " << m_idHelperSvc->toString(readEle->identify()));
                continue;
            }
            const Identifier station_id = idHelper.elementID(readEle->identify());
            ATH_MSG_DEBUG("Check station " << m_idHelperSvc->toString(station_id));

            for (bool measPhi : {true, false}) {
                for (int gap = 1; gap <= readEle->numberOfLayers(measPhi); ++gap) {
                    for (int strip = 1; strip <= readEle->Nstrips(measPhi); ++strip) {
                        bool is_valid{false};
                        const Identifier chanId = idHelper.channelID(station_id,
                                                                     readEle->getDoubletZ(),
                                                                     readEle->getDoubletPhi(),
                                                                     gap, measPhi, strip, is_valid);
                        if (!is_valid) {
                            ATH_MSG_VERBOSE("Invalid Identifier");
                            continue;
                        }
                        bool failure{false};

                        ++n_elements;
                        RpcCablingData cabl_data{};
                        if (!cabling->convert(chanId, cabl_data, false)) {
                            ATH_MSG_ERROR("Invalid identifier " << m_idHelperSvc->toString(chanId));
                            return StatusCode::FAILURE;
                        }
                        /// Next step is to obtain the online Id
                        if (!cabling->getOnlineId(cabl_data, msgStream())) {
                            return StatusCode::FAILURE;
                        }
                        ATH_MSG_DEBUG("Offline channel has an associated online channel " << cabl_data);
                        RpcCablingData onl_data{};
                        /// Construct the cabling online identifier
                        onl_data.RpcCablingOnlineID::operator=(cabl_data);
                        onl_data.channelId = cabl_data.channelId;
                        if (!cabling->getOfflineId(onl_data, msgStream())) {
                            return StatusCode::FAILURE;
                        }
                        if (onl_data != cabl_data) {
                            ATH_MSG_FATAL("Back translation of the cabling failed for hit " << m_idHelperSvc->toString(chanId) << std::endl
                                                                                            << " -- started with: " << cabl_data << std::endl
                                                                                            << " --     got back: " << onl_data);
                            failure = true;
                        }
                        Identifier backChanId{0};
                        if (!cabling->convert(onl_data, backChanId, true)) {
                            ATH_MSG_FATAL("Failed to translate back the cabling object" << onl_data << " to an Identifier");
                            return StatusCode::FAILURE;
                        }
                        if (backChanId != chanId) {
                            ATH_MSG_FATAL("Detected a silent messenger. Started with " 
                                        << m_idHelperSvc->toString(chanId) << " ended up with " << m_idHelperSvc->toString(backChanId));
                            failure = true;
                        }
                        n_success += (!failure);
                        if (failure)
                            return StatusCode::FAILURE;
                    }
                }
            }
        }
        ATH_MSG_INFO(n_success << " out of " << n_elements << " channels were successfully validated.");
        return n_success == n_elements ? StatusCode::SUCCESS : StatusCode::FAILURE;
    }

}