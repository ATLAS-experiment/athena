/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ITkPixelCablingData_h
#define ITkPixelCablingData_h
/**
  * @file ITkPixelCablingData/ITkPixelCablingData.h
  * @author Shaun Roe
  * @date June 2024
  * @brief Data object containing the offline-online mapping for ITkPixels
  */

#include "ITkPixelCabling/ITkPixelOnlineId.h"

// Athena includes
#include "Identifier/Identifier.h"
#include "AthenaKernel/CLASS_DEF.h"
#include "AthenaKernel/CondCont.h"
//STL
#include <unordered_map>
#include <iosfwd>

class ITkPixelCablingData{
public:
  ///stream extraction to read value from stream into ITkPixelCablingData
  friend std::istream& operator>>(std::istream & is, ITkPixelCablingData & cabling);
  ///stream insertion for debugging
  friend std::ostream& operator<<(std::ostream & os, const ITkPixelCablingData & cabling);
  bool empty() const;
  std::size_t size() const;
  ITkPixelOnlineId onlineId(const Identifier & id) const;
  
private:
  std::unordered_map<Identifier, ITkPixelOnlineId> m_offline2OnlineMap;
};
// Magic "CLassID" for storage/retrieval in StoreGate
// These values produced using clid script.
// "clid ITkPixelCablingData"
// 140860927 ITkPixelCablingData
CLASS_DEF( ITkPixelCablingData , 140860927 , 1 )
//"clid -cs ITkPixelCablingData"
//143807283
CONDCONT_DEF( ITkPixelCablingData , 143807283 );
#endif
