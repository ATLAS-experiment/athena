/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ITkStripCablingData_h
#define ITkStripCablingData_h
/**
  * @file ITkStripCablingData/ITkStripCablingData.h
  * @author Edson Carquin
  * @date September 2024
  * @brief Data object containing the offline-online mapping for ITkStrips (based on ITkPixelCabling package)
  */

#include "ITkStripCabling/ITkStripOnlineId.h"

// Athena includes
#include "Identifier/IdentifierHash.h"
#include "Identifier/Identifier.h"
#include "AthenaKernel/CLASS_DEF.h"
#include "AthenaKernel/CondCont.h"
//STL
#include <unordered_map>
#include <iosfwd>

class ITkStripCablingData{
public:
  ///stream extraction to read value from stream into ITkStripCablingData
  friend std::istream& operator>>(std::istream & is, ITkStripCablingData & cabling);
  ///stream insertion for debugging
  friend std::ostream& operator<<(std::ostream & os, const ITkStripCablingData & cabling);
  bool empty() const;
  std::size_t size() const;
  ITkStripOnlineId onlineId(const Identifier & id) const;

  /// Get a vector of all RODs
  void getRods(std::vector<std::uint32_t>& usersVector) const;

  /// Get ITkStripOnlineId from IdentifierHash
  ITkStripOnlineId getOnlineIdFromHash(const IdentifierHash& hash) const;

  enum {NUMBER_OF_HASHES=49536}; // In ITk, we have 49536 sensors.
  
private:
  std::unordered_map<Identifier, ITkStripOnlineId> m_offline2OnlineMap;
  std::set<std::uint32_t> m_rodIdSet; //!< Set of robIds
  std::array<ITkStripOnlineId, NUMBER_OF_HASHES> m_hash2OnlineIdArray; //!< Array for hash to onlineId; hash goes from 0-49536

  static const IdentifierHash s_invalidHash; //!< Invalid IdentifierHash
  static const ITkStripOnlineId s_invalidId; //!< Invalid SCT_OnlineId  
  
};
// Magic "CLassID" for storage/retrieval in StoreGate
// These values produced using clid script.
// "clid ITkStripCablingData"
// 92425761 ITkStripCablingData
CLASS_DEF( ITkStripCablingData , 92425761 , 1 )
//"clid -cs ITkStripCablingData"
// 133529577
CONDCONT_DEF( ITkStripCablingData , 133529577 );
#endif
