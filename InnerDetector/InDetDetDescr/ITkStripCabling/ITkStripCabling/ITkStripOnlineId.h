/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ITkStripOnlineId_h
#define ITkStripOnlineId_h
/**
  * @file ITkStripCablingData/ITkStripOnlineId.h
  * @author Edson Carquin
  * @date September 2024
  * @brief Online Id for ITkStrips (based on ITkPixelCabling package)
  */
#include <cstdint>
#include <iosfwd>
#include <compare>

class ITkStripOnlineId{
public:
  ///representation for debugging, messages
  friend std::ostream& operator<<(std::ostream & os, const ITkStripOnlineId & id);
  /// Default constructor produces an invalid serial number
  ITkStripOnlineId() = default;
  /// Construct from uint32
  ITkStripOnlineId(const std::uint32_t onlineId);
  /// Construct from robId and fibre; a cursory check is made on validity of the input
  ITkStripOnlineId(const std::uint32_t rodId, const std::uint32_t fibre);
  /// Return the rod/rob Id
  std::uint32_t rod() const;
  /// Return the fibre
  std::uint32_t fibre() const;
  /// Overload cast to uint
  explicit operator unsigned int() const {return m_onlineId;}
  /// Equality etc.
  auto operator<=>(const ITkStripOnlineId & other) const = default;
  
  bool isValid() const;
  
  enum {
    INVALID_FIBRE=255, INVALID_ROD=16777215, INVALID_ONLINE_ID=0xFFFFFFFF
  };
private:
  std::uint32_t m_onlineId{INVALID_ONLINE_ID};

};

#endif
