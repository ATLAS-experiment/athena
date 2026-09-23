#include "TgcCablingAlg.h"

#include <fstream>
#include <iterator>
#include <map>
#include <tuple>
#include "AthenaKernel/IOVInfiniteRange.h"
#include "CoralBase/AttributeList.h"
#include "MuonCablingDataR4/TgcCablingMap.h"
#include "StoreGate/ReadCondHandle.h"

namespace MuonR4 {

StatusCode TgcCablingAlg::initialize() {
    ATH_MSG_DEBUG("initialize " << name());
    ATH_CHECK(m_writeKey.initialize());
    ATH_CHECK(m_idHelperSvc.retrieve());

    const bool readFromDatabase{m_jsonFile.value().empty()};  // CHANGED
    ATH_CHECK(m_readKeyMap.initialize(readFromDatabase));     // ADDED

    if (readFromDatabase && m_readKeyMap.empty()) {  // CHANGED
        ATH_MSG_FATAL("Neither JSONFile nor MapFolders is configured");
        return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;
}

StatusCode TgcCablingAlg::parseJsonString(TgcCablingMap& cablingMap, const std::string& jsonString, const std::string& source) const {  // ADDED
    try {
        const nlohmann::json payload = nlohmann::json::parse(jsonString);
        ATH_CHECK(parsePayload(cablingMap, payload));
    } catch (const nlohmann::json::exception& err) {
        ATH_MSG_FATAL("Failed to parse TGC cabling JSON from " << source << ": " << err.what());
        return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
}

StatusCode TgcCablingAlg::execute(const EventContext& ctx) const {
    ATH_MSG_VERBOSE("TgcCablingAlg::execute()");

    SG::WriteCondHandle<TgcCablingMap> writeCablingHandle{m_writeKey, ctx};
    if (writeCablingHandle.isValid()) {
        ATH_MSG_DEBUG("CondHandle " << writeCablingHandle.fullKey() << " is already valid.");
        return StatusCode::SUCCESS;
    }

    auto writeCdo = std::make_unique<TgcCablingMap>(m_idHelperSvc.get());

    if (!m_jsonFile.value().empty()) {  // CHANGED
        writeCablingHandle.addDependency(EventIDRange(IOVInfiniteRange::infiniteRunLB()));

        const std::string& jsonFile = m_jsonFile.value();
        ATH_MSG_INFO("Load the Run-4 TGC cabling JSON file: " << jsonFile);

        std::ifstream jsonStream{jsonFile};
        if (!jsonStream.is_open()) {
            ATH_MSG_FATAL("Failed to open TGC cabling JSON file: " << jsonFile);
            return StatusCode::FAILURE;
        }

        const std::string jsonString{std::istreambuf_iterator<char>{jsonStream}, std::istreambuf_iterator<char>{}};  // CHANGED
        ATH_CHECK(parseJsonString(*writeCdo, jsonString, jsonFile));  // CHANGED
    } else {  // ADDED
        ATH_MSG_INFO("Load the Run-4 TGC cabling JSON from " << m_readKeyMap.fullKey());

        SG::ReadCondHandle<CondAttrListCollection> readHandle{m_readKeyMap, ctx};
        if (!readHandle.isValid()) {
            ATH_MSG_FATAL("Failed to retrieve conditions folder " << m_readKeyMap.fullKey());
            return StatusCode::FAILURE;
        }

        writeCablingHandle.addDependency(readHandle);

        for (const auto& databaseEntry : **readHandle) {
            const coral::AttributeList& attributes = databaseEntry.second;
            const auto* jsonString = static_cast<const std::string*>(attributes["data"].addressOfData());

            if (!jsonString) {
                ATH_MSG_FATAL("Null JSON payload in conditions folder " << m_readKeyMap.fullKey());
                return StatusCode::FAILURE;
            }

            ATH_CHECK(parseJsonString(*writeCdo, *jsonString, m_readKeyMap.key()));
        }
    }

    if (!writeCdo->finalize(msgStream())) {
        return StatusCode::FAILURE;
    }

    ATH_CHECK(writeCablingHandle.record(std::move(writeCdo)));
    return StatusCode::SUCCESS;
}

StatusCode TgcCablingAlg::findSLID(const nlohmann::json& stationBlock, int stationEta, int stationPhi, int16_t& slid) const {
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
        if (!slidEntry.contains("side") || !slidEntry.contains("stationPhi") || !slidEntry.contains("slid")) {
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

    ATH_MSG_FATAL("SLID not found for stationEta=" << stationEta << ", stationPhi=" << stationPhi);
    return StatusCode::FAILURE;
}

StatusCode TgcCablingAlg::parsePayload(TgcCablingMap& cablingMap, const nlohmann::json& payload) const {
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

        const std::string stationName = stationBlock.at("StationName").get<std::string>();
        const int stationNameIndex = m_idHelperSvc->tgcIdHelper().stationNameIndex(stationName);
        if (stationNameIndex < 0) {
            ATH_MSG_FATAL("Unknown TGC station name: " << stationName);
            return StatusCode::FAILURE;
        }

        const nlohmann::json& cellAddressMap = stationBlock.at("CellAddressMap");
        if (!cellAddressMap.is_array()) {
            ATH_MSG_FATAL("'CellAddressMap' is not an array");
            return StatusCode::FAILURE;
        }

        using OfflineGroup = std::tuple<int, int, int, int>;
        std::map<OfflineGroup, int16_t> firstASDChannels{};

        for (const auto& cablPayload : cellAddressMap) {
            if (!cablPayload.is_object()) {
                continue;
            }

            if (!cablPayload.contains("stationEta") ||
                !cablPayload.contains("stationPhi") ||
                !cablPayload.contains("GasGap") ||
                !cablPayload.contains("isStrip") ||
                !cablPayload.contains("ASDstartChannel")) {
                ATH_MSG_FATAL("Incomplete TGC cabling entry");
                return StatusCode::FAILURE;
            }

            const OfflineGroup group{
                cablPayload.at("stationEta").get<int>(),
                cablPayload.at("stationPhi").get<int>(),
                cablPayload.at("GasGap").get<int>(),
                cablPayload.at("isStrip").get<int>()
            };

            const int16_t asdStart = static_cast<int16_t>(
                cablPayload.at("ASDstartChannel").get<int>()
            );

            auto [firstItr, inserted] = firstASDChannels.emplace(group, asdStart);
            if (!inserted && asdStart < firstItr->second) {
                firstItr->second = asdStart;
            }
        }

        for (const auto& cablPayload : cellAddressMap) {
            if (!cablPayload.is_object()) {
                continue;
            }
            if (!cablPayload.contains("stationEta") || !cablPayload.contains("stationPhi") ||
                !cablPayload.contains("GasGap") || !cablPayload.contains("isStrip") ||
                !cablPayload.contains("ASDstartChannel") || !cablPayload.contains("reversed") ||
                !cablPayload.contains("cellAddress")) {
                ATH_MSG_FATAL("Incomplete TGC cabling entry");
                return StatusCode::FAILURE;
            }

            TgcCablingMap::JsonEntry entry{};
            entry.stationNameString = stationName;
            entry.stationName = static_cast<int8_t>(stationNameIndex);
            entry.stationEta = static_cast<int8_t>(cablPayload.at("stationEta").get<int>());
            entry.stationPhi = static_cast<int8_t>(cablPayload.at("stationPhi").get<int>());
            entry.gasGap = static_cast<int8_t>(cablPayload.at("GasGap").get<int>());
            entry.isStrip = static_cast<int8_t>(cablPayload.at("isStrip").get<int>());
            entry.ASDstartChannel = static_cast<int16_t>(cablPayload.at("ASDstartChannel").get<int>());
            entry.reversed = cablPayload.at("reversed").get<bool>();

            const OfflineGroup group{
                static_cast<int>(entry.stationEta),
                static_cast<int>(entry.stationPhi),
                static_cast<int>(entry.gasGap),
                static_cast<int>(entry.isStrip)
            };

            const auto firstItr = firstASDChannels.find(group);
            if (firstItr == firstASDChannels.end()) {
                ATH_MSG_FATAL("Failed to determine the first ASD channel");
                return StatusCode::FAILURE;
            }

            entry.offlineChannelStart = static_cast<int16_t>(
                entry.ASDstartChannel - firstItr->second + 1
            );

            ATH_CHECK(findSLID(stationBlock, entry.stationEta, entry.stationPhi, entry.SLID));

            if (entry.isStrip == 0) {
                if (!cablPayload.contains("channelRangeInASD")) {
                    ATH_MSG_FATAL("Wire entry is missing 'channelRangeInASD'");
                    return StatusCode::FAILURE;
                }

                const nlohmann::json& range = cablPayload.at("channelRangeInASD");
                if (!range.is_array() || range.size() != 2) {
                    ATH_MSG_FATAL("'channelRangeInASD' must have two entries");
                    return StatusCode::FAILURE;
                }

                entry.channelRangeStart = static_cast<int16_t>(range.at(0).get<int>());
                entry.channelRangeEnd = static_cast<int16_t>(range.at(1).get<int>());
            } else {
                entry.channelRangeStart = 1;
                entry.channelRangeEnd = 16;
            }

            const nlohmann::json& cellAddress = cablPayload.at("cellAddress");
            if (cellAddress.is_number_integer()) {
                entry.cellAddress1 = static_cast<int16_t>(cellAddress.get<int>());
                entry.cellAddress2 = static_cast<int16_t>(-1);
                entry.hasSecondCellAddress = false;
            } else if (cellAddress.is_array()) {
                if (cellAddress.empty() || cellAddress.size() > 2) {
                    ATH_MSG_FATAL("Only one or two cellAddress values are supported");
                    return StatusCode::FAILURE;
                }

                entry.cellAddress1 = static_cast<int16_t>(cellAddress.at(0).get<int>());
                if (cellAddress.size() == 2) {
                    entry.cellAddress2 = static_cast<int16_t>(cellAddress.at(1).get<int>());
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