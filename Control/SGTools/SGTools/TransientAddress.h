/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef SGTOOLS_TRANSIENTADDRESS_H
#define SGTOOLS_TRANSIENTADDRESS_H

///< includes:
#include <string>
#include <set>
#include <vector>
#include <algorithm>
#include <atomic>


///< Gaudi includes:
#include "AthenaKernel/IStringPool.h"
#include "AthenaKernel/StoreID.h"
#include "GaudiKernel/ClassID.h"
#include "CxxUtils/CachedValue.h"
#include "CxxUtils/checker_macros.h"

///< forward declarations:
class IOpaqueAddress;
class IAddressProvider;
class IProxyDict;
class EventContext;

namespace SG {

  class TransientAddress
  {

  public:

    /// Strictly a set, but there shouldn't be more than a handful
    /// of entries, so store it as a sorted vector instead.
    typedef std::vector<CLID> TransientClidSet;

    typedef std::vector<std::string> TransientAliasSet;
    typedef IStringPool::sgkey_t sgkey_t;

    ///< Default Constructor
    TransientAddress();

    ///< Construct from clid and string key:
    TransientAddress(CLID id, const std::string& key);

    ///< Construct from clid, key and IOpaqueAddress
    TransientAddress(CLID id, const std::string& key, 
		     IOpaqueAddress* addr, bool clearAddress = true);

    ///< Constructor giving full list of symlinked IDs --- used
    ///  from DataHeaderElement::getAddress().
    TransientAddress(CLID id, const std::string& key, 
		     IOpaqueAddress* addr,
                     const std::vector<CLID>& clids);

    TransientAddress (const TransientAddress&);
    TransientAddress (TransientAddress&&);

    ///< Destructor
    ~TransientAddress();

    TransientAddress& operator= (const TransientAddress&);
    TransientAddress& operator= (TransientAddress&&);

    /// Set the CLID / key.
    /// This will only succeed if the clid/key are currently clear.
    void setID (CLID id, const std::string& key);

    ///< Reset
    void reset();

    ///< Retrieve IOpaqueAddress
    // Can't change the signature here to satisfy the thread-safety
    // checker as this should match the signature in the Gaudi IRegistry.
    IOpaqueAddress* address ATLAS_NOT_CONST_THREAD_SAFE () const; 

    ///< set IOpaqueAddress
    void setAddress(IOpaqueAddress* pAddress);

    ///< Retrieve primary clid
    CLID clID() const;

    ///< Retrieve string key:
    const std::string& name() const;

    ///< Get the primary (hashed) SG key.
    sgkey_t sgkey() const;

    ///< Set the primary (hashed) SG key.
    void setSGKey (sgkey_t sgkey);

    ///< check if it is a transient ID (primary or symLinked):
    bool transientID (CLID id) const;

    ///< set transient CLID's
    void setTransientID(CLID id);

    ///< get transient CLID's
    const TransientClidSet& transientID() const;

    ///< set alias'
    void setAlias(const std::string& key);

    ///< set alias'
    void setAlias(const std::vector<std::string>& keys);

    ///< set alias'
    void setAlias(std::vector<std::string>&& keys);

    /// remove alias from proxy
    bool removeAlias(const std::string& key);

    ///< Test for an alias.
    bool hasAlias(const std::string& key) const;

    ///< get transient alias
    const TransientAliasSet& alias() const;

    ///< set the clearAddress flag: IOA will not be deleted in proxy
    void clearAddress(const bool& flag);

    ///< Return the clearAddress flag.
    bool clearAddress() const;

    ///< this sets the flag whether to consult the provider to update
    /// this transient address if the IOA is not valid.
    void consultProvider(const bool& flag);

    ///< Check the validity of the Transient Address.
    /// If forceUpdate is true, then call @c updateAddress
    /// even if we already have an address.
    /// If ctx is nullptr, we don't try to update the address.
    bool isValid (const EventContext* ctx, bool forceUpdate = false);

    ///< cache the pointer to the Address provider which can update
    ///< this transient address
    IAddressProvider* provider();
    StoreID::type storeID() const;
    void setProvider(IAddressProvider* provider, StoreID::type storeID);

  private:
    TransientAddress(CLID id, const std::string& key, 
		     IOpaqueAddress* addr,
                     bool clearAddress,
                     bool consultProvider);


    // PLEASE NOTE: The data members of this class are ordered so that
    // the most frequently accessed members are grouped together, within
    // the first cache line, when it is embedded in DataProxy.  See the layout
    // at the end of DataProxy.h.
    // Be aware of this when making changes to the layout of this class.


    ///< clid of the concrete class (persistent clid)
    std::atomic<CLID> m_clid;

    ///< (hashed) SG key for primary clid / key.
    std::atomic<sgkey_t> m_sgkey;

    ///< Store type, needed by updateAddress
    StoreID::type m_storeID;

    ///< Controls if IOpaqueAddress should be deleted:
    bool m_clearAddress;

    ///< Control whether the Address Provider must be consulted
    bool m_consultProvider;

    ///< IOpaqueAddress:
    IOpaqueAddress* m_address;

    ///< AddressProvider
    IAddressProvider* m_pAddressProvider;

    ///< string key of this object
    CxxUtils::CachedValue<std::string> m_name;

    ///< all transient clids. They come from symlinks
    TransientClidSet m_transientID; 

    ///< all alias names for a DataObject. They come from setAlias
    TransientAliasSet m_transientAlias;

    static const std::string s_emptyString;
  };
  /////////////////////////////////////////////////////////////////////
  // inlined code:
  /////////////////////////////////////////////////////////////////////

  // Reset the TransientAddress
  inline
  void TransientAddress::reset()
  {
    if (m_clearAddress) setAddress(0);
  }

  /// Retrieve IOpaqueAddress
  inline
  IOpaqueAddress* TransientAddress::address ATLAS_NOT_CONST_THREAD_SAFE () const 
  { 
    return m_address; 
  }

  /// Retrieve clid
  inline
  CLID TransientAddress::clID() const 
  { 
    return m_clid; 
  }

  /// Return StoreGate key
  inline
  const std::string& TransientAddress::name() const
  {
    // Most of the time, m_name is set when the TransientAddress is created
    // and then never changed.  To handle dummy proxies though, m_name
    // can be created as blank and filled in later, but then never changed
    // again.  Used CachedValue to avoid having to use a lock for this.
    if (m_name.isValid()) {
      return *m_name.ptr();
    }
    return s_emptyString;
  }

  /// Get the primary (hashed) SG key.
  inline
  TransientAddress::sgkey_t TransientAddress::sgkey() const
  {
    return m_sgkey;
  }

  /// Set the primary (hashed) SG key.
  inline
  void TransientAddress::setSGKey (sgkey_t sgkey)
  {
    m_sgkey = sgkey;
  }

  /// check if it is a transient ID:
  inline
  bool TransientAddress::transientID(CLID id) const
  {
    return std::find (m_transientID.begin(), m_transientID.end(), id) !=
      m_transientID.end();
  }

  /// get transient CLID's
  inline
  const TransientAddress::TransientClidSet& TransientAddress::transientID() const 
  {
    return m_transientID; 
  }

  /// set transient Alias'
  inline 
  void TransientAddress::setAlias(const std::string& key)
  {
    auto it = std::ranges::lower_bound (m_transientAlias, key);
    if (it == m_transientAlias.end() || *it != key) {
      m_transientAlias.insert (it, key);
    }
  } 

  /// set transient Alias'
  inline 
  void TransientAddress::setAlias(const std::vector<std::string>& keys)
  {
    m_transientAlias = keys;
  } 

  /// set transient Alias'
  inline 
  void TransientAddress::setAlias(std::vector<std::string>&& keys)
  {
    m_transientAlias = std::move(keys);
  } 

  /// remove alias
  inline bool TransientAddress::removeAlias(const std::string& key)
  {
    auto it = std::ranges::lower_bound (m_transientAlias, key);
    if (it != m_transientAlias.end() && *it == key) {
      m_transientAlias.erase (it);
      return true;
    }
    return false;
  }

  /// Test for an alias.
  inline bool TransientAddress::hasAlias(const std::string& key) const
  {
    return std::ranges::binary_search (m_transientAlias, key);
  }

  /// get transient Alias'
  inline
  const TransientAddress::TransientAliasSet& TransientAddress::alias() const
  { 
    return m_transientAlias;
  }

  /// set the clearAddress flag: IOA will not be deleted in proxy
  inline
  void TransientAddress::clearAddress(const bool& flag) 
  {
    m_clearAddress = flag;
  }

  /// Return the clearAddress flag.
  inline
  bool TransientAddress::clearAddress() const
  {
    return m_clearAddress;
  }

  inline
  void TransientAddress::consultProvider(const bool& flag)
  {
    m_consultProvider = flag;
  }

  ///< cache the pointer to the Address provider which can update
  ///< this transient address
  inline
  IAddressProvider* TransientAddress::provider()
  {
    return m_pAddressProvider;
  }
  inline
  StoreID::type TransientAddress::storeID() const
  {
    return m_storeID;
  }

  inline
    void TransientAddress::setProvider(IAddressProvider* provider, 
				       StoreID::type storeID)
  {
    m_pAddressProvider = provider;
    m_consultProvider = true;
    m_storeID=storeID;
  }
} //end namespace SG

#endif // STOREGATE_TRANSIENTADDRESS
