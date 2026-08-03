#include "TgcCablingAlg.h"

#include <fstream>

#include "AthenaKernel/IOVInfiniteRange.h"
#include "MuonCablingDataR4/TgcCablingMap.h"
#include "PathResolver/PathResolver.h"

namespace MuonR4 {

StatusCode TgcCablingAlg::initialize() {
    ATH_MSG_DEBUG("initialize " << name());

    ATH_CHECK(m_writeKey.initialize());
    ATH_CHECK(m_idHelperSvc.retrieve());

    if (m_jsonFile.value().empty()) {
        ATH_MSG_FATAL("JSONFile property is empty");
        return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;
}

StatusCode TgcCablingAlg::execute(const EventContext& ctx) const {
    ATH_MSG_VERBOSE("TgcCablingAlg::execute()");

    SG::WriteCondHandle<TgcCablingMap> writeCablingHandle{m_writeKey, ctx};

    if (writeCablingHandle.isValid()) {
        ATH_MSG_DEBUG("CondHandle " << writeCablingHandle.fullKey()
                                   << " is already valid.");
        return StatusCode::SUCCESS;
    }

    writeCablingHandle.addDependency(
        EventIDRange(IOVInfiniteRange::infiniteRunLB()));

    ATH_MSG_INFO("Load the Run-4 TGC cabling");

    const std::string resolvedFile =
        PathResolverFindCalibFile(m_jsonFile.value());

    if (resolvedFile.empty()) {
        ATH_MSG_FATAL("Failed to resolve TGC cabling JSON file: "
                      << m_jsonFile.value());
        return StatusCode::FAILURE;
    }

    ATH_MSG_INFO("Resolved TGC cabling JSON file: " << resolvedFile);

    std::ifstream jsonStream(resolvedFile);
    if (!jsonStream.is_open()) {
        ATH_MSG_FATAL("Failed to open TGC cabling JSON file: "
                      << resolvedFile);
        return StatusCode::FAILURE;
    }

    nlohmann::json payload;
    try {
        jsonStream >> payload;
    } catch (const nlohmann::json::exception& err) {
        ATH_MSG_FATAL("Failed to parse TGC cabling JSON file "
                      << resolvedFile << ": " << err.what());
        return StatusCode::FAILURE;
    }

    auto writeCdo = std::make_unique<TgcCablingMap>(m_idHelperSvc.get());

    ATH_CHECK(parsePayload(*writeCdo, payload));

    if (!writeCdo->finalize(msgStream())) {
        return StatusCode::FAILURE;
    }

    ATH_CHECK(writeCablingHandle.record(std::move(writeCdo)));
    return StatusCode::SUCCESS;
}

StatusCode TgcCablingAlg::findSLID(const nlohmann::json& stationBlock,
                                  int stationEta,
                                  int stationPhi,
                                  int16_t& slid) const {
    if (!stationBlock.contains("slid")) {
        ATH_MSG_FATAL("key 'slid' not found in station block");
        return StatusCode::FAILURE;
    }

    const nlohmann::json& slidMap = stationBlock.at("slid");

    if (!slidMap.is_array()) {
        ATH_MSG_FATAL("'slid' is not an array");
        return StatusCode::FAILURE;
    }

    int side{-1};

    if (stationEta < 0) {
        side = 0;
    } else if (stationEta > 0) {
        side = 1;
    } else {
        ATH_MSG_FATAL("stationEta must be non-zero");
        return StatusCode::FAILURE;
    }

    for (const auto& slidEntry : slidMap) {
        if (!slidEntry.is_object()) {
            continue;
        }

        if (!slidEntry.contains("side") ||
            !slidEntry.contains("stationPhi") ||
            !slidEntry.contains("slid")) {
            continue;
        }

        if (slidEntry.at("side").get<int>() != side) {
            continue;
        }

        if (slidEntry.at("stationPhi").get<int>() != stationPhi) {
            continue;
        }

        const nlohmann::json& slidValue = slidEntry.at("slid");

        if (slidValue.is_number_integer()) {
            slid = static_cast<int16_t>(slidValue.get<int>());
            return StatusCode::SUCCESS;
        }

        if (slidValue.is_array() && !slidValue.empty()) {
            slid = static_cast<int16_t>(slidValue.at(0).get<int>());
            return StatusCode::SUCCESS;
        }

        ATH_MSG_FATAL("Invalid slid value for stationPhi=" << stationPhi);
        return StatusCode::FAILURE;
    }

    ATH_MSG_FATAL("SLID not found for stationEta=" << stationEta
                                                   << ", stationPhi="
                                                   << stationPhi);
    return StatusCode::FAILURE;
}

StatusCode TgcCablingAlg::parsePayload(TgcCablingMap& cablingMap,
                                       const nlohmann::json& payload) const {
    if (!payload.contains("CablingData")) {
        ATH_MSG_FATAL("key 'CablingData' not found");
        return StatusCode::FAILURE;
    }

    const nlohmann::json& cablingData = payload.at("CablingData");

    if (!cablingData.is_array()) {
        ATH_MSG_FATAL("'CablingData' is not an array");
        return StatusCode::FAILURE;
    }

    for (const auto& stationBlock : cablingData) {
        if (!stationBlock.is_object()) {
            continue;
        }

        if (!stationBlock.contains("StationName")) {
            ATH_MSG_FATAL("key 'StationName' not found in station block");
            return StatusCode::FAILURE;
        }

        if (!stationBlock.contains("CellAddressMap")) {
            ATH_MSG_FATAL("key 'CellAddressMap' not found in station block");
            return StatusCode::FAILURE;
        }

        const std::string stationName =
            stationBlock.at("StationName").get<std::string>();

        const int stationNameIndex =
            m_idHelperSvc->tgcIdHelper().stationNameIndex(stationName);

        if (stationNameIndex < 0) {
            ATH_MSG_FATAL("Unknown TGC station name: " << stationName);
            return StatusCode::FAILURE;
        }

        const nlohmann::json& cellAddressMap =
            stationBlock.at("CellAddressMap");

        if (!cellAddressMap.is_array()) {
            ATH_MSG_FATAL("'CellAddressMap' is not an array");
            return StatusCode::FAILURE;
        }

        for (const auto& cablPayload : cellAddressMap) {
            if (!cablPayload.is_object()) {
                continue;
            }

            if (!cablPayload.contains("stationEta") ||
                !cablPayload.contains("stationPhi") ||
                !cablPayload.contains("GasGap") ||
                !cablPayload.contains("isStrip") ||
                !cablPayload.contains("ASDstartChannel") ||
                !cablPayload.contains("reversed") ||
                !cablPayload.contains("cellAddress")) {
                ATH_MSG_FATAL("Incomplete TGC cabling entry");
                return StatusCode::FAILURE;
            }

            TgcCablingMap::JsonEntry entry{};

            entry.stationNameString = stationName;
            entry.stationName = static_cast<int8_t>(stationNameIndex);
            entry.stationEta =
                static_cast<int8_t>(cablPayload.at("stationEta").get<int>());
            entry.stationPhi =
                static_cast<int8_t>(cablPayload.at("stationPhi").get<int>());
            entry.gasGap =
                static_cast<int8_t>(cablPayload.at("GasGap").get<int>());
            entry.isStrip =
                static_cast<int8_t>(cablPayload.at("isStrip").get<int>());
            entry.ASDstartChannel = static_cast<int16_t>(
                cablPayload.at("ASDstartChannel").get<int>());
            entry.reversed = cablPayload.at("reversed").get<bool>();

            ATH_CHECK(findSLID(stationBlock,
                               entry.stationEta,
                               entry.stationPhi,
                               entry.SLID));

            if (entry.isStrip == 0) {
                if (!cablPayload.contains("channelRangeInASD")) {
                    ATH_MSG_FATAL(
                        "Wire entry is missing 'channelRangeInASD'");
                    return StatusCode::FAILURE;
                }

                const nlohmann::json& range =
                    cablPayload.at("channelRangeInASD");

                if (!range.is_array() || range.size() != 2) {
                    ATH_MSG_FATAL(
                        "'channelRangeInASD' must have two entries");
                    return StatusCode::FAILURE;
                }

                entry.channelRangeStart =
                    static_cast<int16_t>(range.at(0).get<int>());
                entry.channelRangeEnd =
                    static_cast<int16_t>(range.at(1).get<int>());
            } else {
                entry.channelRangeStart = 1;
                entry.channelRangeEnd = 16;
            }

            const nlohmann::json& cellAddress =
                cablPayload.at("cellAddress");

            if (cellAddress.is_number_integer()) {
                entry.cellAddress1 =
                    static_cast<int16_t>(cellAddress.get<int>());
                entry.cellAddress2 = static_cast<int16_t>(-1);
                entry.hasSecondCellAddress = false;
            } else if (cellAddress.is_array()) {
                if (cellAddress.empty() || cellAddress.size() > 2) {
                    ATH_MSG_FATAL(
                        "Only one or two cellAddress values are supported");
                    return StatusCode::FAILURE;
                }

                entry.cellAddress1 =
                    static_cast<int16_t>(cellAddress.at(0).get<int>());

                if (cellAddress.size() == 2) {
                    entry.cellAddress2 =
                        static_cast<int16_t>(cellAddress.at(1).get<int>());
                    entry.hasSecondCellAddress = true;
                } else {
                    entry.cellAddress2 = static_cast<int16_t>(-1);
                    entry.hasSecondCellAddress = false;
                }
            } else {
                ATH_MSG_FATAL("'cellAddress' must be integer or array");
                return StatusCode::FAILURE;
            }

            ATH_CHECK(cablingMap.insertChannels(entry, msgStream()));
        }
    }

    return StatusCode::SUCCESS;
}

}  // namespace MuonR4