/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "AthenaKernel/errorcheck.h"
#include "CSCcabling/CSCcablingSvc.h"
#include "MuonRDO/CscRawDataCollectionIdHash.h" 
#include "MuonRDO/CscRawDataCollection.h" 

// default contructor 
CscRawDataCollectionIdHash::CscRawDataCollectionIdHash()
{
  // initialize RPC cabling service
  ServiceHandle<CSCcablingSvc> cabling{"CSCcablingSvc", "CscRawDataCollectionIdHash"};
  if (cabling.retrieve().isFailure()) {
    REPORT_MESSAGE_WITH_CONTEXT(MSG::ERROR, "CscRawDataCollectionIdHash")
      << "Cannot get CSC cabling Service " << endmsg;
  }

  // loop over all RODs
  for (uint16_t id=0; id<cabling->maxId(); ++id)
    {
      // map
      m_lookup[id]=m_size;
      m_int2id.push_back(id);
      // ROD ID
      m_int2rodId.push_back( id%cabling->nROD() );
      ++m_size;

      // SubDetectorID
      if ( id < cabling->nROD() ) // C-side
        m_int2subDetectorId.push_back(0x6A);
      else                                      // A-side
        m_int2subDetectorId.push_back(0x69);
    }
}


/** reverse conversion */
CscRawDataCollectionIdHash::ID CscRawDataCollectionIdHash::identifier(int index) const
{
  if (index>=0 && index < m_size)
    return m_int2id[index]; 
  
  // if invalid index 
  return INVALID_ID; 
}


/** reverse conversion : SubDetectorID */
uint16_t CscRawDataCollectionIdHash::subDetectorId(int index) const
{
  if (index>=0 && index < m_size)
    return m_int2subDetectorId[index]; 
  
  // if invalid index 
  return INVALID_ID; 
}


/** reverse conversion : ROD ID */
uint16_t CscRawDataCollectionIdHash::rodId(unsigned int index) const
{
  //if (index>=0 && index < m_size)
  //  return m_int2rodId[index]; 
  
  if(index < m_int2rodId.size())
     return m_int2rodId.at(index);
  // if invalid index 
  return INVALID_ID; 
}


/**Convert ID to int. return -1 if invalid ID */
int CscRawDataCollectionIdHash::operator() (const ID& id) const 
{
  std::map<ID,int>::const_iterator it = m_lookup.find(id); 
  if(it!=m_lookup.end())
    return it->second; 

 // if invalid ID
 return INVALID_ID; 
}

