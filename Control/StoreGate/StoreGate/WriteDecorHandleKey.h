// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file StoreGate/WriteDecorHandleKey.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Mar, 2017
 * @brief Property holding a SG store/key/clid/attr name from which a
 *        WriteDecorHandle is made.
 */


#ifndef STOREGATE_WRITEDECORHANDLEKEY_H
#define STOREGATE_WRITEDECORHANDLEKEY_H


#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadHandleKey.h"


class AthAlgorithm;
class AthReentrantAlgorithm;
class AthAlgTool;


namespace SG {


/**
 * @brief Property holding a SG store/key/clid/attr name from which a
 *        WriteDecorHandle is made.
 *
 * This class holds the key part of a WriteDecorHandle.
 *
 * Use this to add a decoration to an object such that the scheduler
 * is aware of it.  This is used just like a @c WriteHandleKey (which see)
 * except that the key string is of the form CCC.DDD, where CCC is the
 * name of the container in StoreGate and DDD is the name of the decoration.
 *
 * Example:
 *
 *@code
 *  class MyAlgorithm : public AthReentrantAlgorithm {
 *    ...
 *    SG::WriteDecorHandleKey<MyCont> m_key{this, "Key",  "container.decor"};
 *  };
 *
 *  StatusCode MyAlgorithm::initialize()
 *  {
 *    ATH_CHECK( m_key.initialize() );
 *    ...
 *  }
 *
 *  StatusCode MyAlgorithm::execute (const EventContext& ctx) const
 *  {
 *    SG::WriteDecorHandle<MyCont, float> h (m_key, ctx);
 *    for (const MyObj& o : *h) {  // Access the container.
 *      h (o) = calculate (o);  // Add the decoration.
 *      ...
 *@endcode
 *
 * Alternatively, one can construct the key with an additional VarHandleKey
 * that represents the container and the key then only holds the decoration
 * name. This is useful to avoid hard-coding the container name twice in case
 * the algorithm also has a @c ReadHandleKey or @c WriteHandleKey for the container:
 *
 * Example:
 *
 *@code
 *  class MyAlgorithm : public AthReentrantAlgorithm {
 *    ...
 *    SG::WriteHandleKey<MyCont> m_whkey{this, "WHKey", "container"};
 *    SG::WriteDecorHandleKey<MyCont> m_wdhkey{this, "Key", m_whkey, "decor"};
 *  };
 *  ...
 *@endcode
 *
 * Implementation note: We want to make two dependencies from this key: a read
 * dependency on the container itself, and a write dependency on the decoration.
 * This class derives from @c WriteHandleKey, which provides the output dependency
 * on the decoration.  We also hold as a member a @c ReadHandleKey for the
 * container.  This extra dependency is added at initialize time via
 * registerWriteDecorHandleKey(), which see.
 */
template <class T>
class WriteDecorHandleKey
  : public WriteHandleKey<T>
{
public:
  typedef WriteHandleKey<T> Base;


  constexpr static bool isDecorHandleKey = true;

  
  /**
   * @brief Constructor.
   * @param key The StoreGate key for the object.
   * @param storeName Name to use for the store, if it's not encoded in sgkey.
   *
   * The provided key may actually start with the name of the store,
   * separated by a "+":  "MyStore+Obj".  If no "+" is present
   * the store named by @c storeName is used.
   */
  explicit
  WriteDecorHandleKey (const std::string& key = "",
                       const std::string& storeName = StoreID::storeName(StoreID::EVENT_STORE));


  /**
   * @brief Constructor with associated container.
   * @param contKey VarHandleKey of the associated container
   * @param decorKey The decoration name.
   * @param storeName Name to use for the store.
   *
   * The decoration @decorKey will be applied on the container referenced
   * by @contKey.
   */
  explicit
  WriteDecorHandleKey (const VarHandleKey& contKey,
                       const std::string& decorKey = "",
                       const std::string& storeName = StoreID::storeName(StoreID::EVENT_STORE));


  /**
   * @brief auto-declaring Property Constructor.
   * @param owner Owning component.
   * @param name name of the Property
   * @param key  default StoreGate key for the object.
   * @param doc Documentation string.
   *
   * will associate the named Property with this WDHK via declareProperty
   *
   * The provided key may actually start with the name of the store,
   * separated by a "+":  "MyStore+Obj".  If no "+" is present
   * the store named by @c storeName is used.
   */
  template <class OWNER,
            typename = typename std::enable_if<std::is_base_of<IProperty, OWNER>::value>::type>
  WriteDecorHandleKey( OWNER* owner,
                      const std::string& name,
                      const std::string& key = {},
                      const std::string& doc = "");

  /**
   * @brief auto-declaring Property Constructor.
   * @param owner Owning component.
   * @param name name of the Property
   * @param contKey VarHandleKey of the associated container
   * @param decorKey name The decoration name.
   * @param doc Documentation string.
   *
   * Will associate the named Property with this WDHK via declareProperty
   * The container part of the decoration key will be taken from @c contKey,
   * while @c decorKey gives the name of the decoration itself.
   * If @c decorKey is blank, then the overall key will be blank.
   */
  template <class OWNER,
            typename = typename std::enable_if<std::is_base_of<IProperty, OWNER>::value>::type>
  WriteDecorHandleKey( OWNER* owner,
                      const std::string& name,
                      const VarHandleKey& contKey,
                      const std::string& decorKey = {},
                      const std::string& doc = "");


  /**
   * @brief Change the key of the object to which we're referring.
   * @param sgkey The StoreGate key for the object.
   * 
   * The provided key may actually start with the name of the store,
   * separated by a "+":  "MyStore+Obj".  If no "+" is present,
   * the store is not changed.
   */
  WriteDecorHandleKey& operator= (const std::string& sgkey);


  /**
   * @brief Change the key of the object to which we're referring.
   * @param sgkey The StoreGate key for the object.
   * 
   * The provided key may actually start with the name of the store,
   * separated by a "+":  "MyStore+Obj".  If no "+" is present
   * the store is not changed.  A key name that starts with a slash
   * is interpreted as a hierarchical key name, not an empty store name.
   *
   * Returns failure the key string format is bad.
   */
  virtual StatusCode assign (const std::string& sgkey) override;

  
  /**
   * @brief If this object is used as a property, then this should be called
   *        during the initialize phase.  It will fail if the requested
   *        StoreGate service cannot be found or if the key is blank.
   * @param used If false, then this handle is not to be used.
   *             Instead of normal initialization, the key will be cleared.
   */
  StatusCode initialize (bool used = true);


  /**
   * @brief If this object is used as a property, then this should be called
   *        during the initialize phase.  This variant will allow the key
   *        to be blank.
   * @param Flag to select this variant.  Call like
   *@code
   *  ATH_CHECK( key.initialize (SG::AllowEmpty) );
   @endcode
   */
  StatusCode initialize (AllowEmptyEnum);


  /**
   * @brief Return the handle key for the container.
   */
  const ReadHandleKey<T>& contHandleKey() const;


  /**
   * @brief Declare that this item does not participate in scheduling.
   *
   * If a WriteDataHandleKey is renounced, then we skip creating the
   * output alias.  Further, we skip the check that the element provided
   * is a member of the declared container.  A WriteDataHandle initialized
   * from a renounced key will effectively behave like a simple Decorator.
   */
  void renounce();


  /**
   * @brief Return the renounced flag.
   */
  bool renounced() const;


private:
  friend class ::AthAlgorithm;
  friend class ::AthReentrantAlgorithm;
  friend class ::AthAlgTool;

  /**
   * @brief Python representation of Handle.
   */
  virtual std::string pythonRepr() const override;

  /**
   * @brief Return the handle key for the container.
   *
   * Should be used only by AthAlgorithm/AthAlgTool.
   */
  ReadHandleKey<T>& contHandleKey_nc();

  /// The container handle.
  ReadHandleKey<T> m_contHandleKey;

  /// Optional container on which decorations are applied
  const VarHandleKey* m_contKey = nullptr;

  /// True if this key has been the target of a renounce().
  /// (In that case, we don't make the output alias.)
  bool m_renounced = false;
};


} // namespace SG


#include "StoreGate/WriteDecorHandleKey.icc"


#endif // not STOREGATE_WRITEDECORHANDLEKEY_H
