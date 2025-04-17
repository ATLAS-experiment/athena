/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ROOTAUXDYNSTORE_H
#define ROOTAUXDYNSTORE_H

#include "AthContainers/AuxStoreInternal.h" 
#include "RootAuxDynReader.h" 

#include <string>
#include <mutex>

class RootAuxDynReader;

class RootAuxDynStore : public SG::AuxStoreInternal
{
public:
   RootAuxDynStore(RootAuxDynReader& reader, long long entry,
                   bool standalone, std::recursive_mutex* iomtx = nullptr);
  
  virtual ~RootAuxDynStore() {}

  /// implementation of the IAuxStore interface
  virtual const void*                getData(SG::auxid_t auxid) const override;
  virtual const SG::IAuxTypeVector*  getVector (SG::auxid_t auxid) const override;
  virtual void*                      getData(SG::auxid_t auxid, size_t size, size_t capacity) override;

  ///  implementation of the IAuxStoreIO interface
  virtual const void*                getIOData(SG::auxid_t auxid) const override;


  /**
   * @brief Return the data vector for one aux data decoration item.
   * @param auxid The identifier of the desired aux data item.
   * @param size The current size of the container (in case the data item
   *             does not already exist).
   * @param capacity The current capacity of the container (in case
   *                 the data item does not already exist).
   */
  virtual void* getDecoration (SG::auxid_t auxid, size_t size, size_t capacity) override;


  /**
   * @brief Test if a particular variable is tagged as a decoration.
   * @param auxid The identifier of the desired aux data item.
   */
  virtual bool isDecoration (SG::auxid_t auxid) const override;

  /**
   * @brief Return the number of elements in the store.
   *
   * May return 0 for a store with no aux data.
   */
  virtual size_t size() const override;

     
protected:
  /// read data from ROOT and store it in m_vecs. Returns False on error
  virtual bool readData(SG::auxid_t auxid) = 0;
     
protected:
  long long          m_entry;

  /// Mutex used to synchronize modifications to the cache vector.
  typedef AthContainers_detail::mutex mutex_t;
  typedef AthContainers_detail::lock_guard<mutex_t> guard_t;
  mutable mutex_t m_mutex;

  /// Optional mutex used to serialize I/O with RootStorageSvc (not owned)
  std::recursive_mutex       *m_iomutex;
};

#endif
