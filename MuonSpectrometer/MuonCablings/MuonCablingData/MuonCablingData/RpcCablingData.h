/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONCABLINGDATA_RPCCABLINGDATA_H
#define MUONCABLINGDATA_RPCCABLINGDATA_H


#include <cstdint>
#include <iostream>
#include <set>
/** @brief:  Helper structs to convert the Identifier of a Rpc measurement from its offline representation
 *           into its online equivalent. Unlike, for the other detectors, the offline fields do correpond to
 *           a one by one translation of the online fields. As readout cards may be shared between two different
 *           readout elements. */
/// Helper macro to define the constructors and comparison operators of the 
/// Online & Offline Identifiers of the cabling
#define CABLING_OPERATORS(CL_NAME) \
      bool operator<(const CL_NAME& other)  const { return m_cache.hash < other.m_cache.hash; } \
      bool operator==(const CL_NAME& other) const {return m_cache.hash == other.m_cache.hash; } \
      bool operator!=(const CL_NAME& other) const { return m_cache.hash != other.m_cache.hash; } \
      bool operator!() const {return !m_cache.hash;} \
      \
      CL_NAME() = default; \
      CL_NAME(const CL_NAME& other): CL_NAME{} { \
         m_cache.hash = other.m_cache.hash; \
      } \
      CL_NAME& operator=(const CL_NAME& other) { \
            if (&other != this) m_cache.hash = other.m_cache.hash; \
            return *this; \
      }

namespace Muon{
    /** @brief  Struct to represent the offline identifier of the Rpc measurement decomposed into the 
     *          particular fields of the layer Identifier:
     *              - StationName      - doubletR        - gasGap
     *              - StationEta       - doubletZ        - measuresPhi
     *              - StationPhi       - doubletPhi
     *          while the internal representation of the data is compressed into a single 64-bit integer. **/
    struct RpcCablingOfflineID {
         // cppcheck-suppress operatorEqVarError; stationIndex, etc intentionally not copied.
        CABLING_OPERATORS(RpcCablingOfflineID)
        
        int8_t& stationIndex{m_cache.cache[0]};  /// Station of the chamber (i.e, BIL,BIS,etc.)
        int8_t& eta{m_cache.cache[1]};           /// Eta index of the RPC station
        int8_t& phi{m_cache.cache[2]};           /// Station phi of the RPC station
        int8_t& doubletR{m_cache.cache[3]};      /// doublet R -> 1,2

        int8_t& doubletPhi{m_cache.cache[4]};  /// doublet Phi -> 1,2
        int8_t& doubletZ{m_cache.cache[5]};    /// doublet Z -> 1,2
        int8_t& gasGap{m_cache.cache[6]};      /// gas gap -> 1-3
        /** The BIL-RPC chambers have two strip-layers, both oriented to measure the eta coordinate.
         *  From a readout perspective, the strips share the same channel, despite that their readout 
         *  positions are on opposite sides. In order to disentangle the particular sites of the strips,
         *  the second bit of the measPhi field is set. */
        static constexpr int8_t measPhiBit = 1 << 0; 
        static constexpr int8_t stripSideBit = 1 << 1;
    
        /** @brief: Does the channel measure phi */
        bool measuresPhi() const { return m_measPhiStrip & measPhiBit; }
        /** @brief: Is the strip readout on the opposite side  */ 
        bool stripSide() const { return m_measPhiStrip & stripSideBit; }
        /** Sets the measuresPhi & stripSide fields of the cabling data object */
        void setMeasPhiAndSide(bool measPhi, bool stripSide) {
            m_measPhiStrip = measPhi * measPhiBit | stripSideBit * stripSide;
        }
    private:
            union {
                long int hash{0};
                int8_t cache[8];
            } m_cache{};
            int8_t& m_measPhiStrip{m_cache.cache[7]};     /// measures phi -> 0,1

    };

    /** @brief In an analogous way to the `RpcCablingOfflineID`, the `RpcCablingOnlineID`, holds all information
     *         needed to navigate to a particular RPC gasGap from the readout-side. The particular fields are 
     *         exhibited to the user while the internal storeage is compressed into a single 64 bit integer. */
    struct RpcCablingOnlineID {
         // cppcheck-suppress operatorEqVarError; subDetector, etc intentionally not copied.
        CABLING_OPERATORS(RpcCablingOnlineID)
        /** @brief  Identifier of the subdetector region in the readout BA / BC etc. */
        int16_t& subDetector{m_cache.cache[0]};
        /** @brief Unique Identifier of the Rpc chamber from an online perspective  */
        int16_t& boardSector{m_cache.cache[1]};
        /** @brief Unique identifier of the tdc chip */
        int16_t& board{m_cache.cache[2]};
    private:
        union {
            int64_t hash{0};
            int16_t cache[4];
        } m_cache{};
    };

    /** @brief Helper struct that is parsed to the cabling map to translate 
     *         between the offline & online Identifiers */
    struct RpcCablingData : public RpcCablingOfflineID, RpcCablingOnlineID {
        /** @brief Default constructor */
        RpcCablingData() = default;
        /** @brief Offline strip number */
        uint8_t strip{0};
        /** @brief Online board channel number */
        uint8_t channelId{0};
        /** @brief No sorting operator */
        bool operator<(const RpcCablingData&) const = delete;
        /** @brief Equality in terms of all offline & online fields are matching */
        bool operator==(const RpcCablingData& other) const {
            return strip == other.strip && channelId == other.channelId &&
                static_cast<const RpcCablingOfflineID&>(*this) == other &&
                static_cast<const RpcCablingOnlineID&>(*this) == other;
        }
        /** @brief Inequality operator */
        bool operator!=(RpcCablingData& other) const {
            return !((*this) == other);
        }
    };
    std::ostream& operator<<(std::ostream& ostr, const RpcCablingOfflineID& obj);
    std::ostream& operator<<(std::ostream& ostr, const RpcCablingOnlineID& obj);
    std::ostream& operator<<(std::ostream& ostr, const RpcCablingData& obj);

}

#undef CABLING_OPERATORS
#endif
