/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


#include "SGTools/DataProxy.h"
#include "SGTools/DataProxy_cast.h"
#include "AthenaKernel/IResetable.h"
#include "AthenaKernel/getMessageSvc.h"

#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IConverter.h"
#include "GaudiKernel/GenericAddress.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/GaudiException.h"

#include "SGTools/TransientAddress.h"
#include "SGTools/T2pMap.h"
#include "SGTools/CurrentEventStore.h"
#include "AthenaKernel/DataBucketBase.h"
#include "AthenaKernel/IProxyDict.h"
#include "AthenaKernel/EventContextClid.h"
#include <algorithm> 
#include <cassert>
#include <stdexcept>

using SG::DataProxy;
using SG::TransientAddress;
using std::find;


namespace SG {
  typedef IProxyDict** getDataSourcePointerFunc_t (const std::string&);
  extern getDataSourcePointerFunc_t* getDataSourcePointerFunc;

  class DataProxyHolder
  {
  public:
    static void resetCachedSource();
  };
}

namespace {

class PushStore
{
public:
  PushStore (IProxyDict* store)
  {
    static std::string storeName = "StoreGateSvc";
    m_storePtr = (*SG::getDataSourcePointerFunc) (storeName);
    m_store = *m_storePtr;
    if (store && store != m_store) {
      *m_storePtr = store;
      SG::DataProxyHolder::resetCachedSource();
    }
  }

  ~PushStore()
  {
    if (*m_storePtr != m_store) {
      *m_storePtr  = m_store;
      SG::DataProxyHolder::resetCachedSource();
    }
  }

private:
  IProxyDict** m_storePtr;
  IProxyDict* m_store;
};

}


namespace {
  ///sets pMember to pgref (resetting it if pgref is 0). Handles Gaudi refcount
  template <class GAUDIREF>
  void setGaudiRef(GAUDIREF* pgref, GAUDIREF*& pMember) {
    if (0 != pgref) pgref->addRef();
    if (0 != pMember) pMember->release();
    pMember = pgref;
  }
  
  ///resets pMember. Handles Gaudi refcount
  template <class GAUDIREF>
  void resetGaudiRef(GAUDIREF*& pMember) { setGaudiRef((GAUDIREF*)0, pMember); }
  
} //end of unnamed namespace

// Default Constructor
DataProxy::DataProxy():
  m_refCount(0),
  m_resetFlag(true),
  m_boundHandles(false),
  m_const(false),
  m_origConst(false),
  m_dObject(nullptr), 
  m_dataLoader(nullptr),
  m_t2p(nullptr),
  m_store(nullptr),
  m_errno(ALLOK)
{ 
}

// DataProxy constructor with Transient Address
// (typically called from Proxy Provider)
DataProxy::DataProxy(TransientAddress* tAddr, 
		     IConverter* svc,
		     bool constFlag, bool resetOnly)
  : DataProxy (std::move(*tAddr),
               svc, constFlag, resetOnly)
{
  delete tAddr;
}

// DataProxy constructor with Transient Address
// (typically called from Proxy Provider)
DataProxy::DataProxy(std::unique_ptr<TransientAddress> tAddr, 
		     IConverter* svc,
		     bool constFlag, bool resetOnly)
  : DataProxy (std::move(*tAddr), svc, constFlag, resetOnly)
{
  //assert( tAddr->clID() != 0 );
  if (svc) svc->addRef();
}

DataProxy::DataProxy(TransientAddress&& tAddr, 
		     IConverter* svc,
		     bool constFlag, bool resetOnly):
  m_refCount(0),
  m_resetFlag(resetOnly),
  m_boundHandles(false),
  m_const(constFlag),
  m_origConst(constFlag),
  m_dObject(0), 
  m_tAddress(std::move(tAddr)),
  m_dataLoader(svc),
  m_t2p(nullptr),
  m_store(nullptr),
  m_errno(ALLOK)
{
  //assert( tAddr->clID() != 0 );
  if (svc) svc->addRef();
}

// with Data Object:
// (typically called from a StoreGate record
DataProxy::DataProxy(DataObject* dObject, 
		     TransientAddress* tAddr,
		     bool constFlag, bool resetOnly):
  m_refCount(0),
  m_resetFlag(resetOnly),
  m_boundHandles(false),
  m_const(constFlag),
  m_origConst(constFlag),
  m_dObject(0), 
  m_tAddress(std::move(*tAddr)),
  m_dataLoader(nullptr),
  m_t2p(nullptr),
  m_store(nullptr),
  m_errno(ALLOK)
{
  setObject(dObject);
  delete tAddr;
}

DataProxy::DataProxy(DataObject* dObject, 
		     TransientAddress&& tAddr,
		     bool constFlag, bool resetOnly):
  m_refCount(0),
  m_resetFlag(resetOnly),
  m_boundHandles(false),
  m_const(constFlag),
  m_origConst(constFlag),
  m_dObject(0), 
  m_tAddress(std::move(tAddr)),
  m_dataLoader(nullptr),
  m_t2p(nullptr),
  m_store(nullptr),
  m_errno(ALLOK)
{
  setObject(dObject);
}

// Destructor
DataProxy::~DataProxy()
{  
  finalReset();
}

void DataProxy::setT2p(T2pMap* t2p)
{
  lock_t lock (m_mutex);
  m_t2p = t2p;
}


/**
 * @brief Mark this object as const.  (Lock the object.)
 *
 * If the object held that derives from @c ILockable, then we also
 * call @c lock on the object.
 */
void DataProxy::setConst()
{
  objLock_t objLock (m_objMutex);
  lock_t lock (m_mutex);
  if (!m_const) {
    m_const = true;
    this->lock (objLock);
  }
}

bool DataProxy::bindHandle(IResetable* ir) {
  assert(ir);
  lock_t lock (m_mutex);
  if (ir->isSet()) {
    return false;
  } else {
    m_handles.push_back(ir);
    m_boundHandles = true;
    if (IProxyDict* store = m_store)
      store->boundHandle(ir);
    return true;
  }
}


/// Drop the reference to the data object.
void DataProxy::resetRef()
{
  DataObject* dobj = m_dObject;
  resetGaudiRef(dobj);
  m_dObject = dobj;
  m_tAddress.reset();
  m_const = m_origConst;
}


void DataProxy::reset (bool hard /*= false*/)
{
  resetBoundHandles (hard);

  objLock_t objLock (m_objMutex);
  lock_t lock (m_mutex);
  resetRef();
}


void DataProxy::finalReset()
{
  handleList_t handles;
  {
    objLock_t objLock (m_objMutex);
    lock_t lock (m_mutex);
    m_const=false; //hack to force the resetting of proxy ptr in VarHandleBase

    handles = m_handles;

    DataObject* dobj = m_dObject;
    resetGaudiRef(dobj);
    m_dObject = dobj;
    resetGaudiRef(m_dataLoader);

    if (m_handles.empty()) {
      m_boundHandles = false;
    }
  }

  for (auto ih: handles) {
    if (0 != ih) ih->finalReset();
  }
}

/// don't need no comment
void DataProxy::resetBoundHandles (bool hard) {
  handleList_t handles;
  {
    lock_t lock (m_mutex);
    // Early exit if the list is empty.
    if (!m_boundHandles) return;

    // Make a copy and drop the lock, so we're not holding the lock
    // during the callback.
    handles = m_handles;
  }

  for (IResetable* h : handles) {
    h->reset(hard);
  }
}

void DataProxy::unbindHandle(IResetable *ir) {
  assert(ir);
  lock_t lock (m_mutex);
  auto ifr = find(m_handles.begin(), m_handles.end(), ir );
  if (ifr != m_handles.end()) {
    m_handles.erase(ifr);
    if (IProxyDict* store = m_store)
      store->unboundHandle(ir);
  }
  m_boundHandles = !m_handles.empty();
}
  
/// return refCount
unsigned long DataProxy::refCount() const
{
  lock_t lock (m_mutex);
  return m_refCount;
}

/// Add reference to object
unsigned long DataProxy::addRef()
{ 
  lock_t lock (m_mutex);
  return ++m_refCount;
}

/// release reference to object
unsigned long DataProxy::release()
{
  unsigned long count;
  {
    lock_t lock (m_mutex);
    count = --m_refCount;
  }
  if ( 0 == count ) delete this;
  return count;
}


/**
 * @brief Reset/release a proxy at the end of an event.
 * @param force If true, force a release rather than a reset.
 * @param hard Do a hard reset if true.
 * @returns True if the caller should release the proxy.
 *
 * This is usually called at the end of an event.
 * No locking is done, so there should be no other threads accessing
 * this proxy.
 *
 * `Release' means that we want to remove the proxy from the store.
 * `Reset' means that we keep the proxy, but remove the data object
 * that it references.
 * Each proxy has a flag saying whether it wants to do a release or a reset.
 * This can be forced via the FORCE argument; this would typically be done
 * when deleting the store.
 * This function does not actually release the proxy.  If it returns
 * true, the caller is expected to release the proxy.
 *
 * See AthenaKernel/IResetable.h for the meaning of HARD.
 */
bool DataProxy::requestRelease(bool force, bool hard) {

  if (m_boundHandles) {
    resetBoundHandles(hard);
  }
  bool canRelease = force;
  if (!m_resetFlag) canRelease = true;
#ifndef NDEBUG
  MsgStream gLog(m_ims, "DataProxy");
  if (gLog.level() <= MSG::VERBOSE) {
    gLog << MSG::VERBOSE << "requestRelease(): "
	 << (canRelease ? " release " : " reset")
	 <<" object " 
	 << name() << " CLID " << clID() << " address " << MSG::hex
	 << object() << MSG::dec << endmsg;
  }
#endif
  if (!canRelease) {
    resetRef();
  }
  return canRelease;
}

/// set a DataObject address
/// If doreg is true, then call setRegistry to set the backpointer
/// from obj to the proxt.
void DataProxy::setObject(objLock_t& objLock, DataObject* dObject, bool doreg)
{
  DataObject* dobj = m_dObject;
  setGaudiRef(dObject, dobj);
  m_dObject = dobj;
  if (0 != dobj) {
    if (doreg) dobj->setRegistry(this);
    if (m_const) this->lock (objLock);
  }
}


/// set a DataObject address
void DataProxy::setObject(DataObject* dObject, bool doreg /*= true*/)
{
  objLock_t objLock (m_objMutex);
  setObject (objLock, dObject, doreg);
}


// set IOpaqueAddress
void DataProxy::setAddress(IOpaqueAddress* address)
{
  lock_t lock (m_mutex);
  m_tAddress.setAddress(address);
}

// set IOpaqueAddress
void DataProxy::setAddress(CxxUtils::RefCountedPtr<IOpaqueAddress> address)
{
  lock_t lock (m_mutex);
  m_tAddress.setAddress(std::move(address));
}
//////////////////////////////////////////////////////////////


/**
 * @brief Read in a new copy of the object referenced by this proxy.
 *
 * If this proxy has an associated loader and address, then load
 * a new copy of the object and return it.  Any existing copy
 * held by the proxy is unaffected.
 *
 * This will fail if the proxy does not refer to an object read from an
 * input file.
 *
 * Returns a null pointer on failure.
 */
std::unique_ptr<DataObject> DataProxy::readData()
{
  // Public wrapper for readData().
  objLock_t objLock (m_objMutex);
  return readData (objLock, nullptr);
}


/**
 * @brief Read in a new copy of the object referenced by this proxy.
 * @param errNo If non-null, set to the resulting error code.
 *
 * If this proxy has an associated loader and address, then load
 * a new copy of the object and return it.  Any existing copy
 * held by the proxy is unaffected.
 *
 * This will fail if the proxy does not refer to an object read from an
 * input file.
 */
std::unique_ptr<DataObject> DataProxy::readData (objLock_t&, ErrNo* errNo)
{
  if (errNo) {
    if (*errNo == RECURSIVEREAD) {
      // This can end up being called from noexcept functions.
      // Creating a MsgStream can throw an exception --- don't let
      // it escape.
      try {
        MsgStream gLog(m_ims, "DataProxy");
        gLog << MSG::ERROR
             << "readData: ERROR recursive read for object" 
             <<m_tAddress.clID() << '/' << m_tAddress.name() << '\n'
             <<" Returning NULL DataObject pointer  " << endmsg;
      }
      catch (const GaudiException&) {
        std::cerr << "DataProxy::readData: Problem creating MsgStream\n";
      }
      return nullptr;
    }
    *errNo = RECURSIVEREAD;
  }

  IConverter* dataLoader;
  IProxyDict* store;
  IOpaqueAddress* address;
  {
    lock_t lock (m_mutex);
    if (0 == m_dataLoader) {
      //MsgStream gLog(m_ims, "DataProxy");
      //gLog << MSG::WARNING
      //	  << "accessData:  IConverter ptr not set" <<endmsg;
      if (errNo) *errNo=NOCNVSVC;
      return nullptr;
    }

    dataLoader = m_dataLoader;
    store = m_store;
    address = m_tAddress.address();
  }

  if (!isValidAddress()) {
    //MsgStream gLog(m_ims, "DataProxy");
    //gLog << MSG::WARNING
    //	 << "accessData:  IOA pointer not set" <<endmsg;
    if (errNo) *errNo=NOIOA;
    return nullptr;
  }

  SG::CurrentEventStore::Push push (store);

  DataObject* obj = nullptr;
  StatusCode sc;
  if (store)
    sc = store->createObj (dataLoader, address, obj);
  else
    sc = dataLoader->createObj (address, obj);
  if (sc.isSuccess()) {
    if (errNo && *errNo == RECURSIVEREAD) *errNo = ALLOK;
    return std::unique_ptr<DataObject>(obj);
  }
  if (errNo) *errNo = CNVFAILED;
  return nullptr;
}


/// Access DataObject on-demand using conversion service
DataObject* DataProxy::accessDataOol()
{
  // This is done in the inlined accessData().
  //if (0 != m_dObject) return m_dObject;  // cached object

  objLock_t objLock (m_objMutex);
  // Check again after acquiring the lock.
  if (0 != m_dObject) return m_dObject;  // cached object

  if (isValidAddress()) {
    // An address provider called by isValidAddress may have set the object
    // pointer directly, rather than filling in the address.  So check
    // the cached object pointer again.
    if (0 != m_dObject) return m_dObject;  // cached object
  }

  std::unique_ptr<DataObject> obju = readData (objLock, &m_errno);
  if (!obju) {
    if (m_errno == NOIOA) {
      MsgStream gLog(m_ims, "DataProxy");
      gLog << MSG::WARNING
           << "accessData:  IOA pointer not set" <<endmsg;
    }
    else if (m_errno == CNVFAILED) {
      MsgStream gLog(m_ims, "DataProxy");
      gLog << MSG::WARNING 
           << "accessData: conversion failed for data object " 
           <<m_tAddress.clID() << '/' << m_tAddress.name() << '\n'
           <<" Returning NULL DataObject pointer  " << endmsg;
    }
    setObject(objLock, 0, true);
    return 0;   
  }

  DataBucketBase* bucket = dynamic_cast<DataBucketBase*>(obju.get());
  if (m_t2p) {
    if (bucket) {
      void* payload = bucket->object();
      m_t2p->t2pRegister(payload, this);
      m_errno=ALLOK;

      // Register bases as well.
      const SG::BaseInfoBase* bi = SG::BaseInfoBase::find (m_tAddress.clID());
      if (bi) {
        std::vector<CLID> base_clids = bi->get_bases();
        for (unsigned i=0; i < base_clids.size(); ++i) {
          // nb. DataProxy_cast here will give an infinite recursion!
          void* bobj = SG::Storable_cast (obju.get(), base_clids[i], nullptr, true);
          if (bobj && bobj != payload)
            m_t2p->t2pRegister (bobj, this);
        }
      }
    }
    else {
      MsgStream gLog(m_ims, "DataProxy");
      gLog << MSG::ERROR
           << "accessData: ERROR registering object in t2p map" 
           <<m_tAddress.clID() << '/' << m_tAddress.name() << '\n'
           <<" Returning NULL DataObject pointer  " << endmsg;
      obju.reset();
      m_errno=T2PREGFAILED;
    }
  }

  // Must get everything else set up before setting m_dObject, because
  // once we set it, it's immediately visible to other threads.
  // In particular, we were previously doing this before the t2p registration.
  // This meant that another thread could retrieve the pointer from
  // the proxy, but then fail trying to map the pointer back to the proxy.
  // See ATEAM-1126.
  std::atomic_thread_fence (std::memory_order_seq_cst);
  setObject(objLock, obju.release(), true);

  return m_dObject;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
bool DataProxy::isValidAddress() const
{
  // Looking up the context is relatively expensive.
  // So first try isValid() without the context.
  {
    lock_t lock (m_mutex);
    if (const_cast<DataProxy*>(this)->m_tAddress.isValid(nullptr)) {
      return true;
    }
  }
  // Get the context.  (Must not be holding m_mutex here.)
  const EventContext& ctx = contextFromStore();
  // Try again with the context.
  lock_t lock (m_mutex);
  return const_cast<DataProxy*>(this)->m_tAddress.isValid(&ctx);
}

bool DataProxy::updateAddress()
{
  // Be sure to get the context before acquiring the lock.
  const EventContext& ctx = contextFromStore();
  lock_t lock (m_mutex);
  return m_tAddress.isValid(&ctx, true);
}

/**
 * @brief Try to get the pointer back from a @a DataProxy,
 *        converted to be of type @a clid.
 * @param proxy The @a DataProxy.
 * @param clid The ID of the class to which to convert.
 *
 * Only works if the held object is a @a DataBucket.
 * Returns nullptr on failure,
 */
void* SG::DataProxy_cast (SG::DataProxy* proxy, CLID clid, const std::type_info* tinfo /*= nullptr*/)
{
  if (nullptr == proxy || !proxy->isValid()) {
    return nullptr;
  }

  DataObject* pObject = proxy->accessData();
  if (nullptr == pObject) {
#ifndef NDEBUG
    MsgStream gLog(Athena::getMessageSvc(), "SG::DataProxy_cast");
    gLog << MSG::WARNING 
         << "this proxy " << MSG::hex << proxy
         << MSG::dec << " has a NULL data object ptr" << endmsg;
#endif
    return nullptr;
  }

  void* result = SG::Storable_cast (pObject, clid, tinfo, true, proxy, proxy->isConst());
  if (nullptr == result) { 
    //if result is null, probably CLID is neither the type the object was
    // stored with, nor it inherits from it. 
    // Before giving up let's check its transient CLIDs
    DataBucketBase* db(0);
    if (proxy->transientID(clid) &&
        nullptr != (db = dynamic_cast<DataBucketBase*>(pObject)) )
    {
      //it is a symlink after all. Let's hard cast and keep our fingers Xed
      // But first: if this is a non-const proxy, then the cast
      // may have failed because it needed to go via a copying conversion
      // that's not allowed for non-const objects.  So try the conversion
      // again as const; if that works, then don't do the hard cast.
      if (!proxy->isConst() &&
          SG::Storable_cast(pObject, clid, tinfo, true, proxy, true) != nullptr)
      {
#ifndef NDEBUG
        MsgStream gLog(Athena::getMessageSvc(), "SG::DataProxy_cast");
        gLog << MSG::WARNING 
             << "Request for a non-const object via copying conversion; "
             << "requested CLID = " << clid
             << ", proxy primary ID is " << proxy->clID() << endmsg   ;
#endif
      }
      else {
        // ok, hard cast.
        result = db->object();
      }
    }
    else { 
#ifndef NDEBUG
      MsgStream gLog(Athena::getMessageSvc(), "SG::DataProxy_cast");
      gLog << MSG::WARNING 
           << "Request for an invalid object; requested CLID = " 
           << clid
           << ", proxy primary ID is " << proxy->clID() << endmsg   ;
#endif
    } //try symlink
  } //result 0

  return result;
}

  
/**
 * @brief Register a transient object in a t2p map.
 * @param trans The object to register.
 */
void DataProxy::registerTransient (void* p)
{
  lock_t lock (m_mutex);
  if (m_t2p)
    m_t2p->t2pRegister (p, this);
}


/**
 * @brief Lock the data object we're holding, if any.
 *
 * Should be called with the mutex held.
 */
void DataProxy::lock (objLock_t&)
{
  DataObject* dobj = m_dObject;
  DataBucketBase* bucket = dynamic_cast<DataBucketBase*>(dobj);
  if (bucket)
    bucket->lock();
}


/**
 * @brief Retrieve the EventContext saved in the parent store.
 *
 * If there is no context recorded in the store, return a default-initialized
 * context.
 *
 * Do not call this holding m_mutex, or we could deadlock (ATEAM-755).
 * (The store lock must be acquired before the DataProxy lock.)
 */
const EventContext& DataProxy::contextFromStore() const
{
  IProxyDict* store = m_store;
  if (store) {
    static const SG::sgkey_t ctxkey = 
      store->stringToKey ("EventContext", ClassID_traits<EventContext>::ID());
    SG::DataProxy* proxy = store->proxy_exact (ctxkey);
    if (proxy && proxy->object()) {
      EventContext* ctx = SG::DataProxy_cast<EventContext> (proxy);
      if (ctx) return *ctx;
    }
  }
  static const EventContext emptyContext;
  return emptyContext;
}
