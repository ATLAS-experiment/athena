/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonCablingData/RpcCablingMap.h"
#include "MuonCablingData/RpcFlatCableTranslator.h"

#include "MuonIdHelpers/IMuonIdHelperSvc.h" 
#include "MuonIdHelpers/RpcIdHelper.h"
#include <cmath>
#include <ranges>
#include <format>

namespace Muon {
    RpcCablingMap::RpcCablingMap(const Muon::IMuonIdHelperSvc* idHelperSvc):
        m_rpcIdHelper{idHelperSvc->rpcIdHelper()}{}

    RpcCablingMap::~RpcCablingMap() = default;

    bool RpcCablingMap::convert(const RpcCablingData& translator, Identifier& id, bool check_valid) const {
        bool valid{!check_valid};
        id = check_valid
           ? m_rpcIdHelper.channelID(translator.stationIndex, translator.eta, translator.phi,
                                     translator.doubletR, translator.doubletZ,
                                     translator.doubletPhi, translator.gasGap,
                                     translator.measuresPhi(), translator.strip, valid)
            : m_rpcIdHelper.channelID(translator.stationIndex, translator.eta, translator.phi,
                                      translator.doubletR, translator.doubletZ,
                                      translator.doubletPhi, translator.gasGap,
                                      translator.measuresPhi(), translator.strip);
        return valid;
    }

    bool RpcCablingMap::convert(const Identifier& channelId, RpcCablingData& translator, bool setSideBit) const {
        if (!m_rpcIdHelper.is_rpc(channelId)) {
            return false;
        }
        translator.stationIndex = m_rpcIdHelper.stationName(channelId);
        translator.eta = m_rpcIdHelper.stationEta(channelId);
        translator.phi = m_rpcIdHelper.stationPhi(channelId);
        translator.doubletR = m_rpcIdHelper.doubletR(channelId);
        translator.doubletPhi = m_rpcIdHelper.doubletPhi(channelId);
        translator.doubletZ = m_rpcIdHelper.doubletZ(channelId);
        translator.gasGap = m_rpcIdHelper.gasGap(channelId);
        translator.setMeasPhiAndSide(m_rpcIdHelper.measuresPhi(channelId), setSideBit);
        translator.strip = m_rpcIdHelper.strip(channelId);
        return true;
    }

    /** get the online id from the offline id */
    bool RpcCablingMap::getOnlineId(RpcCablingData& translatorCache, MsgStream& log) const {
        if (log.level()<= MSG::DEBUG) {
            log<<MSG::DEBUG<<"RpcCablingMap::"<<__func__<<"() - "<<__LINE__<<": test "<<translatorCache<<endmsg;
        }    
        const RpcCablingOfflineID& offId{translatorCache};
        OfflToOnlMap::const_iterator itr = m_offToOnline.find(offId);
        if (itr == m_offToOnline.end()) {
            log << MSG::ERROR<<"RpcCablingMap::"<<__func__<<"() - "<<__LINE__<<": The offline identifier "
                << offId << " is unknown " << endmsg;
            return false;
        }
        const AllTdcsPerGasGap& tdcs{itr->second};
        ///
        if (log.level() <= MSG::DEBUG) {
            log<<MSG::DEBUG<<"RpcCablingMap::"<<__func__<<"() - "<<__LINE__<<": Chamber "<<offId<<" has "
               <<tdcs.size()<<" cards "<<endmsg;
        }

        const AllTdcsPerGasGap::const_iterator onlineCardItr = 
                    std::ranges::find_if(tdcs, [&translatorCache](const GasGapToTdcAssociation& tdc){
                        return tdc.firstStrip + RpcFlatCableTranslator::readStrips > translatorCache.strip
                            && tdc.firstStrip <= translatorCache.strip;
                    });
        if (onlineCardItr != itr->second.end()) {
            const GasGapToTdcAssociation& onlineCard = (*onlineCardItr);
            /// Assign the TDC / tdcSector / subDetector ID
            std::optional<uint8_t> tdcChannel = onlineCard.flatCable->tdcChannel(translatorCache.strip, log);
            if (tdcChannel) {
                static_cast<RpcCablingOnlineID&>(translatorCache) = onlineCard.tdcID;
                translatorCache.channelId = (*tdcChannel);
                if (log.level() <= MSG::DEBUG) {
                    log <<MSG::DEBUG<<"RpcCablingMap::"<<__func__<<"() - "<<__LINE__<<": Mapped to card "
                        <<onlineCard.tdcID<<", channel: "<<static_cast<int>(translatorCache.channelId)<<endmsg;
                }
                return true;
            } else if (log.level() <= MSG::DEBUG) {
                log <<MSG::DEBUG<<"RpcCablingMap::"<<__func__<<"() - "<<__LINE__<<":\n"<<(*onlineCard.flatCable)<<endmsg;
            }
        }
        log << MSG::ERROR<<"RpcCablingMap::"<<__func__<<"() - "<<__LINE__<<": No tdc channel could be found for the object "
            << static_cast<const RpcCablingOfflineID&>(translatorCache)<< " strip: " << static_cast<int>(translatorCache.strip) << endmsg;
        return false;
    }
    bool RpcCablingMap::getOfflineId(RpcCablingData& translatorCache, MsgStream& log) const {
        OnlToOfflMap::const_iterator itr = m_onToOffline.find(translatorCache);
        if (itr == m_onToOffline.end()) {
            log << MSG::ERROR <<"RpcCablingMap::"<<__func__<<"() - "<<__LINE__<<": The tdc chip "
                << static_cast<const RpcCablingOnlineID&>(translatorCache) << " is unknown " << endmsg;
            return false;
        }
        for (const TdcToGasGapAssociation& connectChamb: itr->second) {
            const std::optional<uint8_t> stripCh = connectChamb.flatCable->stripChannel(translatorCache.channelId, log);
            /// The channel is not connected to this slot
            if (!stripCh) {
                continue;
            }
            /// Assign the chamber
            static_cast<RpcCablingOfflineID&>(translatorCache) = connectChamb.gasGapID;
            translatorCache.strip = (*stripCh) + connectChamb.firstStrip;
            if (log.level() <= MSG::DEBUG) {
                log <<MSG::DEBUG<<"RpcCablingMap::"<<__func__<<"() - "<<__LINE__<<":   -- Card "
                    <<connectChamb.gasGapID<<", strip: "<<static_cast<int>(translatorCache.strip)<<endmsg;
            }
            return true;
        }
        log << MSG::ERROR<<"RpcCablingMap::"<<__func__<<"() - "<<__LINE__<<": No tdc channel could be found for the object "
            << static_cast<const RpcCablingOnlineID&>(translatorCache)
            << " tdc: " << static_cast<int>(translatorCache.channelId) << endmsg;

        return false;
    }

    bool RpcCablingMap::insertChannels(CoolDBEntry&& entry, MsgStream& log) {
        RpcCablingOfflineID offCh{entry};
        RpcCablingOnlineID  onlCh{entry};
        GasGapToTdcAssociation offToOnl{};
        offToOnl.tdcID = onlCh;
        offToOnl.flatCable = entry.flatCable;
        offToOnl.firstStrip = entry.firstStrip;

        m_offToOnline[offCh].emplace_back(std::move(offToOnl));
        std::ranges::sort(m_offToOnline[offCh], 
                          [](const GasGapToTdcAssociation& a, const GasGapToTdcAssociation& b){
                            return a.firstStrip < b.firstStrip;
                          });

        TdcToGasGapAssociation onlToOff{};
        onlToOff.gasGapID = offCh;
        onlToOff.firstStrip = entry.firstStrip - RpcFlatCableTranslator::firstStrip;
        onlToOff.flatCable = entry.flatCable;
        m_onToOffline[onlCh].push_back(std::move(onlToOff));
        std::ranges::sort(m_onToOffline[onlCh], 
                          [](const TdcToGasGapAssociation& a, const TdcToGasGapAssociation& b){
                            return a.firstStrip < b.firstStrip;
                          });
        
        if (log.level()<= MSG::DEBUG) {
            log<<MSG::DEBUG<<"RpcCablingMap::"<<__func__<<"() - "<<__LINE__<<": Add new cabling channel "
                <<offCh<<std::format("firstStrip: {:2d}", entry.firstStrip)<<" - "<<onlCh
                <<std::format("flatCable: {:2d}", entry.flatCable->id())<<endmsg;
        }

        int sub = entry.subDetector;
        int rod = entry.boardSector;

        int32_t hardId = (sub << 16) | rod;

        const bool robInitialized = std::find(m_listOfROB.begin(), m_listOfROB.end(), hardId) != m_listOfROB.end();
        if (!robInitialized) {
            m_listOfROB.push_back(hardId);
        }
    
        return true;
    }
    bool RpcCablingMap::finalize(MsgStream& log) {
        if (m_offToOnline.empty()) {
            log << MSG::ERROR <<"RpcCablingMap::"<<__func__<<"() - "<<__LINE__
                <<": finalize() -- No data has been loaded " << endmsg;
            return false;
         }
         /// Generate the ROB maps
        RpcCablingData RobOffId{};
        RobOffId.strip = 1;
        for (const auto& [card, chambers] : m_onToOffline) {
            for (const TdcToGasGapAssociation& chamber : chambers) {
                static_cast<RpcCablingOfflineID&>(RobOffId) = chamber.gasGapID;
                Identifier chId{0};
                if (!convert(RobOffId, chId, true)) {
                    log << MSG::ERROR<<"RpcCablingMap::"<<__func__<<"() - "<<__LINE__
                        << ": Could not construct an offline identifier from " << chamber.gasGapID << endmsg;
                    return false;
                }
                IdentifierHash hash{0};
                if (m_rpcIdHelper.get_module_hash(chId, hash)) {
                    log << MSG::ERROR <<"RpcCablingMap::"<<__func__<<"() - "<<__LINE__
                        << ": Failed to generate a hash for " << chamber.gasGapID << endmsg;
                    return false;
                }
                uint32_t hardId = (card.subDetector << 16) | card.boardSector;
                m_chambROBs[hash] = hardId;
                std::vector<IdentifierHash>& robHashes = m_ROBHashes[hardId];
                if (std::ranges::find(robHashes, hash) == robHashes.end()){
                    robHashes.push_back(hash);
                }
            }
        }
        return true;
    }
    uint32_t RpcCablingMap::getROBId(const IdentifierHash& stationCode, MsgStream& log) const {
        ChamberToROBMap::const_iterator it = m_chambROBs.find(stationCode);
        if (it != m_chambROBs.end()) {
            return it->second;
        }
        if (log.level() <= MSG::WARNING) {
            log << MSG::WARNING << "Station code " << stationCode << " not found !! "<< endmsg;
        }
        return 0;
    }

    RpcCablingMap::ListOfROB RpcCablingMap::getROBId(const std::vector<IdentifierHash>& rpcHashVector, MsgStream& log) const {
        ListOfROB toRet{};
        toRet.reserve(rpcHashVector.size());
        for (const IdentifierHash& hash : rpcHashVector) {
            toRet.push_back(getROBId(hash, log));
        }
        return toRet;
    }
    const std::vector<IdentifierHash>& RpcCablingMap::getChamberHashVec(const uint32_t ROBI, MsgStream& log) const {
        ROBToChamberMap::const_iterator itr = m_ROBHashes.find(ROBI);
        if (itr != m_ROBHashes.end()){
            return itr->second;
        }
        if (log.level() <= MSG::WARNING) {
            log << MSG::WARNING << "ROB ID " << ROBI << " not found ! " << endmsg;
        }
        static const std::vector<IdentifierHash> dummy;
        return dummy;
    }
    std::vector<IdentifierHash> RpcCablingMap::getChamberHashVec(const ListOfROB& ROBs, MsgStream& log) const {
        std::unordered_set<IdentifierHash> hashSet{};
        for (const uint32_t rob : ROBs) {
            const std::vector<IdentifierHash>& hashFromROB = getChamberHashVec(rob, log);
            hashSet.insert(hashFromROB.begin(), hashFromROB.end());
        }
        std::vector<IdentifierHash> hashVec{};
        hashVec.insert(hashVec.end(), hashSet.begin(), hashSet.end());
        return hashVec;
    }

    const RpcCablingMap::ListOfROB& RpcCablingMap::getAllROBId() const {
        return m_listOfROB;
    }
    std::set<RpcCablingMap::FlatCablePtr> RpcCablingMap::flatCables() const {
        std::set<FlatCablePtr> cables{};
        for (const auto &[offId, gapToTdcs] : m_offToOnline) {
            for (const GasGapToTdcAssociation& assoc : gapToTdcs){
                cables.insert(assoc.flatCable);
            }
        }
        return cables;
    }
}
