/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUON_CABLING_NRPCCABLINGDATA_H
#define MUON_CABLING_NRPCCABLINGDATA_H

#include <cstdint>
#include <iostream>
#include <set>
/*
 * @brief: Helper struct containing all information to convert from the offline
 * identifiers to the online identifiers
 */

/// Helper macro to define the constructors and comparison operators of the 
/// Online & Offline Identifiers of the cabling
#define CABLING_OPERATORS(CL_NAME) \
      bool operator<(const CL_NAME& other)  const { return m_cache.hash < other.m_cache.hash; } \
      bool operator==(const CL_NAME& other) const {return m_cache.hash == other.m_cache.hash; } \
      bool operator!=(const CL_NAME& other) const { return m_cache.hash != other.m_cache.hash; } \
      bool operator!() const {return !m_cache.hash;} \
      \
      CL_NAME() = default; \
      CL_NAME( const CL_NAME& other): CL_NAME() { \
         m_cache.hash = other.m_cache.hash; \
      } \
      CL_NAME& operator=(const CL_NAME& other) { \
            if (&other != this) m_cache.hash = other.m_cache.hash; \
            return *this; \
      }

/// @brief  Representation of the offline Identifier in a more tangible format 
struct NrpcCablingOfflineID {
    CABLING_OPERATORS(NrpcCablingOfflineID)
    
    int8_t& stationIndex{m_cache.cache[0]};  /// Station of the chamber (i.e, BIL,BIS,etc.)
    int8_t& eta{m_cache.cache[1]};           /// Eta of the RPC station
    int8_t& phi{m_cache.cache[2]};           /// Phi sector of the RPC station
    int8_t& doubletR{m_cache.cache[3]};      /// doublet R -> 1,2

    int8_t& doubletPhi{m_cache.cache[4]};  /// doublet Phi -> 1,2
    int8_t& doubletZ{m_cache.cache[5]};    /// doublet Z -> 1,2
    int8_t& gasGap{m_cache.cache[6]};      /// gas gap -> 1-3
    /** The BIL-RPC chambers have two strip-layers, both oriented to measure the eta coordinate.
     *  From a readout perspective, the strips share the same channel, despite that their readout 
     *  positions are at opposite sides. In order to disentangle the particular sites of the strips,
     *  the second bit of the measPhi field is set.
     */
     static constexpr int8_t measPhiBit = 1 << 0; 
     static constexpr int8_t stripSideBit = 1 << 1;
 
     /** @brief: Does the channel measure phi */
     bool measuresPhi() const { return m_measPhiStrip & measPhiBit; }
     /** @brief: Is the strip readout on the opposite side  */ 
     bool stripSide() const { return m_measPhiStrip & stripSideBit; }
     /** Sets the measuresPhi & stripSide fields of the cabling data object
      * 
      */
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

/// @brief Struct summarizing all the Identifier fields to uniquely Identify a Nrpc TDC chip
struct NrpcCablingOnlineID {
    CABLING_OPERATORS(NrpcCablingOnlineID)
    /// Identifier of the subdetector region in the readout BA / BC etc.
    int16_t& subDetector{m_cache.cache[0]};
    /// Unique Identifier of the Rpc chamber from an online perspective
    int16_t& tdcSector{m_cache.cache[1]};
    /// TDC chip
    int16_t& tdc{m_cache.cache[2]};
   private:
    union {
        int64_t hash{0};
        int16_t cache[4];
    } m_cache{};
};

/// Depending on whether the cabling map shall convert offline -> online 
///           or online -> offline extra information is required
/// Stage Offline -> Online: 
///       Range of channels covered by the chip
/// Stage Online -> Offline:
///       Range of strips covered by the chip
struct NrpcTdcStripRange{
    public:
       /// @brief  First strip covered by the Tdc chip
       uint8_t firstStrip{0};
       /// @brief  Last strip covered by the Tdc chip (inclusive)
       uint8_t lastStrip{0};
       /// Sorting operator. Needed to build consistent sets
       bool operator<(const NrpcTdcStripRange& other ) const{
          return lastStrip < other.firstStrip;
       }       
 };

/// @brief Covered channels by the Online Identifier Object
struct NrpcTdcChannelRange {
    public:
        /// @brief  First tdc channel
        uint8_t firstChannel{0};
        /// @brief Last tdc channel (inclusive)
        uint8_t lastChannel{0};    
        /// Sorting operator. Needed to build consistent sets
        bool operator<(const NrpcTdcChannelRange& other) const{
          return lastChannel < other.firstChannel;
       }
};

/// @brief  Cabling information shipped around the Digi <-> Rdo conversions
struct NrpcCablingData : public NrpcCablingOfflineID, NrpcCablingOnlineID {
    NrpcCablingData() = default;
    uint8_t strip{0};      // Offline strip number
    uint8_t channelId{0};  // Online tdc channel number

    /// Deactivate the sorting operator here
    bool operator<(const NrpcCablingData&) const {return false; }
    /// Equality operator of all Identifier fields
    bool operator==(const NrpcCablingData& other) const {
        return strip == other.strip && channelId == other.channelId &&
            static_cast<const NrpcCablingOfflineID&>(*this) == other &&
            static_cast<const NrpcCablingOnlineID&>(*this) == other;
    }
    bool operator!=(NrpcCablingData& other) const {return ! ((*this) == other);}
};

/// @brief Struct to represent all cabling information coming from the cabling
///        database per channel
struct NrpcCablingCoolData: public NrpcCablingOfflineID, NrpcCablingOnlineID,
                                   NrpcTdcStripRange, NrpcTdcChannelRange {
        NrpcCablingCoolData() = default;
        /// Delete the smaller operator
        bool operator<(const NrpcCablingCoolData&) const { return false;}
};
/// @brief Struct to build the offline -> online map
struct NrpcCablOnDataByStrip: public NrpcTdcStripRange, NrpcTdcChannelRange, NrpcCablingOnlineID {
    NrpcCablOnDataByStrip() = default;
    NrpcCablOnDataByStrip(const NrpcCablingCoolData& data):
        NrpcTdcStripRange(data), 
        NrpcTdcChannelRange(data), 
        NrpcCablingOnlineID(data) {}
    
    /// Equality operator of all Identifier fields
    bool operator<(const NrpcCablOnDataByStrip& other) const{
        if (static_cast<const NrpcCablingOnlineID&>(*this) != other) {
            return static_cast<const NrpcCablingOnlineID&>(*this) < other;
        }
        return static_cast<const NrpcTdcStripRange&>(*this) < other;
    }
};
///@brief Struct to build the online -> offline map
struct NrpcCablOnDataByTdc : public NrpcTdcStripRange, NrpcTdcChannelRange, NrpcCablingOnlineID {
    NrpcCablOnDataByTdc() = default;
    NrpcCablOnDataByTdc(const NrpcCablingCoolData& data):
        NrpcTdcStripRange(data),
        NrpcTdcChannelRange(data),
        NrpcCablingOnlineID(data) {}
    
    /// Equality operator of all Identifier fields
    bool operator<(const NrpcCablOnDataByTdc& other) const{
        if (static_cast<const NrpcCablingOnlineID&>(*this) != other) {
            return static_cast<const NrpcCablingOnlineID&>(*this) < other;
        }
        return static_cast<const NrpcTdcChannelRange&>(*this) < other;
    }
};

/// Let's define the operator needed to read the cabling sets
inline bool operator<(const NrpcCablOnDataByStrip& a,const NrpcCablingData& b) {
    return a.lastStrip < b.strip;
}
inline bool operator<(const NrpcCablingData& a, const NrpcCablOnDataByStrip& b) {
    return a.strip < b.firstStrip;
}
inline bool operator<(const NrpcCablOnDataByStrip& a, const  NrpcCablingCoolData& b) {
     if (static_cast<const NrpcCablingOnlineID&>(a) != static_cast<const NrpcCablingOnlineID&>(b)) {
         return static_cast<const NrpcCablingOnlineID&>(a) < b;
     }
     return static_cast<const NrpcTdcStripRange&>(a) < b;
}
inline bool operator<(const  NrpcCablingCoolData& a, const NrpcCablOnDataByStrip& b) {
     if (static_cast<const NrpcCablingOnlineID&>(a) != b) return static_cast<const NrpcCablingOnlineID&>(a) < b;
     return static_cast<const NrpcTdcStripRange&>(a) < b;
}

inline bool operator<(const NrpcCablOnDataByTdc& a, const NrpcCablingData& b) {
    if (static_cast<const NrpcCablingOnlineID&>(a) != b) return static_cast<const NrpcCablingOnlineID&>(a) < b;
    return a.lastChannel < b.channelId;
}
inline bool operator<(const NrpcCablingData& a, const NrpcCablOnDataByTdc& b) {
    if (static_cast<const NrpcCablingOnlineID&>(a) != b) return static_cast<const NrpcCablingOnlineID&>(a) < b;
    return a.channelId < b.firstChannel;
}
using NrpcCablOnDataByStripSet = std::set<NrpcCablOnDataByStrip, std::less<>>;


/// @brief  Outstream operators
std::ostream& operator<<(std::ostream& ostr, const NrpcCablingOfflineID& obj);
std::ostream& operator<<(std::ostream& ostr, const NrpcCablingOnlineID& obj);
std::ostream& operator<<(std::ostream& ostr, const NrpcTdcStripRange& obj);
std::ostream& operator<<(std::ostream& ostr, const NrpcTdcChannelRange& obj);
std::ostream& operator<<(std::ostream& ostr, const NrpcCablingData& obj);
std::ostream& operator<<(std::ostream& ostr, const NrpcCablingCoolData& obj);
std::ostream& operator<<(std::ostream& ostr, const NrpcCablOnDataByTdc& obj);
std::ostream& operator<<(std::ostream& ostr, const NrpcCablOnDataByStrip& obj);

#undef CABLING_OPERATORS
#endif
