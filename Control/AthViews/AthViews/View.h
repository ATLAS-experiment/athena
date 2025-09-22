/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHVIEWS_VIEW_H
#define ATHVIEWS_VIEW_H

#include "GaudiKernel/ServiceHandle.h"

#include "AthenaKernel/CLASS_DEF.h"
#include "AthenaKernel/IProxyDict.h"
#include "AthContainers/DataVector.h"
#include "AthLinks/ElementLink.h"
#include "SGTools/transientKey.h"
#include "StoreGate/StoreGateSvc.h"
#include "TrigSteeringEvent/TrigRoiDescriptorCollection.h"

#include <set>
#include <vector>

// Forward declarations
namespace SG {
  class DataProxy;
}
class DataObject;


namespace SG {

  /**
   * @brief A "view" of the event store (IProxyDict).
   *
   * This class provides a view of the event store by mangling (prefixing) the key name
   * with a fixed string (see @c viewKey()). Usually the view is carried by the (extended)
   * @c EventContext and automatically applied for algorithms running within a view.
   * For creating/accessing objects from outside the view, the view needs to be explicitly
   * set via @c VarHandleBase::setProxyDict() on the handle (see AthViews/ViewHelper.h).
   *
   * The lookup of an object proceeds in the following order:
   *   1. the current view
   *   2. the parent view if linked
   *   3. the full store if @c allowFallThrough is set
   */
  class View final : public implements<IProxyDict> {
  public:
    /**
     * @brief Create a new View instance
     * @param name Name of the view
     * @param index Index of the view (gets appended to the name if index >= 0)
     * @param allowFallThrough Allow fall-back to default store if object not found within the view
     * @param storeName Store name
     */
    View( const std::string& name, int index, bool allowFallThrough = true,
          const std::string& storeName = "StoreGateSvc" );

    View() = delete;
    virtual ~View() = default;
    View (const View&) = delete;
    View& operator= (const View&) = delete;

    /**
     * @brief Return view index.
     */
    size_t viewID() const {
      return m_index;
    }

    /**
     * @brief Link to the previously used views.
     */
    void linkParent( const IProxyDict* parent );

    /**
     * @brief Returns the links to the previously used views.
     */
    const std::set< const SG::View* >& getParentLinks() const {
      return m_parents;
    }

    /**
     * @brief Set a filtering rule for anything loaded via fall-through.
     * @param inputFilter Only allow keys containing one these strings to fall-through.
     */
    void setFilter( std::vector< std::string > const& inputFilter ) {
      m_fallFilter = inputFilter;
    }

    /**
     * @brief Associated RoI with this view.
     */
    void setROI(const ElementLink<TrigRoiDescriptorCollection>& roi) {
      m_roi = roi;
    }

    /**
     * @brief Return associated RoI.
     */
    const ElementLink<TrigRoiDescriptorCollection>& getROI() const {
      return m_roi;
    }

    /**
     * @brief Print content of the view.
     */
    std::string dump( const std::string& indent = "" ) const;


    /**
     * @{ @name IProxyDict interface
     */

    /**
     * @brief Name of the view
     */
    virtual const std::string& name() const override {
      return m_name;
    }

    /**
     * @brief Get proxy given a hashed key+clid.
     * @param sgkey Hashed key to look up.
     *
     * Find an exact match; no handling of aliases, etc.
     * Returns 0 to flag failure.
     */
    virtual SG::DataProxy* proxy_exact(SG::sgkey_t sgkey) const override;

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
    virtual SG::DataProxy* proxy(const CLID& id, const std::string& key) const override;

    /**
     * @brief Get a proxy referencing a given transient object.
     * @param pTransient The object to find.
     *
     * Returns 0 to flag failure
     */
    virtual SG::DataProxy* proxy(const void* const pTransient) const override {
      return m_store->proxy( pTransient );
    }

    /**
     * @brief Return the list of all current proxies in store.
     */
    virtual std::vector<const SG::DataProxy*> proxies() const override {
      return m_store->proxies();
    }

    /**
     * @brief Add a new proxy to the store.
     * @param id CLID as which the proxy should be added.
     * @param proxy The proxy to add.
     *
     * Simple addition of a proxy to the store.  The key is taken as the
     * primary key of the proxy.  Does not handle things
     * like overwrite, history, symlinks, etc.  Should return failure
     * if there is already an entry for this clid/key.
     */
    virtual StatusCode addToStore(CLID id, SG::DataProxy* proxy) override {
      return m_store->addToStore( id, proxy );
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
    virtual SG::DataProxy* recordObject ( SG::DataObjectSharedPtr<DataObject> obj,
                                          const std::string& key,
                                          bool allowMods,
                                          bool returnExisting) override;

    /**
     * @brief Tell the store that a handle has been bound to a proxy.
     * @param handle The handle that was bound.
     */
    virtual void boundHandle (IResetable* handle) override {
      return m_store->boundHandle( handle );
    }

    /**
     * @brief Tell the store that a handle has been unbound from a proxy.
     * @param handle The handle that was unbound.
     */
    virtual void unboundHandle (IResetable* handle) override {
      return m_store->unboundHandle( handle );
    }

    /**
     * @brief Test to see if the target of an ElementLink has moved.
     * @param sgkey_in Original hashed key of the EL.
     * @param index_in Original index of the EL.
     * @param sgkey_out[out] New hashed key for the EL.
     * @param index_out[out] New index for the EL.
     * @return True if there is a remapping; false otherwise.
     *
     * Not supported. Will throw std::runtime_error.
     */
    virtual bool tryELRemap ( sgkey_t sgkey_in,   size_t index_in,
                              sgkey_t& sgkey_out, size_t& index_out) override;
    /**@}*/


    /**
     * @{ @name IStringPool interface
     */

    /**
     * @brief Find the string and CLID corresponding to a given key.
     *
     * Not supported. Will throw std::runtime_error.
     */
    virtual const std::string* keyToString( IStringPool::sgkey_t key ) const override;

    /**
     * @brief Find the string corresponding to a given key.
     *
     * Not supported. Will throw std::runtime_error.
     */
    virtual const std::string* keyToString( IStringPool::sgkey_t key, CLID& clid ) const override;

    /**
     * @brief Find the key for a string/CLID pair.
     */
    virtual IStringPool::sgkey_t stringToKey( const std::string& str, CLID clid ) override {
      return m_store->stringToKey( viewKey(str), clid );
    }

    /**
     * @brief Remember an additional mapping from key to string/CLID.
     */
    virtual void registerKey( IStringPool::sgkey_t key, const std::string& str, CLID clid ) override {
      m_store->registerKey( key, viewKey(str), clid );
    }
    /**@}*/


  private:
    /**
     * @brief Internal implementation of proxy()
     */
    SG::DataProxy* findProxy( const CLID& id, const std::string& key, bool allowFallThrough ) const;

    /**
     * @brief Construct a key as used in the parent store.
     * @param key The key as used in the view.
     */
    std::string viewKey (const std::string& key) const {
      return SG::transientKey (m_name + "_" + key);
    }

    ServiceHandle< StoreGateSvc > m_store;
    ElementLink<TrigRoiDescriptorCollection> m_roi;

    std::set< const SG::View* > m_parents;
    std::vector< std::string > m_fallFilter;

    std::string m_name;
    size_t m_index{0};
    bool m_allowFallThrough{true};
  };
}  // namespace SG


/**
 * @brief View container for recording in StoreGate
 */
typedef DataVector<SG::View> ViewContainer;

CLASS_DEF( ViewContainer , 1160627009 , 1 )

#endif
