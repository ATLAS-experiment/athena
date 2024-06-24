/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonRDO/TgcRdoIdHash.h" 
#include "MuonRDO/TgcRdo.h" 

// default contructor 
TgcRdoIdHash::TgcRdoIdHash() {
  m_size=0;

  // loop over all RODs
  for (uint16_t id=0; id<30; ++id){
    if (id < 24) {
      // map
      m_lookup[id]=m_size;
      m_int2id.push_back(id);
      ++m_size;

      // SubDetectorID
      if (id < 12) // A-side
        m_int2subDetectorId.push_back(0x67);
      else
        m_int2subDetectorId.push_back(0x68);

      // ROD ID
      m_int2rodId.push_back( (id % 12) + 1);
    } else {  // SROD
      // map
      m_lookup[id]=m_size;
      m_int2id.push_back(id);
      ++m_size;

      // SubDetectorID
      if (id < 27) // A-side
        m_int2subDetectorId.push_back(0x67);
      else
        m_int2subDetectorId.push_back(0x68);

      // SROD ID
      m_int2rodId.push_back( (id - 24) % 3 + 17 ); // 17-19
    }
  }
}


/** reverse conversion */
TgcRdoIdHash::ID TgcRdoIdHash::identifier(int index) const
{
  if (index>=0 && index < m_size)
    return m_int2id[index]; 
  
  // if invalid index 
  return INVALID_ID; 
}


/** reverse conversion : SubDetectorID */
uint16_t TgcRdoIdHash::subDetectorId(int index) const
{
  if (index>=0 && index < m_size)
    return m_int2subDetectorId[index]; 
  
  // if invalid index 
  return INVALID_ID; 
}


/** reverse conversion : ROD ID */
uint16_t TgcRdoIdHash::rodId(int index) const
{
  if (index>=0 && index < m_size)
    return m_int2rodId[index]; 
  
  // if invalid index 
  return INVALID_ID; 
}


/**Convert ID to int. return -1 if invalid ID */
int TgcRdoIdHash::operator() (const ID& id) const 
{
  std::map<ID,int>::const_iterator it = m_lookup.find(id); 
  if(it!=m_lookup.end())
    return (*it).second; 

 // if invalid ID
 return INVALID_ID; 
}
