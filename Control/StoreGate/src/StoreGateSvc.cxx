/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "GaudiKernel/IIncidentSvc.h"
#include "AthenaKernel/errorcheck.h"
#include "StoreGate/StoreClearedIncident.h"
#include "AthAllocators/ArenaHeader.h"

#include "StoreGate/StoreGateSvc.h"
#include "StoreGate/tools/SGImplSvc.h"
#include "SGTools/DataStore.h"

#include "Gaudi/Interfaces/IOptionsSvc.h"
#include "GaudiKernel/IAppMgrUI.h"
#include <fstream>
#include <algorithm>

using namespace SG;
using namespace std;

namespace {
    thread_local HiveEventSlot* currentHiveEventSlot = nullptr;
    thread_local StoreGateSvc* currentStoreGate = nullptr;
}

/// Standard Constructor
StoreGateSvc::StoreGateSvc(const std::string& name,ISvcLocator* svc) : 
  base_class(name,svc),
  m_storeID (StoreID::findStoreID(name))
{
}
 
/// Standard Destructor
StoreGateSvc::~StoreGateSvc()  
{}

void 
StoreGateSvc::setDefaultStore(SGImplSvc* pStore) {
  m_defaultStore = pStore;
}

void 
StoreGateSvc::setSlot(SG::HiveEventSlot* pSlot) { 
  currentHiveEventSlot=pSlot;
  if ( 0 != currentHiveEventSlot) {
    currentHiveEventSlot->pEvtStore->makeCurrent();
  }
}

StoreGateSvc *StoreGateSvc::currentStoreGate() {
  if (!::currentStoreGate) {
    // this is a static function so we don't have many conveniences
    ISvcLocator *svcLocator = Gaudi::svcLocator();
    SmartIF<StoreGateSvc> sg{svcLocator->service("StoreGateSvc/StoreGateSvc")};
    if ( !sg.isValid() ) {
      throw GaudiException(
          "Could not get \"StoreGateSvc\" to initialize currentStoreGate",
          "StoreGateSvc", StatusCode::FAILURE);
    }
    sg->makeCurrent();
    return sg;
  }
  return ::currentStoreGate;
}

SG::HiveEventSlot*
StoreGateSvc::currentSlot() { 
  return currentHiveEventSlot; 
}

/////////////////////////////////////////////////////////////////


void
StoreGateSvc::commitNewDataObjects() {
  _SGVOIDCALL(commitNewDataObjects, ());
}

/// Create a proxy object using an IOpaqueAddress and a transient key
StatusCode 
StoreGateSvc::recordAddress(const std::string& skey,
                            CxxUtils::RefCountedPtr<IOpaqueAddress> pAddress,
                            bool clearAddressFlag) {
  _SGXCALL(recordAddress, (skey, std::move(pAddress), clearAddressFlag), StatusCode::FAILURE);
}
/// Create a proxy object using an IOpaqueAddress
StatusCode 
StoreGateSvc::recordAddress(CxxUtils::RefCountedPtr<IOpaqueAddress> pAddress,
                            bool clearAddressFlag) {
  _SGXCALL(recordAddress, (std::move(pAddress), clearAddressFlag), StatusCode::FAILURE);
}


StatusCode 
StoreGateSvc::setConst(const void* pObject) {
  _SGXCALL(setConst, (pObject), StatusCode::FAILURE);
}


/////////////////////////////////////////////////////////////
/// Service initialization
StatusCode StoreGateSvc::initialize()    {

  // Initialize service:
  CHECK( Service::initialize() );

  verbose() << "Initializing " << name() << endmsg;

  SmartIF<Gaudi::Interfaces::IOptionsSvc> pJOSvc{serviceLocator()->service("JobOptionsSvc")};
  if ( !pJOSvc.isValid() ) {
    error() << "Failed to retrieve JobOptionsSvc" << endmsg;
  }
  //copy our properties to the prototype (default) SGImplSvc
  const std::string implStoreName = name() + "_Impl";
  for (const Gaudi::Details::PropertyBase* p : getProperties()) {
    pJOSvc->set( implStoreName + "." + p->name(), p->toString() );
  }

  //HACK ALERT: using createService rather then the customary service(...,CREATEIF=true) we set our pointer
  // to SGImplSvc early (even before it is initialized). This should help take care of some initialize loops
  // for example when we try to record an address from one of the address providers initialize methods

  std::string implStoreFullName = "SGImplSvc/" + implStoreName;
  debug() << "trying to create store " << implStoreFullName << endmsg;
  
  m_defaultStore = serviceLocator().as<ISvcManager>()->createService(implStoreFullName);

  if (!m_defaultStore) {
    error() << "Could not create store " << implStoreFullName << endmsg;
    return StatusCode::FAILURE;
  }
  
  if ( m_defaultStore->sysInitialize().isSuccess() ) {
    // If this is the default event store (StoreGateSvc), then declare
    // our arena as the default for memory allocations.
    if (name()  == "StoreGateSvc") {
      m_defaultStore->makeCurrent();
    }
  } else {
    error() << "Could not initialize default store " << implStoreFullName 
            << endmsg;
    return StatusCode::FAILURE;
  }
  if ( !m_incSvc.retrieve().isSuccess() ) {
    error() << "Could not locate IncidentSvc" << endmsg;
    return StatusCode::FAILURE;
  }

  // Don't retrieve m_activeStoreSvc here to prevent a possible
  // initialization loop.

  const int PRIORITY=100;
  m_incSvc->addListener(this, "EndEvent",PRIORITY);
  m_incSvc->addListener(this, "BeginEvent", PRIORITY);

  return StatusCode::SUCCESS;
}

/// Service start
StatusCode StoreGateSvc::stop()    {
  verbose() << "Stop " << name() << endmsg;
  //HACK ALERT: ID event store objects refer to det store objects
  //by setting an ad-hoc priority for event store(s) we make sure they are finalized and hence cleared first
  // see e.g. https://savannah.cern.ch/bugs/index.php?99993
  if (m_defaultStore->store()->storeID() == StoreID::EVENT_STORE) {
    auto pISM = serviceLocator().as<ISvcManager>();
    pISM->setPriority(name(), pISM->getPriority(name())+1).ignore();
    verbose() << "stop: setting service priority to " << pISM->getPriority(name()) 
          << " so that event stores get finalized and cleared before other stores" <<endmsg;
  }
  return StatusCode::SUCCESS;
}

void StoreGateSvc::handle(const Incident &inc) {
  currentStore()->handle(inc);
}

//////////////////////////////////////////////////////////////

StatusCode
StoreGateSvc::finalize() {
  CHECK( Service::finalize() );
  verbose() << "Finalizing " << name() << endmsg;
  if (m_defaultStore) {
    // m_defaultStore is not active, so ServiceManager won't finalize it!
    CHECK( m_defaultStore->finalize());
  }

  printBadList (m_badRetrieves, "retrieve()");
  printBadList (m_badRecords,   "record()");
  return StatusCode::SUCCESS;
}


/// get proxy for a given data object address in memory
DataProxy* 
StoreGateSvc::proxy(const void* const pTransient) const {
  _SGXCALL(proxy, (pTransient), 0);
}

/// get default proxy with given id. Returns 0 to flag failure
DataProxy* 
StoreGateSvc::proxy(const CLID& id) const {
  _SGXCALL(proxy, (id), 0);
}

/// get proxy with given id and key. Returns 0 to flag failure
DataProxy* 
StoreGateSvc::proxy(const CLID& id, const std::string& key) const { 
  _SGXCALL(proxy, (id, key), 0);
}


/// get default proxy with given id, optionally checking validity.
///  @returns 0 to flag failure
SG::DataProxy* 
StoreGateSvc::proxy(const CLID& id, bool checkValid) const {
  _SGXCALL(proxy, (id, checkValid), 0);
}

/// get proxy with given id and key, optionally checking validity.
///  @returns 0 to flag failure
SG::DataProxy* 
StoreGateSvc::proxy(const CLID& id, const std::string& key, bool checkValid) const {
  _SGXCALL(proxy, (id, key, checkValid), 0);
}


/**
 * @brief Raw addition of a proxy to the store.
 * @param id CLID of object being added.
 * @param proxy proxy to add.
 */
StatusCode StoreGateSvc::addToStore (CLID id, SG::DataProxy* proxy)
{
  _SGXCALL(addToStore,  (id, proxy), StatusCode::FAILURE);
}


/**
 * @brief Record an object in the store.
 * @param obj The data object to store.
 * @param key The key as which it should be stored.
 * @param allowMods If false, the object will be recorded as const.
 * @param returnExisting If true, return proxy if this key already exists.
 *                       If the object has been recorded under a different
 *                       key, then make an alias.
 *                       If the object has been recorded under a different
 *                       clid, then make a link.
 *
 * Full-blown record.  @c obj should usually be something
 * deriving from @c SG::DataBucket.
 *
 * Returns the proxy for the recorded object; nullptr on failure.
 * If the requested CLID/key combination already exists in the store,
 * the behavior is controlled by @c returnExisting.  If true, then
 * the existing proxy is returned; otherwise, nullptr is returned.
 * In either case, @c obj is destroyed.
 */
SG::DataProxy*
StoreGateSvc::recordObject (SG::DataObjectSharedPtr<DataObject> obj,
                            const std::string& key,
                            bool allowMods,
                            bool returnExisting)
{
  _SGXCALL(recordObject, (std::move(obj), key, allowMods, returnExisting), nullptr);
}


/// return the list of all current proxies in store
vector<const SG::DataProxy*> 
StoreGateSvc::proxies() const {
  vector<const SG::DataProxy*> nullV;
  _SGXCALL(proxies, (), nullV);
}

/// Return all CLIDs in the store.
vector<CLID> 
StoreGateSvc::clids() const
{
  vector<CLID> nullV;
  _SGXCALL(clids, (), nullV);
}

/// get proxy with given id and key. Does not query ProxyProviderSvc.
///  @returns 0 to flag failure
SG::DataProxy* 
StoreGateSvc::transientProxy(const CLID& id, const std::string& key) const {
  _SGXCALL(transientProxy, (id, key), 0);
}


/// find proxy and access its data. Returns 0 to flag failure
DataObject* 
StoreGateSvc::accessData(const CLID& id) const {
  _SGXCALL(accessData, (id), 0);
}

/// find proxy and access its data. Returns 0 to flag failure
DataObject* 
StoreGateSvc::accessData(const CLID& id, const std::string& key) const {
  _SGXCALL(accessData, (id, key), 0);
}

bool
StoreGateSvc::transientSwap( const CLID& id,
                             const std::string& keyA, const std::string& keyB ) {
  _SGXCALL(transientSwap, (id, keyA, keyB), false);
}

/// type-less recording of an object with a key, allow possibility of
/// specifying const-access and history record
StatusCode 
StoreGateSvc::typeless_record( DataObject* obj, const std::string& key,
                               const void* const raw_ptr,
                               bool allowMods, bool resetOnly,
                               bool noHist ) {
  _SGXCALL(typeless_record, (obj, key, raw_ptr, allowMods, resetOnly, noHist), StatusCode::FAILURE);
}
/// same as typeless_record, allows to overwrite an object in memory or on disk
StatusCode 
StoreGateSvc::typeless_overwrite( const CLID& id,
                                  DataObject* obj, const std::string& key,
                                  const void* const raw_ptr,
                                  bool allowMods,
                                  bool noHist,
                                  const std::type_info* tinfo) {
  _SGXCALL(typeless_overwrite, (id, obj, key, raw_ptr, allowMods, noHist, tinfo), StatusCode::FAILURE);
}

/// set store id in DataStore:
void 
StoreGateSvc::setStoreID(StoreID::type id)
{
  m_storeID = id;
  // FIXME: should broadcast this to all instances.
  _SGVOIDCALL(setStoreID,(id));
}

void
StoreGateSvc::keys(const CLID& id, std::vector<std::string>& vkeys, 
                   bool includeAlias, bool onlyValid) const
{ 
  _SGVOIDCALL(keys,(id, vkeys, includeAlias, onlyValid));
} 



std::string 
StoreGateSvc::dump() const {
  std::string nullS = "\n>>>>>>>NO CURRENT STORE<<<<<<<\n";
  _SGXCALL(dump, (), nullS);  
}

int
StoreGateSvc::typeCount(const CLID& clid) const{
  _SGXCALL(typeCount, (clid), -1);
}



typename StoreGateSvc::sgkey_t 
StoreGateSvc::stringToKey (const std::string& str, CLID clid) {
  _SGXCALL( stringToKey, (str, clid), 0 );
}

const std::string* 
StoreGateSvc::keyToString (sgkey_t key) const {
  _SGXCALL( keyToString, (key), 0 );
}

const std::string* 
StoreGateSvc::keyToString (sgkey_t key, CLID& clid) const {
  _SGXCALL( keyToString, (key, clid), 0 );
}

void 
StoreGateSvc::registerKey (sgkey_t key,
                           const std::string& str,
                           CLID clidid) {
  _SGVOIDCALL( registerKey, (key, str, clidid) );
}
void 
StoreGateSvc::remap_impl (sgkey_t source,
                          sgkey_t target,
                          off_t index_offset) {
  _SGVOIDCALL( remap_impl, (source, target, index_offset) );
}

bool 
StoreGateSvc::tryELRemap (sgkey_t sgkey_in, size_t index_in,
                          sgkey_t& sgkey_out, size_t& index_out) {
  _SGXCALL( tryELRemap, (sgkey_in, index_in, sgkey_out, index_out), false );
}

StatusCode 
StoreGateSvc::proxyRange(const CLID& id,
                         SG::ConstProxyIterator& beg,
                         SG::ConstProxyIterator& end) const {
  _SGXCALL( proxyRange, (id, beg, end), StatusCode::FAILURE );
}


void 
StoreGateSvc::releaseObject(const CLID& id, const std::string& key) {
  _SGVOIDCALL( releaseObject, (id, key) );
}

void 
StoreGateSvc::clearProxyPayload(SG::DataProxy* proxy) {
  _SGVOIDCALL( clearProxyPayload, (proxy) );
}


StatusCode 
StoreGateSvc::loadEventProxies() {
  this->makeCurrent();
  _SGXCALL(loadEventProxies, (), StatusCode::FAILURE);
}

//////////////////////////////////////////////////////////////
// clear store
StatusCode 
StoreGateSvc::clearStore(bool forceRemove)
{
  StatusCode sc = currentStore()->clearStore(forceRemove);

  // Send a notification that the store was cleared.
  if (sc.isSuccess()) {
    m_incSvc->fireIncident(StoreClearedIncident (this, name()));
  }
  return sc;
}

void
StoreGateSvc::emptyTrash() {
  _SGVOIDCALL( emptyTrash, () );
}

/// The current store is becoming the active store.  Make this the current
/// StoreGate, switch the allocation arena, and call
/// sg::currenteventstore::setstore
void StoreGateSvc::makeCurrent() {
  ::currentStoreGate = this;
  _SGVOIDCALL (makeCurrent, ());
}

StatusCode StoreGateSvc::removeProxy(SG::DataProxy* proxy, const void* pTrans, 
                                     bool forceRemove) {
  _SGXCALL(removeProxy, (proxy, pTrans, forceRemove), StatusCode::FAILURE);
}


/**
 * @brief Call converter to create an object, with locking.
 * @param cvt The converter to call.
 * @param addr Opaque address information for the object to create.
 * @param refpObject Reference to location of the pointer of the
 *                   created object.
 *
 * This calls the @c createObj method on @c cvt to create the referenced
 * transient object, locking the store during the call.
 */
StatusCode
StoreGateSvc::createObj (IConverter* cvt,
                         IOpaqueAddress* addr,
                         DataObject*& refpObject)
{
  _SGXCALL( createObj, (cvt, addr, refpObject), StatusCode::FAILURE );
}


/**
 * @brief Remember that retrieve or record was called for a MT store.
 * @param bad The list on which to store the operation.
 * @param clid CLID of the operation.
 * @param key Key of the operation.
 */
void StoreGateSvc::rememberBad (BadItemList& bad,
                                CLID clid, const std::string& key) const
{
  if (m_storeID == StoreID::EVENT_STORE && currentHiveEventSlot != nullptr) {
    lock_t lock (m_badMutex);
    std::string algo;
    if (m_algContextSvc.isValid()) {
      if (IAlgorithm* alg = m_algContextSvc->currentAlg()) {
        if (alg->type() == "AthenaOutputStream") return;
        if (alg->type() == "AthIncFirerAlg") return;
        algo = alg->type() + "/" + alg->name();
        bad.insert (BadListItem (clid, key, algo));
      }
    }
  }
}


/** 
 * @brief Print out a list of bad calls during finalization.
 * @param bad List of bad calls.
 * @param what Description of the operation.
 */
void StoreGateSvc::printBadList (const BadItemList& bad,
                                 const std::string& what) const
{
  if (bad.empty()) return;
  std::vector<std::string> lines;
  for (const BadListItem& id : bad) {
    lines.push_back (id.fullKey() + " [" + id.m_algo + "]");
  }
  std::sort (lines.begin(), lines.end());
  warning() << "Called " << what << " on these objects in a MT store" << endmsg;
  for (const std::string& s : lines) {
    warning() << s << endmsg;
  }
}


void SG_dump (StoreGateSvc* sg)
{
  std::cout << sg->dump() << "\n";
}

void SG_dump (StoreGateSvc* sg, const char* fname)
{
  std::ofstream f (fname);
  f << sg->dump() << "\n";
  f.close();
}

