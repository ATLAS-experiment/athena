/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ITkPixelOnlineId_h
#define ITkPixelOnlineId_h
/**
  * @file ITkPixelCablingData/ITkPixelOnlineId.h
  * @author Shaun Roe, Ondra Kovanda
  * @date June 2024
  * @brief Online Id for ITkPixels
  */
#include <cstdint>
#include <iosfwd>
#include <compare>
#include <utility>
#include <typeindex> //provides std::hash

// ***************************************
// The 'online' ID is 64-bits, encoding
// 
// sourceID [32b] | DetectorResourceID [32b]
//
// sourceID is also known as ROD/ROB ID, calculated as
//
// 0 [8b] | subdetector ID [8b] | FELIX host [8b] | 0b00 | FELIX card [1b] | FELIX device [1b] | 0b00 | DMA buffer number [2b]
//
// DetectorResourceID is ITk-specific, and constructed as
//
// chipID [2b] | 0b00 | chipID on/off [1b] | BCID on/off [1b] | 0b00 | module ID [24b]

namespace ITkPixelCabling {

    //mask for the source ID part
    static constexpr uint64_t SOURCE_ID_MASK = 0xFFFFFFFF00000000;

    //mask for the DetectorResourceID part
    static constexpr uint64_t DRID_MASK = 0x00000000FFFFFFFF;

    //mask for the moduleID part of the full DetectorResourceID and the chipID, needed in encoding
    static constexpr uint32_t OFFLINE_DRID_MASK = 0xC0FFFFFF;
    
    //mask for the upper 8 bits of the full DetectorResourceID, which encodes
    //chipID [31:30], unused [29:28], chip ID on/off [27] and BCID on/off [26], unused [25]
    static constexpr uint32_t ONLINE_DRID_MASK = 0xFF000000;

    //mask for the module ID part
    static constexpr uint32_t MODULE_DRID_MASK = 0x00FFFFFF;

    //mask for the chipID part
    static constexpr uint32_t CHIP_DRID_MASK = 0xC0000000;

    //DRID to module ID
    inline uint32_t dridToModuleID(const uint32_t& drid){ return (drid & MODULE_DRID_MASK) << 8;}

    //DRID to chip ID
    inline uint8_t dridToChipID(const uint32_t& drid){ return static_cast<uint8_t>((drid & CHIP_DRID_MASK) >> 30);}

}

class ITkPixelOnlineId{
public:
  ///representation for debugging, messages
  friend std::ostream& operator<<(std::ostream & os, const ITkPixelOnlineId & id);
  /// Default constructor produces an invalid serial number
  ITkPixelOnlineId() = default;
  /// Construct from uint32
  ITkPixelOnlineId(const std::uint64_t onlineId);
  /// Construct from robId and detectorResourceID; a cursory check is made on validity of the input
  ITkPixelOnlineId(const std::uint32_t rodId, const std::uint32_t detectorResourceID);
  /// Return the rod/rob Id
  std::uint32_t sourceID() const;
  /// Return the detectorResourceID
  std::uint32_t detectorResourceID() const;
  /// Return the offline part of detectorResourceID
  std::uint32_t offlineDetectorResourceID() const;
  /// Return the offline module ID part
  std::uint32_t offlineModuleID() const;
  /// Overload cast to uint
  explicit operator unsigned long int() const {return m_onlineId;}
  /// Equality etc.
  auto operator<=>(const ITkPixelOnlineId & other) const = default;
  
  bool isValid() const;
  
  enum {
    INVALID_DETECTORRESOURCE_ID=0xFFFFFFFF, INVALID_SOURCE_ID=0xFFFFFFFF, INVALID_ONLINE_ID=0xFFFFFFFFFFFFFFFF
  };

private:
  std::uint64_t m_onlineId{INVALID_ONLINE_ID};

};

namespace std {
  template<>
  struct hash<ITkPixelOnlineId>{
    size_t operator()(const ITkPixelOnlineId& id) const{
      return static_cast<size_t>((uint64_t)id);
    }
  };
}
  
  

#endif
