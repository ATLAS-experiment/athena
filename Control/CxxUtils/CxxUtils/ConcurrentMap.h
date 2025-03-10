// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file CxxUtils/ConcurrentMap.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Jan, 2023
 * @brief Hash map from integers/pointers allowing concurrent, lockless reads.
 */


#ifndef CXXUTILS_CONCURRENTMAP_H
#define CXXUTILS_CONCURRENTMAP_H


#include "CxxUtils/ConcurrentHashmapImpl.h"
#include "CxxUtils/UIntConv.h"
#include "CxxUtils/concepts.h"
#include "CxxUtils/IsUpdater.h"
#include "boost/iterator/iterator_facade.hpp"
#include "boost/range/iterator_range.hpp"
#include <type_traits>
#include <stdexcept>


namespace CxxUtils {


/**
 * @brief Hash map from integers/pointers allowing concurrent, lockless reads.
 *
 * This class implements a hash map.  Both the key and mapped types
 * may be pointers or any numeric type that can fit in a uintptr_t.
 * (However, using floating types for the key is not recommended.)
 * It allows for concurrent access from many threads.
 * Reads can proceed without taking out a lock, while writes are serialized
 * with each other.  Thus, this is appropriate for maps which are read-mostly.
 *
 * This is based on ConcurrentHashmapImpl.
 *
 * Besides the mapped key and value types,
 * this class is templated on an UPDATER class, which is used to manage
 * the underlying memory.  See IsUpdater.h for details.
 * (AthenaKernel/RCUUpdater is a concrete version
 * that should work in the context of Athena.)
 *
 * The operations used for calculating the hash of the keys and comparing
 * keys can be customized with the HASHER and MATCHER template arguments
 * (though the defaults should usually be fine).
 *
 * One value of the key must be reserved as a null value, which no real keys
 * may take.  By default this is `0', but it may be customized with
 * the NULLVAL template argument.  The map may optionally support erasures
 * (via tombstones).  To allow this, a second reserved key value must be
 * identified and specified by the TOMBSTONE template argument.
 * Note that the type of the NULLPTR and TOMBSTONE template arguments
 * is a uintptr_t, not KEY.
 *
 * This mostly supports the interface of std::unordered_map, with a few
 * differences / omissions:
 *
 *  - Dereferencing the iterator returns a structure by value, not a reference.
 *  - No try_emplace.
 *  - No insert methods with hints.
 *  - No operator==.
 *  - Nothing dealing with the bucket/node interface or merge().
 *
 * Performance:
 *  This is the result from running the test with --perf on my machine,
 *  with gcc 13.0.0
 *
 * ConcurrentMap
 * lookup:   0.843s wall, 0.840s user + 0.000s system = 0.840s CPU (99.6%)
 * iterate:  0.551s wall, 0.540s user + 0.000s system = 0.540s CPU (97.9%)
 * UnorderedMap
 * lookup:   1.890s wall, 1.870s user + 0.000s system = 1.870s CPU (99.0%)
 * iterate:  0.966s wall, 0.970s user + 0.000s system = 0.970s CPU (100.4%)
 * concurrent_unordered_map
 * lookup:   3.718s wall, 3.690s user + 0.010s system = 3.700s CPU (99.5%)
 * iterate:  5.719s wall, 5.690s user + 0.000s system = 5.690s CPU (99.5%)
 * ck_ht
 * lookup:   1.471s wall, 1.460s user + 0.000s system = 1.460s CPU (99.2%)
 * iterate:  1.519s wall, 1.500s user + 0.000s system = 1.500s CPU (98.7%)
 *
 * The timing for the lookup test of UnorderedMap is probably overly-optimistic,
 * since the test doesn't do any locking in that case.
 */
template <class KEY, class VALUE, template <class> class UPDATER,
          class HASHER = std::hash<KEY>,
          class MATCHER = std::equal_to<KEY>,
          detail::ConcurrentHashmapVal_t NULLVAL = 0,
          detail::ConcurrentHashmapVal_t TOMBSTONE = NULLVAL>
requires (detail::IsConcurrentHashmapPayload<KEY> &&
          detail::IsConcurrentHashmapPayload<VALUE> &&
          detail::IsUpdater<UPDATER> &&
          detail::IsHash<HASHER, KEY> &&
          detail::IsBinaryPredicate<MATCHER, KEY>)
class ConcurrentMap
{
private:
  /// Representation type in the underlying map.
  using val_t = CxxUtils::detail::ConcurrentHashmapVal_t;

  // Translate between the Hasher/Matcher provided (which take KEY arguments)
  // and what we need to give to the underlying implementation
  // (which takes a uintptr_t).
  struct Hasher
  {
    size_t operator() (val_t k) const {
      return m_h (keyAsKey (k));
    }
    HASHER m_h;
  };
  struct Matcher
  {
    bool operator() (val_t a, val_t b) const {
      return m_m (keyAsKey (a), keyAsKey (b));
    }
    MATCHER m_m;
  };

  /// The underlying uint->uint hash table.
  using Impl_t = typename detail::ConcurrentHashmapImpl<UPDATER,
                                                        Hasher,
                                                        Matcher,
                                                        NULLVAL,
                                                        TOMBSTONE>;


public:
  /// Standard STL types.
  using key_type = KEY;
  using mapped_type = VALUE;
  using size_type = size_t;
  /// Updater object.
  using Updater_t = typename Impl_t::Updater_t;
  /// Context type.
  using Context_t = typename Updater_t::Context_t;
  /// Type for external locking.
  using Lock_t = typename Impl_t::Lock_t;

  /// Ensure that the underlying map can store our types.
  static_assert( sizeof(typename Impl_t::val_t) >= sizeof(key_type) );
  static_assert( sizeof(typename Impl_t::val_t) >= sizeof(mapped_type) );


  /**
   * @brief Constructor.
   * @param updater Object used to manage memory
   *                (see comments at the start of the class).
   * @param capacity The initial table capacity.
   *                 (Will be rounded up to a power of two.)
   * @param ctx Execution context.
   */
  ConcurrentMap (Updater_t&& updater,
                 size_type capacity = 64,
                 const Context_t& ctx = Updater_t::defaultContext());

  /** 
   * @brief Constructor from another map.
   * @param updater Object used to manage memory
   *                (see comments at the start of the class).
   * @param capacity The initial table capacity of the new table.
   *                 (Will be rounded up to a power of two.)
   * @param ctx Execution context.
   *
   * (Not really a copy constructor since we need to pass @c updater.)
   */
  ConcurrentMap (const ConcurrentMap& other,
                 Updater_t&& updater,
                 size_t capacity = 64,
                 const Context_t& ctx = Updater_t::defaultContext());


  /** 
   * @brief Constructor from a range.
   * @param f Start iterator for the range.
   * @param l End iterator for the range.
   * @param updater Object used to manage memory
   *                (see comments at the start of the class).
   * @param capacity The initial table capacity of the new table.
   *                 (Will be rounded up to a power of two.)
   * @param ctx Execution context.
   *
   * Constructor from a range of pairs.
   */
  template <class InputIterator>
  ConcurrentMap (InputIterator f,
                 InputIterator l,
                 Updater_t&& updater,
                 size_type capacity = 64,
                 const Context_t& ctx = Updater_t::defaultContext());


  /// Copy / move / assign not supported.
  ConcurrentMap (const ConcurrentMap& other) = delete;
  ConcurrentMap (ConcurrentMap&& other) = delete;
  ConcurrentMap& operator= (const ConcurrentMap& other) = delete;
  ConcurrentMap& operator= (ConcurrentMap&& other) = delete;


  /**
   * @brief Destructor.
   */
  ~ConcurrentMap() = default;


  /**
   * @brief Return the number of items currently in the map.
   */
  size_type size() const;


  /**
   * @brief Test if the map is currently empty.
   */
  bool empty() const;


  /**
   * @brief Return the current size (capacity) of the hash table.
   */
  size_t capacity() const;


  /**
   * @brief The number of erased elements in the current table.
   */
  size_t erased() const;


  /**
   * @brief Value structure for iterators.
   */
  using const_iterator_value = std::pair<const key_type, mapped_type>;


  /**
   * @brief Iterator class.
   *
   * This uses boost::iterator_facade to define the methods required
   * by an STL iterator in terms of the private methods below.
   *
   * Since dereference() will be returning a const_iterator_value by value,
   * we also need to override the reference type.
   */
  class const_iterator
    : public boost::iterator_facade<const_iterator,
                                    const const_iterator_value,
                                    std::bidirectional_iterator_tag,
                                    const const_iterator_value>
  {
  public:
   /**
     * @brief Constructor.
     * @param it Iterator of the underlying table.
     */
    const_iterator (typename Impl_t::const_iterator it);


    /**
     * @brief Test if this iterator is valid.
     *
     * This should be the same as testing for != end().
     */
    bool valid() const;


  private:
    /// Required by iterator_facade.
    friend class boost::iterator_core_access;


    /**
     * @brief iterator_facade requirement: Increment the iterator.
     */
    void increment();


    /**
     * @brief iterator_facade requirement: Decrement the iterator.
     */
    void decrement();


    /**
     * @brief iterator_facade requirement: Equality test.
     */
    bool equal (const const_iterator& other) const;


    /**
     * @brief iterator_facade requirement: Dereference the iterator.
     */
    const const_iterator_value dereference() const;


    /// The iterator on the underlying table.
    typename Impl_t::const_iterator m_impl;
  };


  /// A range defined by two iterators.
  typedef boost::iterator_range<const_iterator> const_iterator_range;


  /**
   * @brief Return an iterator range covering the entire map.
   */
  const_iterator_range range() const;


  /**
   * @brief Iterator at the start of the map.
   */
  const_iterator begin() const;


  /**
   * @brief Iterator at the end of the map.
   */
  const_iterator end() const;


  /**
   * @brief Iterator at the start of the map.
   */
  const_iterator cbegin() const;


  /**
   * @brief Iterator at the end of the map.
   */
  const_iterator cend() const;


  /**
   * @brief Test if a key is in the container.
   * @param key The key to test.
   */
  bool contains (key_type key) const;


  /**
   * @brief Return the number of times a given key is in the container.
   * @param key The key to test.
   *
   * Returns either 0 or 1, depending on whether or not the key is in the map.
   */
  size_type count (key_type key) const;


  /**
   * @brief Look up an element in the map.
   * @param key The element to find.
   *
   * Returns either an iterator referencing the found element or end().
   */
  const_iterator find (key_type key) const;


  /**
   * @brief Look up an element in the map.
   * @param key The element to find.
   *
   * Returns the value associated with the key.
   * Throws @c std::out_of_range if the key does not exist in the map.
   */
  mapped_type at (key_type key) const;


  /**
   * @brief Return a range of iterators with entries matching @c key.
   * @param key The element to find.
   *
   * As keys are unique in this container, this is either a single-element
   * range, or both iterators are equal to end().
   */
  std::pair<const_iterator, const_iterator>
  equal_range (key_type key) const;


  /**
   * @brief Take a lock on the container.
   *
   * Take a lock on the container.
   * The lock can then be passed to insertion methods, allowing to factor out
   * the locking inside loops.  The lock will be released when the
   * lock object is destroyed.
   */
  Lock_t lock();


  /**
   * @brief Add an element to the map.
   * @param key The key of the new item to add.
   * @param val The value of the new item to add.
   * @param ctx Execution context.
   *
   * This will not overwrite an existing entry.
   * The first element in the returned pair is an iterator referencing
   * the added item.  The second is a flag that is true if a new element
   * was added.
   */
  std::pair<const_iterator, bool>
  emplace (key_type key, mapped_type val,
           const Context_t& ctx = Updater_t::defaultContext());


  /**
   * @brief Add an element to the map, with external locking.
   * @param lock The lock object returned from lock().
   * @param key The key of the new item to add.
   * @param val The value of the new item to add.
   * @param ctx Execution context.
   *
   * This will not overwrite an existing entry.
   * The first element in the returned pair is an iterator referencing
   * the added item.  The second is a flag that is true if a new element
   * was added.
   */
  std::pair<const_iterator, bool>
  emplace (const Lock_t& lock,
           key_type key, mapped_type val,
           const Context_t& ctx = Updater_t::defaultContext());


  /**
   * @brief Add an element to the map, or overwrite an existing one.
   * @param key The key of the new item to add.
   * @param val The value of the new item to add.
   * @param ctx Execution context.
   *
   * This will overwrite an existing entry.
   * The first element in the returned pair is an iterator referencing
   * the added item.  The second is a flag that is true if a new element
   * was added.
   */
  std::pair<const_iterator, bool>
  insert_or_assign (key_type key, mapped_type val,
                    const Context_t& ctx = Updater_t::defaultContext());


  /**
   * @brief Add an element to the map, or overwrite an existing one,
   *        with external locking.
   * @param lock The lock object returned from lock().
   * @param key The key of the new item to add.
   * @param val The value of the new item to add.
   * @param ctx Execution context.
   *
   * This will overwrite an existing entry.
   * The first element in the returned pair is an iterator referencing
   * the added item.  The second is a flag that is true if a new element
   * was added.
   */
  std::pair<const_iterator, bool>
  insert_or_assign (const Lock_t& lock,
                    key_type key, mapped_type val,
                    const Context_t& ctx = Updater_t::defaultContext());


  /**
   * @brief Add an element to the map.
   * @param p The item to add.
   *          Should be a pair where first is the key
   *          and second is the integer value.
   *
   * This will not overwrite an existing entry.
   * The first element in the returned pair is an iterator referencing
   * the added item.  The second is a flag that is true if a new element
   * was added.
   *
   * For external locking, use emplace().
   */
  template <class PAIR>
  std::pair<const_iterator, bool> insert (const PAIR& p);


  /**
   * @brief Insert a range of elements to the map.
   * @param first Start of the range.
   * @param last End of the range.
   *
   * The range should be a sequence of pairs where first is the string key
   * and second is the integer value.
   */
  template <class InputIterator>
  void insert (InputIterator first, InputIterator last);


  /**
   * @brief Erase an entry from the table.
   * @param key The key to erase.
   *
   * Mark the corresponding entry as deleted.
   * Return true on success, false on failure (key not found).
   *
   * The tombstone value must be different from the null value.
   *
   * This may cause the key type returned by an iterator to change
   * asynchronously to the tombstone value.
   **/
  bool erase (key_type key);


  /**
   * @brief Erase an entry from the table, with external locking.
   * @param lock The lock object returned from lock().
   * @param key The key to erase.
   *
   * Mark the corresponding entry as deleted.
   * Return true on success, false on failure (key not found).
   *
   * The tombstone value must be different from the null value.
   *
   * This may cause the key type returned by an iterator to change
   * asynchronously to the tombstone value.
   **/
  bool erase (const Lock_t& lock, key_type key);


  /**
   * @brief Increase the table capacity.
   * @param capacity The new table capacity.
   * @param ctx Execution context.
   *
   * No action will be taken if @c capacity is smaller
   * than the current capacity.
   */
  void reserve (size_type capacity,
                const Context_t& ctx = Updater_t::defaultContext());


  /**
   * @brief Increase the table capacity.
   * @param capacity The new table capacity.
   *
   * No action will be taken if @c capacity is smaller
   * than the current capacity.
   */
  void rehash (size_type capacity);


  /**
   * @brief Erase the table and change the capacity.
   * @param capacity The new table capacity.
   * @param ctx Execution context.
   */
  void clear (size_t capacity,
              const Context_t& ctx = Updater_t::defaultContext());


  /**
   * @brief Erase the table (don't change the capacity).
   * @param ctx Execution context.
   */
  void clear (const Context_t& ctx = Updater_t::defaultContext());


  /**
   * @brief Erase the table in-place.
   *
   * This method avoids allocating a new version of the table.
   * However, it is not safe to use concurrently --- no other threads
   * may be accessing the container at the same time, either for read
   * or write.
   */
  void forceClear();


  /**
   * @brief Called when this thread is no longer referencing anything
   *        from this container.
   * @param ctx Execution context.
   */
  void quiescent (const Context_t& ctx);


  /**
   * @brief Swap this container with another.
   * @param other The container with which to swap.
   *
   * This will also call swap on the Updater object; hence, the Updater
   * object must also support swap.  The Hasher and Matcher instances
   * are NOT swapped.
   *
   * This operation is NOT thread-safe.  No other threads may be accessing
   * either container during this operation.
   */
  void swap (ConcurrentMap& other);


  /**
   * @brief Access the Updater instance.
   */
  Updater_t& updater();


private:
  /**
   * @brief Convert an underlying key value to this type's key value.
   * @param val The underlying key value.
   */
  static key_type keyAsKey (val_t val);


  /**
   * @brief Convert this type's key value to an underlying key value.
   * @param k The key.
   */
  static val_t keyAsVal (key_type k);


  /**
   * @brief Convert an underlying mapped value to this type's mapped value.
   * @param val The underlying mapped value.
   */
  static mapped_type mappedAsMapped (val_t val);


  /**
   * @brief Convert this type's mapped value to an underlying mapped value.
   * @param val The mapped value.
   */
  static val_t mappedAsVal (mapped_type val);


  /**
   * @brief Do a lookup in the table.
   * @param key The key to look up.
   *
   * Returns an iterator of the underlying map pointing at the found
   * entry or end();
   */
  typename Impl_t::const_iterator get (key_type key) const;


  /**
   * @brief Insert / overwrite an entry in the table.
   * @param key The key of the new item to add.
   * @param val The value of the new item to add.
   * @param overwrite If true, allow overwriting an existing entry.
   * @param ctx Execution context.
   *
   * The first element in the returned pair is an iterator referencing
   * the added item.  The second is a flag that is true if a new element
   * was added.
   */
  std::pair<const_iterator, bool>
  put (key_type key,
       mapped_type val,
       bool overwrite = true,
       const Context_t& ctx = Updater_t::defaultContext());


  /**
   * @brief Insert / overwrite an entry in the table, with external locking.
   * @param lock The lock object returned from lock().
   * @param key The key of the new item to add.
   * @param val The value of the new item to add.
   * @param overwrite If true, allow overwriting an existing entry.
   * @param ctx Execution context.
   *
   * The first element in the returned pair is an iterator referencing
   * the added item.  The second is a flag that is true if a new element
   * was added.
   */
  std::pair<const_iterator, bool>
  put (const Lock_t& lock,
       key_type key,
       mapped_type val,
       bool overwrite = true,
       const Context_t& ctx = Updater_t::defaultContext());


  /// The underlying hash table.
  Impl_t m_impl;
};


} // namespace CxxUtils


#include "CxxUtils/ConcurrentMap.icc"


#endif // not CXXUTILS_CONCURRENTMAP_H
