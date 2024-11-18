/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


#ifndef LARHVNMAP_H
#define LARHVNMAP_H

#include "Identifier/IdentifierHash.h"
#include "LArIdentifier/LArHVLineID.h"

#include <vector>

class LArHVNMap {
 
  public:

   LArHVNMap()=delete;
 
   LArHVNMap(std::vector<short>& vVec, const LArHVLineID* hvHelper);
   LArHVNMap(unsigned len, const LArHVLineID* hvHelper);
   ~LArHVNMap () {};


   // retrieving ncell using hvline ID  
   short HVNcell(const HWIdentifier& chid) const;

   short HVNcell_Hash(const IdentifierHash& h) const {
     if (h<m_hvNcell.size())  
       return m_hvNcell[h]; 
     else 
       return m_noCell;
   }

  const std::vector<short>& HVNcellVec() const {return m_hvNcell;}

  void incHVline(unsigned hvHash);


 private:
   const LArHVLineID* m_hvHelper;

   std::vector<short>       m_hvNcell;
   const short              m_noCell;
};

#include "AthenaKernel/CondCont.h"
CLASS_DEF( LArHVNMap, 18310106, 1)
CONDCONT_DEF( LArHVNMap, 38107992);

inline void LArHVNMap::incHVline(unsigned hvHash) {
   if(hvHash<m_hvNcell.size()) ++m_hvNcell[hvHash];
}

#endif
