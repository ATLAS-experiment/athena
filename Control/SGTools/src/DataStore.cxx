/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "SGTools/DataStore.h"
#include "SGTools/DataProxy.h"
#include "SGTools/exceptions.h"
#include "AthenaKernel/IStringPool.h"
#include "GaudiKernel/ClassID.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/ISvcLocator.h"
#include "SGAudCore/ISGAudSvc.h"
#include "CxxUtils/ConcurrentPtrSet.h"
#include "CxxUtils/SimpleUpdater.h"
#include "CxxUtils/checker_macros.h"
#include "CxxUtils/AthUnlikelyMacros.h"

using namespace std;
using SG::DataStore;
using SG::DataProxy;
using SG::ProxyMap;
using SG::ProxyIterator;
using SG::ConstProxyIterator;


/**
 * @brief Constructor.
 * @param pool The string pool associated with this store.
 */
DataStore::DataStore (IProxyDict& pool)
  : m_pool (pool),
    m_storeMap(),
    m_keyMap(KeyMap_t::Updater_t()),
    m_storeID(StoreID::UNKNOWN), m_t2p(), 
    m_noAudSvc(0), m_pSvcLoc(0)
{
   setSvcLoc().ignore();
}

//Destructor
DataStore::~DataStore()
{
  clearStore(false, true, nullptr);
}

StatusCode DataStore::setSvcLoc(){
  StatusCode sc(StatusCode::FAILURE);
  m_pSvcLoc = Gaudi::svcLocator( );
  if (!m_pSvcLoc) std::cout<<"DataStore::setSvcLoc: WARNING svcLocator not found! "<<std::endl;
  return sc;
}

void DataStore::setSGAudSvc() {
  if (!m_pSGAudSvc) {
    //try once to get the service
    if (!m_noAudSvc) {
      const bool DONOTCREATE(false);
      m_pSGAudSvc = m_pSvcLoc->service("SGAudSvc", DONOTCREATE);
      m_noAudSvc = not m_pSGAudSvc.isValid();
    }
  }
  return;
}


//////////////////////////////////////////////////////////////
void DataStore::clearStore(bool force, bool hard, MsgStream* /*pmlog*/)
{
  /// Be careful with changing this --- it's important for performance
  /// for some analysis use cases.

  /// Rather than dealing with erasures in m_keyMap, we first do the
  /// removals just from m_storeMap, remembering along the way the
  /// set of proxies to be removed (not reset-ony).  Then if there were any
  /// such proxies, we run through the entries
  /// in m_keyMap and copy all that match one of the reset-only proxies
  /// to a new map and then swap.  (We can't ask the proxy itself if it's
  /// reset-only at this point because if it isn't, it'll have been deleted.)
  std::unordered_set<DataProxy*> removed;
  
  /// Go through the list of unique proxies, and run requestRelease()
  /// on each.  If that returns true, then we're meant to remove this
  /// proxy from the store.  Be careful that removing a proxy from the
  /// store will modify the m_proxies list, so if the proxy is successfully
  /// removed, we should not bump the index.
  for (size_t i = 0; i < m_proxies.size(); ) {
    SG::DataProxy* dp = m_proxies[i];
    if (ATH_UNLIKELY (dp->requestRelease (force, hard))) {
      removed.insert (dp);
      if (removeProxyImpl (dp, i).isFailure()) {
        ++i;
      }
    }
    else {
      ++i;
    }
  }

  if (!removed.empty()) {
    KeyMap_t newMap (KeyMap_t::Updater_t(), m_keyMap.capacity());
    {
      auto lock = newMap.lock();
      auto ctx = KeyMap_t::Updater_t::defaultContext();
      for (auto p : m_keyMap) {
        if (removed.count (p.second) == 0) {
          newMap.emplace (lock, p.first, p.second, ctx);
        }
      }
    }
    m_keyMap.swap (newMap);
  }

  // clear T2PMap
  m_t2p.clear();
}

/////////////////////////////////////////////////////////////
// access all the keys associated with an object: 
// 
void DataStore::keys(const CLID& id, std::vector<std::string>& vkeys,
		     bool includeAlias, bool onlyValid)
{
  vkeys.clear();
  for (const ProxyMap::value_type& p : m_storeMap[id]) {
    bool includeProxy(true);
    if (onlyValid) includeProxy=p.second->isValid();
    if (includeAlias) {
      if (includeProxy) vkeys.push_back(p.first);
    }
    else {
      if (p.first == p.second->name() && includeProxy) 
	vkeys.push_back(p.first);
    }
  }
}

//////////////////////////////////////////////////////////////
//---------------------------------------------------------------//
// record the proxy in StoreGate
StatusCode 
DataStore::addToStore(const CLID& id, DataProxy* dp)
{
  if (!dp) {
    return StatusCode::FAILURE;
  }

  /// If this proxy has not yet been added to the store, then make
  /// an entry for it in m_proxies.
  bool primary = false;
  if (dp->store() == nullptr) {
    m_proxies.push_back (dp);
    primary = true;
  }

  if (id == 0 && dp->clID() == 0 && dp->sgkey() != 0) {
    // Handle a dummied proxy.
    m_keyMap.emplace (dp->sgkey(), dp);
  }
  else {
    ProxyMap& pmap = m_storeMap[id];

    // Set the primary key.
    sgkey_t primary_sgkey = m_pool.stringToKey (dp->name(), dp->clID());
    sgkey_t sgkey = primary_sgkey;
    if (id != dp->clID()) {
      sgkey = m_pool.stringToKey (dp->name(), id);
    }
    if (dp->sgkey() == 0) {
      dp->setSGKey (sgkey);
    }

    if (!m_keyMap.emplace (sgkey, dp).second) {
      if (primary) {
        m_proxies.pop_back();
      }
      return StatusCode::FAILURE;
    }

    pmap.insert(ProxyMap::value_type(dp->name(), dp));
  }

  // Note : No checking if proxy already exists in store because it
  // is already checked in StoreGateSvc.  
  //  if (pmap.find(dp->name()) == pmap.end()) {
  dp->addRef();   // The store now owns this
  dp->setT2p(&m_t2p);
  dp->setStore(&m_pool);

  return StatusCode::SUCCESS;
}


//////////////////////////////////////////////////////////////////
//---------------------------------------------------------------//
// if Proxy is a resettable proxy only then reset it, otherwise
// delete it and remove from proxy map.
StatusCode
DataStore::removeProxy(DataProxy* proxy, bool forceRemove, bool hard)
{
  if (!proxy) {
    return StatusCode::FAILURE;
  }
  
  if (!forceRemove && proxy->isResetOnly()) {
    // A reset-only proxy.  Don't remove it.
    proxy->reset (hard);
    return StatusCode::SUCCESS;
  }

  // First remove from m_keyMap.
  // We may fail to find the entry if this is a key that has been versioned.
  // E.g., we add aVersObj.  Call this DP1.
  // Then we add a new version of it.  Call this DP2.
  // The version logic will add the alias ';00;aVersObj' to DP1.
  // It will also then add the alias `aVersObj' to DP2.
  // This will overwrite the entries for DP1 in pmap and m_keyMap.
  // If we then clear and DP2 is removed first, then the m_keyMap entry
  // for DP1's primary key will be gone.
  // FIXME: Should just remove the versioned key code ... it's anyway
  // not compatible with MT.
  removeFromKeyMap (proxy).ignore();

  // Then remove from the m_storeMap and release the proxy.
  auto it = std::find (m_proxies.begin(), m_proxies.end(), proxy);
  if (it == m_proxies.end()) {
    return StatusCode::FAILURE;
  }
  return removeProxyImpl (proxy, it - m_proxies.begin());
}


/**
 * @brief Remove a proxy from m_keyMap.
 * @param proxy The proxy being removed.
 */
StatusCode DataStore::removeFromKeyMap (DataProxy* proxy)
{
  SG::DataProxy::AliasCont_t alias_set = proxy->alias();
  std::string name = proxy->name();

  for (CLID symclid : proxy->transientID()) 
  {
    sgkey_t sgkey = m_pool.stringToKey (name, symclid);
    m_keyMap.erase (sgkey);

    for (const std::string& alias : alias_set) {
      m_keyMap.erase (m_pool.stringToKey (alias, symclid));
    }
  }

  return StatusCode::SUCCESS;
}


/**
 * @brief Helper for removing a proxy.
 * @param proxy The proxy being removed.
 * @param index The index of this proxy in m_proxies.
 *
 * This removes the proxy from m_storeMap and releases it,
 * but does NOT remove it from m_keyMap.
 */
StatusCode
DataStore::removeProxyImpl (DataProxy* proxy, int index)
{
  proxy->setStore (nullptr);
    
  std::string name = proxy->name();
  CLID clid = proxy->clID();
  SG::DataProxy::AliasCont_t alias_set = proxy->alias();

  // nb. This has to be here, not just before the loop below,
  //     as the proxy may be deleted in the meantime.
  SG::DataProxy::CLIDCont_t clids = proxy->transientID();

  StoreIterator storeIter = m_storeMap.find(clid);
  if (storeIter != m_storeMap.end()) {
    ProxyMap& pmap = storeIter->second;

    // first remove the alias key:
    SG::DataProxy::AliasCont_t alias_set = proxy->alias();
    for (const std::string& alias : alias_set) {
      if (1 == pmap.erase(alias)) proxy->release();
    }
      
    // Remove primary entry.
    if (1 == pmap.erase(name)) {
      proxy->release();
    }
  }
  else {
    // A dummy proxy.
    proxy->release();
  }

  // Remove all symlinks too.
  for (CLID symclid : clids) 
  {
    if (clid == symclid) continue;
    storeIter = m_storeMap.find(symclid);
    if (storeIter != m_storeMap.end()) {
      ProxyMap& pmap = storeIter->second;
      SG::ProxyIterator it = pmap.find (name);
      if (it != pmap.end() && it->second == proxy) {
        storeIter->second.erase (it);
        proxy->release();
      }
        
      for (const std::string& alias : alias_set) {
        if (1 == pmap.erase (alias)) proxy->release();
      }
    }
  } //symlinks loop

  if (index != -1 && !m_proxies.empty()) {
    // Remove the proxy from m_proxies.  If it's not at the end, then
    // move the proxy at the end to this proxy's index (and update the
    // index for the other proxy stored in m_keyMap).
    if (index != (int)m_proxies.size() - 1) {
      m_proxies[index] = m_proxies.back();
    }
    m_proxies.pop_back();
  }

  return StatusCode::SUCCESS;
}

//////////////////////////////////////////////////////////////
//---------------------------------------------------------------//
// record the symlink in StoreGate
StatusCode 
DataStore::addSymLink(const CLID& linkid, DataProxy* dp)
{
  // Make sure the symlink doesn't already exist.
  DataProxy* exist = proxy_exact (linkid, dp->name());
  if (exist == dp) {
    // Entry already exists pointing at the desired proxy.
    return StatusCode::SUCCESS;
  }
  else if (!exist) {
    dp->setTransientID(linkid); 
    return addToStore(linkid, dp);
  }

  // Already an existing proxy.
  if (exist->isValidObject()) {
    // And it's set to something.  Ok if it's the same DataObject.
    // Otherwise fail.
    if (exist->object() == dp->object()) {
      dp->setTransientID(linkid); 
      return StatusCode::SUCCESS;
    }
    return StatusCode::FAILURE;
  }
  if (!dp->object()) {
    return StatusCode::FAILURE;
  }
  if (exist->loader()) {
    return StatusCode::FAILURE;
  }

  // The existing proxy is not set to anything, and it doesn't have a loader.
  // It may have been created by a forward-declared link to a base class.
  // In that case, set the existing proxy to also point at the same DataObject.
  const SG::BaseInfoBase* bib = SG::BaseInfoBase::find (dp->clID());
  if (bib && bib->is_base (exist->clID())) {
    dp->setTransientID(linkid);
    exist->setObject (dp->object(), false);
    return StatusCode::SUCCESS;
  }
    
  // Entry already exists pointing at another proxy.
  // Don't change the existing entry.
  return StatusCode::FAILURE;
}
//---------------------------------------------------------------//
// record the alias in StoreGate
StatusCode
DataStore::addAlias(const std::string& aliasKey, DataProxy* dp)
{
  for (CLID clid : dp->transientID()) {
    // locate proxy map and add alias to proxymap
    ProxyMap& pmap = m_storeMap[clid];

    // check if another proxy for the same type caries the same alias.
    // if yes, then remove that alias from that proxy and establish the
    // alias in the new proxy.
    // pmap.insert will overwrite, associate alias with new proxy.
    ConstProxyIterator p_iter = pmap.find(aliasKey);
    if (p_iter != pmap.end() && dp->clID() == p_iter->second->clID()) {
      if (dp->name() == p_iter->second->name()) return StatusCode::SUCCESS;
      p_iter->second->removeAlias(aliasKey);
      p_iter->second->release();
    }
    dp->addRef();
    pmap[aliasKey] = dp;
    m_keyMap.emplace (m_pool.stringToKey (aliasKey, clid), dp);
  }

  // set alias in proxy
  dp->setAlias(aliasKey);

  return StatusCode::SUCCESS;
}
//---------------------------------------------------------------//
// Count instances of TYPE in store
int DataStore::typeCount(const CLID& id) const
{
  ConstStoreIterator storeIter = m_storeMap.find(id);
  if (storeIter == m_storeMap.end()) return 0;
  return (storeIter->second).size();
}

//---------------------------------------------------------------//
// Return proxy for a given Transient Address
DataProxy* DataStore::proxy(const TransientAddress* tAddr) const
{
  return proxy(tAddr->clID(), tAddr->name());
}

// Return proxy for a given Transient ID (TYPE and KEY) 
DataProxy* DataStore::proxy(const CLID& id, const std::string& key) const
{	 

  // The logic here: if a key is specified, we locate it.
  // If we don't find it and the default key (DEFAULTKEY) is specified,
  // then we return any existing proxy for this type as long as there
  // is exactly one, not counting aliases.  More than one would be ambiguous
	  
  ConstStoreIterator siter = m_storeMap.find(id);
  DataProxy *p(0);
  if (siter != m_storeMap.end()) 
  {
    const ProxyMap& pmap = siter->second;
    ConstProxyIterator p_iter = pmap.find(key);
    if (p_iter != pmap.end()) {
      p=p_iter->second;

    } else if (key == SG::DEFAULTKEY && !pmap.empty()) {
      // we did not find the object using key.
      // Now check for default object.
      // If there are multiple entries, they must all be
      // referring to the same proxy (aliases).
      for (const auto& ent : pmap) {
        if (!p) {
          p = ent.second;
        }
        else if (p != ent.second) {
          p = nullptr;
          break;
        }
      }

      // If that didn't work, try it again, considering only objects that
      // are exactly the type being requested.
      if (!p) {
        for (const auto& ent : pmap) {
          if (ent.second->clID() == id) {
            if (!p) {
              p = ent.second;
            }
            else if (p != ent.second) {
              p = nullptr;
              break;
            }
          }
        }
      }
    }
    else {
      // Ok since access to DataStore is serialized by StoreGate.
      DataStore* nc_store ATLAS_THREAD_SAFE = const_cast<DataStore*> (this);
      p = nc_store->findDummy (id, key);
    }
  }
  else {
    // Ok since access to DataStore is serialized by StoreGate.
    DataStore* nc_store ATLAS_THREAD_SAFE = const_cast<DataStore*> (this);
    p = nc_store->findDummy (id, key);
  }

  if (p && m_pSGAudSvc) 
    m_pSGAudSvc->SGAudit(p->name(), id, 0, m_storeID);

  return p;
}


/**
 * @brief Look for (and convert) a matching dummy proxy.
 * @param id The CLID for which to search.
 * @param key The key for which to search.
 *
 * In some cases, we may enter into the store a `dummy' proxy,
 * which is identified only by the hashed CLID/key pair.
 * (This can happen when we have a link to an object that's not
 * listed in the DataHeader; in this case, we know the only hashed key
 * and not the CLID or key.)
 *
 * This function is called after we fail to find a proxy by CLID/key.
 * It additionally checks to see if there exists a dummy proxy with
 * a hash matching this CLID/key.  If so, the CLID/key are filled
 * in in the proxy, and the proxy is entered in m_storeMap.
 *
 * Returns either the matching proxy or 0.
 */
DataProxy* DataStore::findDummy (CLID id, const std::string& key)
{
  sgkey_t sgkey = m_pool.stringToKey (key, id);
  DataProxy* p = proxy_exact (sgkey);
  if (p) {
    p->setID (id, key);
    ProxyMap& pmap = m_storeMap[id];
    if (!pmap.insert(ProxyMap::value_type(key, p)).second) {
      // This shouldn't happen.
      DataProxy* p2 = pmap[key];
      throw SG::ExcProxyCollision (id, key, p2->clID(), p2->name());
    }
  }
  return p;
}


/// get proxy with given key. Returns 0 to flag failure
/// the key must match exactly (no wild carding for the default key)
DataProxy* DataStore::proxy_exact(sgkey_t sgkey) const
{
  if (m_pSGAudSvc) {
    CLID clid;
    const std::string* strkey = m_pool.keyToString (sgkey, clid);
    if (strkey)
      m_pSGAudSvc->SGAudit(*strkey, clid, 0, m_storeID);
  }
  KeyMap_t::const_iterator i = m_keyMap.find (sgkey);
  if (i != m_keyMap.end())
    return i->second;
  return 0;
}


/// Like proxy_exact, but intended to be called without holding
/// the store lock.  However, the store lock still must be passed
/// as an argument; it will be acquired should be need to call
/// the auditor service.
DataProxy* DataStore::proxy_exact_unlocked (sgkey_t sgkey,
                                            std::recursive_mutex& mutex) const
{
  if (m_pSGAudSvc) {
    std::unique_lock lock (mutex);
    CLID clid;
    const std::string* strkey = m_pool.keyToString (sgkey, clid);
    if (strkey)
      m_pSGAudSvc->SGAudit(*strkey, clid, 0, m_storeID);
  }
  KeyMap_t::const_iterator i = m_keyMap.find (sgkey);
  if (i != m_keyMap.end())
    return i->second;
  return 0;
}


/// get proxy with given key. Returns 0 to flag failure
/// the key must match exactly (no wild carding for the default key)
DataProxy* DataStore::proxy_exact(const CLID& id,
                                  const std::string& key) const
{
  // Suppress warning here about calling to a nonconst method
  // of m_pool.  Ok since all callers here must own the store lock.
  IProxyDict& pool_nc ATLAS_THREAD_SAFE = m_pool;
  return proxy_exact (pool_nc.stringToKey (key, id));
}


//---------------------------------------------------------------//
// Return an iterator over proxies for a given CLID:
StatusCode DataStore::pRange(const CLID& id, ConstProxyIterator& pf,
			                     ConstProxyIterator& pe) const
{
  static const ProxyMap emptyMap;
  StatusCode sc(StatusCode::FAILURE);

  ConstStoreIterator storeIter = m_storeMap.find(id);
  if (storeIter != m_storeMap.end()) 
  {
    const ProxyMap& pmap = storeIter->second;
    pf = pmap.begin();
    pe = pmap.end();
    if (pmap.size() > 0) sc = StatusCode::SUCCESS;
  } else {
    //keep valgrind happy
    pf = emptyMap.end();
    pe = emptyMap.end();
  }
  return sc;
}
//---------------------------------------------------------------//
// Return an iterator over the Store Map
StatusCode DataStore::tRange(ConstStoreIterator& tf,
			     ConstStoreIterator& te) const
{
  tf = m_storeMap.begin();
  te = m_storeMap.end();
  return StatusCode::SUCCESS;
}
//---------------------------------------------------------------//
// locate Persistent Representation of a Transient Object
//
DataProxy* DataStore::locatePersistent(const void* const pTransient) const
{
  return m_t2p.locatePersistent(pTransient);
}

//////////////////////////////////////////////////////////////////
StatusCode
DataStore::t2pRegister(const void* const pTrans, DataProxy* const pPers)
{
    std::string name=pPers->name();
    int i=name.find('/', 0);
    name=name.erase(0,i+1);  

  if (doAudit()) m_pSGAudSvc->SGAudit(name, pPers->clID(), 1, m_storeID);

  return (m_t2p.t2pRegister(pTrans, pPers)) ? 
			StatusCode::SUCCESS :
			StatusCode::FAILURE; 
}


const std::vector<DataProxy*>& DataStore::proxies() const
{
  return m_proxies;
}

