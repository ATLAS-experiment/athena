/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "AthContainers/exceptions.h"

#include "RootAuxDynStore.h"
#include "RootAuxDynIO/RootAuxDynIO.h"


RootAuxDynStore::RootAuxDynStore(RootAuxDynIO::IRootAuxDynReader& reader,
                                 long long entry, bool standalone, std::recursive_mutex* iomtx)
  : SG::AuxStoreInternal( standalone ),
    m_entry(entry),
    m_iomutex(iomtx)
{
   for( auto id : reader.auxIDs() ) {
      addAuxID(id);
   }
   lock();
}

      
const void* RootAuxDynStore::getData(SG::auxid_t auxid) const
{
  guard_t guard (m_mutex);
  // lock
  const void* ret = SG::AuxStoreInternal::getData (auxid);
  if (!ret) {
    auto this_nc ATLAS_THREAD_SAFE = const_cast<RootAuxDynStore*>(this); // locked above
    if( this_nc->readData(auxid) ) {
       ret = SG::AuxStoreInternal::getData(auxid);
    }
  }
  return ret;
}


void* RootAuxDynStore::getData(SG::auxid_t /*auxid*/, size_t /*size*/, size_t /*capacity*/)
{
  // MN: how do we add new attributes to this store?  A:for now we don't
  throw("Non-const access to RootAuxDynStore is not supported");
}


const void* RootAuxDynStore::getIOData(SG::auxid_t auxid) const
{
  guard_t guard (m_mutex);
  const void* ret = SG::AuxStoreInternal::getIODataInternal (auxid, true);
  if (!ret) {
    auto this_nc ATLAS_THREAD_SAFE = const_cast<RootAuxDynStore*>(this); // locked above
    if( this_nc->readData(auxid) ) {
       ret = SG::AuxStoreInternal::getIOData (auxid);
    }
  }
  return ret;
}

/**
 * @brief Return the data vector for one aux data decoration item.
 * @param auxid The identifier of the desired aux data item.
 * @param size The current size of the container (in case the data item
 *             does not already exist).
 * @param capacity The current capacity of the container (in case
 *                 the data item does not already exist).
 *
 * Each aux data item is stored as a vector, with one entry
 * per entry in the owning container.  This returns a pointer
 * to the start of the vector.
 *
 * The base class implementation works except for the case where we have
 * not yet created a vector for an item in the root file.  We need to 
 * detect that case and raise an exception.
 */
void*
RootAuxDynStore::getDecoration (SG::auxid_t auxid, size_t size, size_t capacity)
{
  guard_t guard (m_mutex);
  if (SG::AuxStoreInternal::getIODataInternal (auxid, true) == 0 &&
      SG::AuxStoreInternal::getAuxIDs().count (auxid) > 0)
  {
    throw SG::ExcStoreLocked (auxid);
  }
  return SG::AuxStoreInternal::getDecoration (auxid, size, capacity);
}


/**
 * @brief Test if a particular variable is tagged as a decoration.
 * @param auxid The identifier of the desired aux data item.
 *
 * See @c getDecoration() above.
 */
bool RootAuxDynStore::isDecoration (SG::auxid_t auxid) const
{
  guard_t guard( m_mutex );
  if (SG::AuxStoreInternal::getIODataInternal (auxid, true) == 0 &&
      SG::AuxStoreInternal::getAuxIDs().count (auxid) > 0)
  {
    return false;
  }
  return SG::AuxStoreInternal::isDecoration (auxid);
}

/**
 * @brief Return the number of elements in the store.
 * NOTE: this method will attempt to read data if size unknown (0)
 * May return 0 for a store with no aux data.
 */
size_t RootAuxDynStore::size() const
{
  const std::size_t s = SG::AuxStoreInternal::size();
  if( s != 0 ) {
    return s;
  }

  for( SG::auxid_t id : getAuxIDs() ) {
    if( getData( id ) != nullptr ) {
      return SG::AuxStoreInternal::size();
    }
  }

  return 0;
}
