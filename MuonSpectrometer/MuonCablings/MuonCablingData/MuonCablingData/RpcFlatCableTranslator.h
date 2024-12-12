/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONCABLINGDATA_ROCCABLINGTRANSLATOR_H
#define MUONCABLINGDATA_ROCCABLINGTRANSLATOR_H

#include <array>
#include <cstdint>
#include <optional>
#include <iostream>
#include <CxxUtils/ArrayHelper.h>

/**  @brief 32 Rpc strips are connected to a flat-cable connector socket. The signal is transmitted via the
 * 
 */
class MsgStream;
namespace Muon{
    class RpcFlatCableTranslator{
        public:
            /** @brief  Number of channels covered by one chip */
            static constexpr uint8_t readStrips = 32;
            /** @brief Default value indicating that the channel is not set */
            static constexpr uint8_t notSet = 250;
            /** @brief Convention of the lowest strip number */
            static constexpr uint8_t firstStrip = 1;
            /** @brief Convention of the lowest tdc number */
            static constexpr uint8_t firstTdc = 0;
            /** @brief Internal storage array */
            using Storage_t = std::array<uint8_t, readStrips>;
            /** @brief Standard constructor taking the card ID as input */
            RpcFlatCableTranslator(const uint8_t cardId);
            /** @brief Identifier of the card layout */
            uint8_t id() const;
            /** @brief Returns the connected tdc channel connected to a strip. If the strip is not connected to a 
             *         tdc, a nullopt is returned.  
             *  @param strip: Strip number. It should range from the first strip until the last covered strip
             *  @param log: Logger used for debugging */
            std::optional<uint8_t> tdcChannel(uint8_t strip, MsgStream& log) const;
            /** @brief Returns the strip channel connected to a strip. If the tdc is not connected, 
             *         a nullopt is returned 
             *  @param strip: Strip number. It should range from the first strip until the last covered strip
             *  @param log: Logger used for debugging */
            std::optional<uint8_t> stripChannel(uint8_t tdcChannel, MsgStream& log) const;
            /** @brief Connect the strip with a tdc channel. If the channel has been connected
              *        before, false is returned.
              * @param strip: Strip number on the chamber end
              * @param tdc: Tdc channel on the chip end
             *  @param log: Logger used for debugging */
            bool mapChannels(uint8_t strip, uint8_t tdc, MsgStream& log);
            /** @brief Returns underlying strip mapping storage */ 
            const Storage_t& stripMap() const;
            /** @brief Returns the underlying tdc mapping storage */
            const Storage_t& tdcMap() const;
            /** @brief Number of connected channels */
            uint8_t connectedChannels() const;
        private:
            Storage_t m_stripToTdc{make_array<uint8_t, readStrips>(notSet)};
            Storage_t m_tdcToStrip{make_array<uint8_t, readStrips>(notSet)};
            uint8_t m_id{0};
            /** Number of channels that are connected  to the cable */
            uint8_t m_nCh{0};
    };
    std::ostream& operator<<(std::ostream& ostr, const RpcFlatCableTranslator& translator);
}
#endif