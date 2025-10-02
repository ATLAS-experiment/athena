/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "AthViews/View.h"
#include "TrigSteeringEvent/TrigRoiDescriptorCollection.h"

#include <stdexcept>


/**
 * @brief Create a new View instance
 * @param name Name of the view
 * @param index Index of the view (gets appended to the name if index >= 0)
 * @param allowFallThrough Allow fall-back to default store if object not found within the view
 * @param storeName Store name
 */
SG::View::View( std::string const& name, int index, bool allowFallThrough, std::string const& storeName ) :
  m_store( storeName, name ),
  m_keyMap(KeyMap_t::Updater_t()),
  m_name( name ),
  m_allowFallThrough( allowFallThrough )
{
  if ( index >= 0 ) {
    m_index = index;
    m_name += '_';
    m_name += std::to_string( index );
  }
}


/**
 * @brief Link to the previously used views.
 */
void SG::View::linkParent( const IProxyDict* parent ) {
  auto castParent = dynamic_cast< const SG::View* >( parent );
  if ( castParent ) {
    m_parents.insert( castParent );
  }
  else {
    throw std::runtime_error( "Unable to link parent view that cannot be cast to SG::View" );
  }
}


/**
 * @brief Record an object in the store.
 * @param obj The data object to store.
 * @param key The key as which it should be stored.
 * @param allowMods If false, the object will be recorded as const.
 *
 * Full-blown record.  @c obj should usually be something
 * deriving from @c SG::DataBucket.
 *
 * Returns the proxy for the recorded object; nullptr on failure.
 */
SG::DataProxy* SG::View::recordObject ( SG::DataObjectSharedPtr<DataObject> obj,
                                        const std::string& key,
                                        bool allowMods,
                                        bool returnExisting)
{
  // Record the object under the view name
  SG::DataProxy* proxy = m_store->recordObject( obj, viewKey(key), allowMods, returnExisting );

  // Remember the hashed rawKey -> viewKey mapping for use in proxy_exact
  if (proxy) {
    const IStringPool::sgkey_t keyNoView = m_store->stringToKey( key, obj->clID() );
    m_keyMap.emplace(keyNoView, proxy->sgkey());
  }

  return proxy;
}


/**
 * @brief Get proxy given a hashed key+clid.
 * @param sgkey Hashed key to look up.
 *
 * Find an exact match; no handling of aliases, etc.
 * Returns 0 to flag failure.
 *
 * @note
 * The implementation of proxy_exact is a bit special for the view case.
 * This method is called by the Read/WriteHandle to avoid lengthy lookups via the
 * string key. However, the @c sgkey that is passed is the hashed key of the
 * original HandleKey (without the view prefix). So whenever we store an object in
 * a view, we calculate the raw hashed key in @c recordObject() and store a mapping to
 * the hashed view key to be able to retrieve the correct proxy here.
 */
SG::DataProxy* SG::View::proxy_exact ( SG::sgkey_t sgkey ) const {
  auto itr = m_keyMap.find(sgkey);
  if (itr != m_keyMap.end()) {
    return m_store->proxy_exact( itr->second );
  }
  else return nullptr;
}


/**
 * @brief Get proxy with given id and key.
 * @param id The @c CLID of the desired object.
 * @param key The key of the desired object.
 *
 * If the key is a null string, then it is a @em default key.
 * Finding a proxy via the default key should succeed only if there
 * is exactly one object with the given @c CLID in the store.
 * Finding a proxy via a default key is considered deprecated
 * for the case of the event store.
 * 
 * Returns 0 to flag failure
 */
SG::DataProxy * SG::View::proxy( const CLID& id, const std::string& key ) const
{
  return findProxy( id, key, m_allowFallThrough );
}


/**
 * @brief Internal implementation of proxy()
 */
SG::DataProxy * SG::View::findProxy( const CLID& id, const std::string& key, bool allowFallThrough ) const
{
  auto isValid = [](const SG::DataProxy* p) { return p != nullptr and p->isValid(); };
  auto localProxy = m_store->proxy( id, viewKey(key) );
  if ( isValid( localProxy ) ) {
    return localProxy;
  }

  for ( auto parent: m_parents ) {
    // Don't allow parents to access whole-event store independently of this view
    auto inParentProxy = parent->findProxy( id, key, false );
    if ( isValid( inParentProxy ) ) {
      return inParentProxy;
    }
  }
  
  //Look in the default store if could not find in any view - for instance for event-wise IDCs
  if ( (not isValid( localProxy )) and allowFallThrough ) {

    //Apply filtering
    if ( !m_fallFilter.empty() ) {
      bool filterPass = false;

      //Filter passes if the key contains one of the possible values
      for ( auto& entry : m_fallFilter ) {
        if ( key.find( entry ) != std::string::npos ) {
          filterPass = true;
          break;
        }
      }

      if ( !filterPass ) return nullptr;
    }

    return m_store->proxy( id, key );
  }
  return nullptr;
}


/**
 * @brief Print content of the view.
 */
std::string SG::View::dump( const std::string& indent ) const
{
  // Dump view contents
  std::string ret = indent + "Dump " + name() + "\n";
  ret += indent + "[";
  for ( const SG::DataProxy* dp: proxies() ) {
    if ( dp->name().find( name() ) == 0 ) 
      ret += " " + dp->name();
  }
  ret += " ]\n";

  // Dump parent views
  if ( m_parents.size() ) ret += indent + "Parents:\n";
  for ( auto p : m_parents ) {
    ret += p->dump( indent + "  " );
  }

  // Fallthrough
  if ( indent == "" ) {
    if ( m_allowFallThrough ) {
      ret += indent + "May access main store: " + m_store->name();
    } else {
      ret += indent + "May not access main store";
    }
  }
  return ret;
}


bool SG::View::tryELRemap( sgkey_t, size_t, sgkey_t&, size_t& )
{
  throw std::runtime_error( "Not implemented: SG::View::tryELRemap" );
}

const std::string* SG::View::keyToString( IStringPool::sgkey_t, CLID& ) const
{
  throw std::runtime_error( "Not implemented: SG::View::keyToString" );
}

const std::string* SG::View::keyToString( IStringPool::sgkey_t ) const
{
  throw std::runtime_error( "Not implemented: SG::View::keyToString" );
}

