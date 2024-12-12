/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonCablingData/RpcFlatCableTranslator.h>
#include <AthenaBaseComps/AthMessaging.h>
#include <format>


namespace Muon{
    std::ostream &operator<<(std::ostream &ostr, const RpcFlatCableTranslator &map) {
        ostr << "flat-cable: " <<static_cast<unsigned>(map.id()) << ", \n   tdcs: ";
        auto dumpMapping = [&ostr](const RpcFlatCableTranslator::Storage_t &s) {
            for (size_t p = 0; p < RpcFlatCableTranslator::readStrips; ++p) {
                const unsigned int ch = s[p];
                if (ch != RpcFlatCableTranslator::notSet) {
                    ostr << std::format("{:2d}{:}", ch+1, (p + 1) != RpcFlatCableTranslator::readStrips ? "  " : "");
                }
                else {
                    ostr << std::format("--{:}", (p + 1) != RpcFlatCableTranslator::readStrips ? "  " : "");
                }
            }
        };
        dumpMapping(map.tdcMap());
        ostr << "\n strips: ";
        dumpMapping(map.stripMap());
        return ostr;
    }

    RpcFlatCableTranslator::RpcFlatCableTranslator(const uint8_t cardId):
        m_id{cardId} {}
    
    uint8_t RpcFlatCableTranslator::connectedChannels() const { return m_nCh; }
 
    const RpcFlatCableTranslator::Storage_t& RpcFlatCableTranslator::stripMap() const { return m_stripToTdc; }
    const RpcFlatCableTranslator::Storage_t& RpcFlatCableTranslator::tdcMap () const { return m_tdcToStrip; }

    uint8_t RpcFlatCableTranslator::id() const { return m_id; } 
    std::optional<uint8_t> RpcFlatCableTranslator::tdcChannel(uint8_t strip, MsgStream& log) const {
        const uint8_t mapStrip = (strip - firstStrip) % readStrips;
        if (m_stripToTdc[mapStrip] == notSet) {
            if (log.level() <= MSG::VERBOSE) {
                log<<MSG::VERBOSE<<"RpcFlatCableTranslator::"<<__func__<<"() - "<<__LINE__<<": The channel "
                   <<static_cast<unsigned>(strip)<<" is unmapped."<<endmsg;
            }
            return std::nullopt;
        }
        if (log.level() <= MSG::VERBOSE) {
            log<<MSG::VERBOSE<<"RpcFlatCableTranslator::"<<__func__<<"() - "<<__LINE__<<":  Map channel "
               <<static_cast<unsigned>(strip)<<" to tdc "<<static_cast<unsigned>(m_stripToTdc[mapStrip] + firstTdc)<<endmsg;
        }
        return std::make_optional<uint8_t>(m_stripToTdc[mapStrip] + firstTdc);
    }
    std::optional<uint8_t> RpcFlatCableTranslator::stripChannel(uint8_t tdcChannel, MsgStream& log) const {
        const uint8_t mapTdc = (tdcChannel - firstTdc);
        if (mapTdc >= readStrips && log.level() <= MSG::WARNING) {
            log<<MSG::WARNING<<"RpcFlatCableTranslator::"<<__func__<<"() - "<<__LINE__<<": The parsed tdc "
               <<static_cast<unsigned>(tdcChannel)<<" is out of range"<<endmsg;
            return std::nullopt;
        } 
        if (m_tdcToStrip[mapTdc] == notSet) {
            if (log.level() <= MSG::VERBOSE){
                log<<MSG::VERBOSE<<"RpcFlatCableTranslator::"<<__func__<<"() - "<<__LINE__<<": The parsed tdc "
                   <<static_cast<unsigned>(tdcChannel)<<" is not connected."<<endmsg;
            }
            return std::nullopt;
        }
        return m_tdcToStrip[mapTdc] + firstStrip;
    }
    bool RpcFlatCableTranslator::mapChannels(uint8_t strip, uint8_t tdc, MsgStream& log) {
        const uint8_t mapStrip = strip - firstStrip;
        const uint8_t mapTdc = tdc - firstTdc;
        if (mapStrip >= readStrips || m_stripToTdc[mapStrip] != notSet){
            log<<MSG::ERROR<<"RpcFlatCableTranslator::"<<__func__<<"() - "<<__LINE__<<": The strip "<<static_cast<unsigned>(strip)
               <<" is already mapped to "<<static_cast<unsigned>(m_stripToTdc[mapStrip] + firstTdc)<<std::endl<<(*this)<<endmsg;
            return false;
        }
        if (mapTdc >= readStrips || m_tdcToStrip[mapTdc] != notSet){
            log<<MSG::ERROR<<"RpcFlatCableTranslator::"<<__func__<<"() - "<<__LINE__<<": The strip "<<static_cast<unsigned>(strip)
               <<" is already mapped to "<<static_cast<unsigned>(m_tdcToStrip[mapTdc] + firstStrip)<<std::endl<<(*this)<<endmsg;
            return false;
        }
        m_stripToTdc[mapStrip] = mapTdc;
        m_tdcToStrip[mapTdc] = mapStrip;
        ++m_nCh;
        if (log.level() <= MSG::VERBOSE) {
            log<<MSG::VERBOSE<<"RpcFlatCableTranslator::"<<__func__<<"() - "<<__LINE__<<": Map strip "<<static_cast<unsigned>(strip)
                <<" to tdc "<<static_cast<unsigned>(tdc)<<endmsg;
        }
        return true;
    }
}