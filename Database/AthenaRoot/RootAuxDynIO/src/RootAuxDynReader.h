/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ROOTAUXDYNREADER_H
#define ROOTAUXDYNREADER_H

#include "AthContainers/AuxStoreInternal.h" 
#include "RootAuxDynIO/IRootAuxDynIO.h" 

#include <set>

class RootAuxDynReader : virtual public RootAuxDynIO::IRootAuxDynReader
{
public :
   /// Aux IDs of all the Aux attributes belonging to the Aux container being read
   const SG::auxid_set_t& auxIDs() const;

   bool addAuxID(const SG::auxid_t& id);
   
   void addBytes(size_t bytes);

   size_t getBytesRead() const;

   void resetBytesRead();

   virtual ~RootAuxDynReader() = default;

protected:
   // auxids that could be found in registry for attribute names from the file
   SG::auxid_set_t                       m_auxids;
  
   // counter for bytes read
   size_t                                m_bytesRead = 0;
};



inline void
RootAuxDynReader::addBytes(size_t bytes) {
   m_bytesRead += bytes;
}

inline size_t
RootAuxDynReader::getBytesRead() const{
   return m_bytesRead;
}

inline void
RootAuxDynReader::resetBytesRead() {
   m_bytesRead = 0;
}

inline const SG::auxid_set_t&
RootAuxDynReader::auxIDs() const {
    return m_auxids;
}

inline bool
RootAuxDynReader::addAuxID(const SG::auxid_t& id) {
  if( id != SG::null_auxid ) {
     m_auxids.insert(id);
     return true;
  }
  return false;
}

#endif

