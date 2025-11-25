/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 *   */

#ifndef POOL_ROOTTREEINDEXCONTAINER_H
#define POOL_ROOTTREEINDEXCONTAINER_H 1

#include "RootTreeContainer.h"

#include <cstdint>

// Forward declarations
class TTree;
class TBranch;

namespace pool {
  /** @class RootTreeIndexContainer RootTreeIndexContainer.h src/RootTreeIndexContainer.h
    *
    * Description:
    * Spezialization of RootTreeContainer
    * ROOT specific implementation of Database container.
    * Since objects in root are stored in "trees with branches",
    * this object corresponds to a tuple (Tree/Branch), where
    * each object type (determined by the location of the transient
    * object within the data store) is accessed by an index number
    * stored inside its tree.
    *
    * @author  N.Dulin, P.v.Gemmeren
    * @date    8/1/2018
    * @version 1.0
    */

   class RootTreeIndexContainer : public RootTreeContainer {
   public:
      explicit RootTreeIndexContainer(const std::string& name);
      RootTreeIndexContainer (const RootTreeIndexContainer&) = delete;
      RootTreeIndexContainer& operator= (const RootTreeIndexContainer&) = delete;

      virtual ~RootTreeIndexContainer() override {}

      /// Open the container
      virtual DbStatus open(DbDatabase&, const std::string&, const DbTypeInfo*, DbAccessMode) override final;

      /// Number of entries within the container
      virtual uint64_t nextRecordId() override final;

      /// Suggest next Record ID for tbe next object written - for synced indexes
      virtual void useNextRecordId( uint64_t ) override final;

      /// Find object by object identifier and load it into memory
      /** @param  ptr    [IN/OUT]  ROOT-style address of the pointer to object
        * @param  shape     [IN]   Object type
        * @param  oid      [OUT]   Object OID
        *
        * @return Status code indicating success or failure.
        */
      virtual DbStatus loadObject( void** ptr, ShapeH shape, 
                                   Token::OID_t& oid) override;
      
      /// Commit single entry to container
      virtual DbStatus writeObject(ActionList::value_type&) override;

   private:
      /// Pointer to index branch
      TBranch*          m_indexBranch;

      uint64_t          m_index_entries;

      /// Index multiplier (e.g. pid - ppid), fill in c'tor
      const uint32_t    m_index_multi;

      /// Index (64 bit) - using ROOT type that matches the earlier 'long long'
      Long64_t          m_index;

      /// How much to increase nextRecordID to keep index values synced with other containers
      int64_t           m_indexBump;

      /// first object retrieval attempt flag (to rebuild index)
      bool              m_firstRead;
   };
}
#endif //POOL_ROOTTREEINDEXCONTAINER_H
