// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file StoreGate/SGIterator.h
 * @brief A standard conforming forward iterator over items in StoreGate.
 */


#ifndef STOREGATE_SGITERATOR_H
#define STOREGATE_SGITERATOR_H


#include "StoreGate/exceptions.h"
#include "SGTools/DataProxy.h"
#include "SGTools/DataProxy_cast.h"
#include "SGTools/ProxyMap.h"
#include "AthenaKernel/IProxyDict.h"
#include <boost/iterator/iterator_facade.hpp>
#include <iterator>
#include <concepts>
class SGImplSvc;


namespace SG {


namespace detail {

/// A bidirectional_iterator over DataProxy*.
template <class ITERATOR>
concept DataProxyIterator =
  std::bidirectional_iterator<ITERATOR> && std::same_as<typename std::iterator_traits<ITERATOR>::value_type, DataProxy*>;


/** @class IteratorBase 
 *  Implementation class, not to be used directly
 *  Iterates over valid proxies in the range.
 *  Maintains a reference count on proxies within the range.
 */
class IteratorBase {
protected:
  // Required for boost iterator adapter.
  friend class boost::iterator_core_access;

  // To access setState.
  friend class ::SGImplSvc;


  /**
   * @brief Default constructor.
   */
  IteratorBase() = default;


  /**
   * @brief Constructor from proxy iterator pair.
   * @param itr Starting proxy iterator.
   * @param itrEnd Ending proxy iterator.
   * @param isConst If false, take only non-const proxies.
   *
   * Will skip ahead to the first valid proxy.
   */
  explicit IteratorBase (SG::ConstProxyIterator itr,
                         SG::ConstProxyIterator itrEnd,
                         bool isConst);


  /**
   * @brief Constructor from iterator pair over DataProxy*.
   * @param itr Starting iterator.
   * @param itrEnd Ending iterator.
   * @param isConst If false, take only non-const proxies.
   *
   * Will skip ahead to the first valid proxy.
   */
  template <DataProxyIterator ITERATOR>
  explicit IteratorBase (ITERATOR itr, ITERATOR itrEnd, bool isConst);


  /**
   * @brief Copy constructor.
   */
  IteratorBase (const IteratorBase& other);


  /**
   * @brief Destructor.
   */
  ~IteratorBase();


  /**
   * @brief Assignment.
   */
  IteratorBase& operator= (const IteratorBase& other);

    
  /**
   * @brief Reset state of the iterator.
   * @param itr Starting proxy iterator.
   * @param itrEnd Ending proxy iterator.
   * @param isConst Is this for a const iterator?
   *
   * Will skip ahead to the first valid proxy.
   * Returns FAILURE if the range is empty.
   */
  StatusCode setState (SG::ConstProxyIterator itr,
                       SG::ConstProxyIterator itrEnd,
                       bool isConst);

  
  /**
   * @brief Reset state of the iterator.
   * @param itr Starting  iterator over DataProxy*.
   * @param itrEnd Ending iterator.
   * @param isConst Is this for a const iterator?
   *
   * Will skip ahead to the first valid proxy.
   * Returns FAILURE if the range is empty.
   */
  template <DataProxyIterator ITERATOR>
  StatusCode setState (ITERATOR itr, ITERATOR itrEnd, bool isConst);


  /**
   * @brief Move to the next valid proxy.
   *
   * Used by the boost iterator adapter.
   */
  void increment();


  /**
   * @brief Equality check.
   */
  bool eql(const IteratorBase& rhs) const;


  /**
   * @brief Const check: throw an exception if we're pointing at a const proxy.
   *
   * Called when dereferencing a non-const iterator.
   */
  void const_check() const;


public:
  /**
   * @brief The proxy pointed at by this iterator.
   */
  DataProxy* proxy() const;


  /**
   * @brief Get the key string with which the current object was stored.
   */
  const std::string& key() const;


private:
  /**
   * @brief Add a reference count to all proxies in our range.
   */
  void addRef();


  /**
   * @brief Remove a reference count from all proxies in our range.
   */
  void release();


  /// All proxies in our range, in reverse order.
  std::vector<SG::DataProxy*> m_proxies;
};


} //end ns detail


/**
 * @class TypelessIterator
 * an iterator facade to DataProxy*.  Behaves like a forward iterator.
 */
class TypelessIterator
  : public detail::IteratorBase,
    public boost::iterator_facade<
      TypelessIterator, 
      DataProxy*,
      boost::forward_traversal_tag,
      DataProxy*>
{
public:
  using detail::IteratorBase::IteratorBase;


  /**
   * @brief Default constructor.
   */
  TypelessIterator() = default;


  /**
   * @brief Constructor from proxy iterator pair.
   * @param itr Starting proxy iterator.
   * @param itrEnd Ending proxy iterator.
   *
   * Will skip ahead to the first valid proxy.
   */
  explicit TypelessIterator (SG::ConstProxyIterator itr,
                             SG::ConstProxyIterator itrEnd);


  /**
   * @brief Constructor from iterator pair over DataProxy*.
   * @param itr Starting iterator.
   * @param itrEnd Ending iterator.
   *
   * Will skip ahead to the first valid proxy.
   */
  template <detail::DataProxyIterator ITERATOR>
  explicit TypelessIterator (ITERATOR itr, ITERATOR itrEnd);


  /**
   * @brief Reset state of the iterator.
   * @param itr Starting proxy iterator.
   * @param itrEnd Ending proxy iterator.
   *
   * Will skip ahead to the first valid proxy.
   * Will return failure if the first proxy is const or if the range is empty.
   */
  StatusCode setState (SG::ConstProxyIterator itr,
                       SG::ConstProxyIterator itrEnd);


  /**
   * @brief Reset state of the iterator.
   * @param itr Starting iterator over DataProxy*.
   * @param itrEnd Ending iterator.
   *
   * Will skip ahead to the first valid proxy.
   * Will return failure if the first proxy is const or if the range is empty.
   */
  template <detail::DataProxyIterator ITERATOR>
  StatusCode setState (ITERATOR itr, ITERATOR itrEnd);


private:
  // Required for boost iterator adapter.
  friend class boost::iterator_core_access;
  //INHERITED: void increment();


  /**
   * @brief Equality check.
   *
   * Used by the boost iterator adapter.
   */
  bool equal(const TypelessIterator& rhs) const;


  /**
   * @brief Dereference the iterator.
   *
   * Used by the boost iterator adapter.
   */
  DataProxy* dereference() const;
};


/**
 * @class ConstIterator 
 *  a const_iterator facade to DATA. Behaves like a forward iterator
 */
template <class DATA>
class ConstIterator
  : public detail::IteratorBase, 
    public boost::iterator_facade<
      ConstIterator<DATA>, 
      const DATA,
      boost::forward_traversal_tag > 
{
public:
  /**
   * @brief Default constructor.
   */
  ConstIterator();


  /**
   * @brief Constructor from proxy iterator pair.
   * @param itr Starting proxy iterator.
   * @param itrEnd Ending proxy iterator.
   *
   * Will skip ahead to the first valid proxy.
   */
  explicit ConstIterator (SG::ConstProxyIterator itr,
                          SG::ConstProxyIterator itrEnd);


  /**
   * @brief Reset state of the iterator.
   * @param itr Starting proxy iterator.
   * @param itrEnd Ending proxy iterator.
   *
   * Will skip ahead to the first valid proxy.
   * Will return failure if the range is empty.
   */
  StatusCode setState (SG::ConstProxyIterator itr,
                       SG::ConstProxyIterator itrEnd);


private:
  // Required for boost iterator adapter.
  friend class boost::iterator_core_access;
  // INHERITED: void increment();


  /**
   * @brief Equality check.
   *
   * Used by the boost iterator adapter.
   */
  bool equal (const ConstIterator& rhs) const;


  /**
   * @brief Dereference the iterator.
   *
   * Used by the boost iterator adapter.
   *
   * Will throw SG::ExcInvalidIterator() if the proxy is not valid.
   */
  const DATA& dereference() const;
};


/** @class Iterator 
 *  an iterator facade to DATA. Behaves like a forward iterator
 */
template <class DATA>
class Iterator
  : public detail::IteratorBase, 
    public boost::iterator_facade<
      Iterator<DATA>, 
     DATA,
     boost::forward_traversal_tag > 
{
public:
  /**
   * @brief Default constructor.
   */
  Iterator();


  /**
   * @brief Constructor from proxy iterator pair.
   * @param itr Starting proxy iterator.
   * @param itrEnd Ending proxy iterator.
   *
   * Will skip ahead to the first valid proxy.
   */
  explicit Iterator (SG::ConstProxyIterator itr,
                     SG::ConstProxyIterator itrEnd);


  /**
   * @brief Reset state of the iterator.
   * @param itr Starting proxy iterator.
   * @param itrEnd Ending proxy iterator.
   *
   * Will skip ahead to the first valid proxy.
   * Will return failure if the first proxy is const or if the range is empty.
   */
  StatusCode setState (SG::ConstProxyIterator itr,
                       SG::ConstProxyIterator itrEnd);


private:
  // Required for boost iterator adapter.
  friend class boost::iterator_core_access;
  //INHERITED: void increment();


  /**
   * @brief Equality check.
   *
   * Used by the boost iterator adapter.
   */
  bool equal(const Iterator<DATA>& rhs) const;


  /**
   * @brief Dereference the iterator.
   *
   * Used by the boost iterator adapter.
   *
   * Will throw SG::ExcInvalidIterator() if the proxy is not valid.
   * Will throw SG::ExcConstObject if the proxy is const.
   */
  DATA& dereference() const;
};


} // namespace SG


#include "StoreGate/SGIterator.icc"


#endif
    
