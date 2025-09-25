/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ITkPixelOnlineId_h
#define ITkPixelOnlineId_h
/**
  * @file ITkPixelCablingData/ITkPixelOnlineId.h
  * @author Shaun Roe
  * @date June 2024
  * @brief Online Id for ITkPixels
  */
#include <cstdint>
#include <iosfwd>
#include <compare>
#include <utility>
#include <typeindex> //provides std::hash

class ITkPixelOnlineId{
public:
  ///representation for debugging, messages
  friend std::ostream& operator<<(std::ostream & os, const ITkPixelOnlineId & id);
  /// Default constructor produces an invalid serial number
  ITkPixelOnlineId() = default;
  /// Construct from uint32
  ITkPixelOnlineId(const std::uint32_t onlineId);
  /// Construct from robId and fibre; a cursory check is made on validity of the input
  ITkPixelOnlineId(const std::uint32_t rodId, const std::uint32_t fibre);
  /// Return the rod/rob Id
  std::uint32_t rod() const;
  /// Return the fibre
  std::uint32_t fibre() const;
  /// Overload cast to uint
  explicit operator unsigned int() const {return m_onlineId;}
  /// Equality etc.
  auto operator<=>(const ITkPixelOnlineId & other) const = default;
  
  bool isValid() const;
  
  enum {
    INVALID_FIBRE=255, INVALID_ROD=16777215, INVALID_ONLINE_ID=0xFFFFFFFF
  };
private:
  std::uint32_t m_onlineId{INVALID_ONLINE_ID};

};

namespace std {
  template<>
  struct hash<ITkPixelOnlineId>{
    size_t operator()(const ITkPixelOnlineId& id) const{
      return static_cast<size_t>((unsigned int)id);
    }
  };
}
  
  

#endif
