/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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
  
private:
  std::unordered_map<Identifier, ITkStripOnlineId> m_offline2OnlineMap;
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
