/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHENASERVICES_ALTDATABUCKET_H
#define ATHENASERVICES_ALTDATABUCKET_H

#include <cstdlib>  // std::abort
#include <memory>   // std::unique_ptr, std::make_unique
#include <string>   // std::string
#include <typeinfo> // std::type_info

// Framework include files
#include "AthenaKernel/DataBucketBase.h"
#include "AthenaKernel/IRegisterTransient.h"
#include "GaudiKernel/ClassID.h"
#include "SGTools/DataProxy.h"
#include "SGTools/TransientAddress.h"

namespace {
  /**
   * @brief Concrete DataBucket class for writing out an object
   *        as a base class.
   *
   * Normally, when an object is selected for writing via ItemList,
   * the object is written as its dynamic type (the type by which it was
   * originally recorded in StoreGate) rather than as the type written
   * in the ItemList.  This is because the selection works by forming
   * a list of @DataObject instances; once this list is formed, the
   * types used to select the list are no longer used.
   *
   * However, in some cases, it is useful to be able to write an object
   * as one of its base classes; for example, to write an auxiliary store
   * object as xAOD::AuxContainerBase, in order to get all variables saved
   * as dynamic variables.  This can be requested by adding a ! after
   * the type name in the ItemList, which sets the `exact' flag
   * in the SG::FolderItem.  To make this work, then, when we get
   * an item with the exact flag set, we need to construct a new DataObject
   * instance that holds the object as the requested type.  That is the
   * purpose of this class.
   */
  class AltDataBucket
    : public DataBucketBase
  {
  public:
    AltDataBucket (void* ptr, CLID clid, const std::type_info& tinfo,
                   const SG::DataProxy& proxy)
      : m_proxy(this, makeTransientAddress(clid, proxy).release()),
        m_ptr (ptr), m_clid (clid), m_tinfo (tinfo)
    {
      addRef();
    }
    // Extra constructor to create a temporary proxy without aliases, for objects in MetaContainers
    AltDataBucket(void* ptr, CLID clid, const std::type_info& tinfo, const std::string& name) :
       m_proxy(this, new SG::TransientAddress(clid, name) ),
       m_ptr(ptr), m_clid(clid), m_tinfo(tinfo)
    {
      addRef();
    }

    virtual const CLID& clID() const override { return m_clid; }
    virtual void* object() override { return m_ptr; }
    virtual const std::type_info& tinfo() const override { return m_tinfo; }
    using DataBucketBase::cast;
    virtual void* cast (CLID /*clid*/,
                        SG::IRegisterTransient* /*irt*/ = nullptr,
                        bool /*isConst*/ = true) override
    { std::abort(); }
    virtual void* cast (const std::type_info& tinfo,
                        SG::IRegisterTransient* /*irt*/ = nullptr,
                        bool /*isConst*/ = true) override
    { if (tinfo == m_tinfo)
        return m_ptr;
      return nullptr;
    }
    virtual void relinquish() override {}
    virtual void lock() override {}

    
  private:
    static
    std::unique_ptr<SG::TransientAddress>
    makeTransientAddress (CLID clid, const SG::DataProxy& oldProxy);

    SG::DataProxy m_proxy;
    void* m_ptr;
    CLID  m_clid;
    const std::type_info& m_tinfo;
  };


  std::unique_ptr<SG::TransientAddress>
  AltDataBucket::makeTransientAddress (CLID clid, const SG::DataProxy& oldProxy)
  {
    auto newTad = std::make_unique<SG::TransientAddress>
      (clid, oldProxy.name());
    newTad->setAlias (oldProxy.alias());
    for (CLID tclid : oldProxy.transientID()) {
      // Note: this will include derived CLIDs.
      // Strictly speaking, that's not right; however, filtering them
      // out can break ElementLinks (for example those used by
      // ShallowAuxContainer).  One will not actually be able to get
      // a pointer of the derived type, as the conversions in StorableConversion
      // only support derived->base, not the other way around.
      newTad->setTransientID (tclid);
    }
    return newTad;
  }

} // anonymous namespace

#endif
