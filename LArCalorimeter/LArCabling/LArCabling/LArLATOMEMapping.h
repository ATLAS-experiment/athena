//Dear emacs, this is -*-c++-*-

/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARLATOMEMAPPING_H
#define LARLATOMEMAPPING_H

#include "Identifier/HWIdentifier.h"
#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"

#include <vector>
//#include <algorithm>
#include <unordered_map>
#include "CxxUtils/AthUnlikelyMacros.h"
/**
  * fixed number of channels on one LATOME board<br>
  *
  */
#define N_LATOME_CHANNELS 320

/**
 * @brief class to provide SC mapping
 */

class LArLATOMEMapping {

  /**
   * class which fills object from conditions DB<br>
   *
   */
  friend class LArLATOMEMappingAlg; //The conditions alg filling this object

 public:
  LArLATOMEMapping(unsigned nLatomes=0);
  
  /**
   * create a HWIdentifier from an Identifier <br>
   *
   */
  HWIdentifier getChannelID(const unsigned int sourceID, const unsigned int chan) const;



 private:

  ///Invalid default instance of HWIdentifier
  const HWIdentifier m_hwidEmpty;

  std::vector<unsigned int>  m_sourceID;
  std::vector<std::vector< HWIdentifier> > m_onlineID;
  std::unordered_map<unsigned,std::vector< HWIdentifier> > m_map;

};


//Inline methods:
inline HWIdentifier LArLATOMEMapping::getChannelID(const unsigned int sourceID, const unsigned int chan) const {
  auto it = m_map.find(sourceID);
  if (ATH_UNLIKELY(it == m_map.end())) {
    return m_hwidEmpty;
  }
  const std::vector<HWIdentifier>& chanVec = it->second;
  if (ATH_UNLIKELY(chan >= chanVec.size())) {
    return m_hwidEmpty;
  }
  return chanVec[chan];
}

#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF( LArLATOMEMapping, 219977637 , 1)
#include "AthenaKernel/CondCont.h"
CONDCONT_DEF(  LArLATOMEMapping, 84043487  );

#endif
