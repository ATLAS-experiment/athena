// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file AthContainers/tools/JaggedVecConversions.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Jul, 2024
 * @brief Conversions for accessing jagged vector variables.
 */


#ifndef ATHCONTAINERS_JAGGEDVECCONVERSIONS_H
#define ATHCONTAINERS_JAGGEDVECCONVERSIONS_H


#include "AthContainers/JaggedVecImpl.h"
#include "AthContainers/AuxVectorData.h"
#include "AthContainers/tools/AuxDataTraits.h"
#include "AthContainersInterfaces/IAuxTypeVector.h"
#include "AthContainersInterfaces/AuxDataSpan.h"
#include "AthContainersInterfaces/AuxTypes.h"
#include "CxxUtils/range_with_conv.h"
#include "CxxUtils/ranges.h"
#include <variant>
#include <cassert>
#include <stdexcept>


// For unit testing.
void test_JaggedVecProxyValBase();
void test_JaggedVecProxyRefBase();


namespace SG { namespace detail {


/**
 * @brief Helper: Make a span from a jagged vector element.
 *        An instance of this class is constructed from an @c AuxDataSpan
 *        over the payload vector.  (This has a reference to the range
 *        held by the @c AuxTypeVector, so if the payload vector changes,
 *        we'll see it here.)  @c operator() then takes a @c JaggedVecElt
 *        and produces a span representing that element.
 */
template <class PAYLOAD_T>
class JaggedVecConstConverter
{
public:
  /// The type of the jagged vector payload.
  using Payload_t = PAYLOAD_T;

  /// An @c AuxDataSpan representing the payload vector.
  using const_Payload_span = detail::AuxDataConstSpan<Payload_t>;

  /// The type of the span we produce.
  using element_type = CxxUtils::range_with_conv<typename AuxDataTraits<Payload_t>::const_span>;


  /**
   * @brief Constructor.
   * @param elts Start of the element vector.
   * @param payload Span over the payload vector.
   */
  JaggedVecConstConverter (const JaggedVecEltBase* elts,
                           const_Payload_span payload);


  /**
   * @brief Convert to a span.
   * @param elt The jagged vector element to transform.
   */
  const element_type operator() (const JaggedVecEltBase& elt) const;


private:
  /// The vector of elements.
  const JaggedVecEltBase* m_elts;

  /// The span over the payload vector.
  const_Payload_span m_payload;
};


//****************************************************************************


/**
 * @brief Base class for jagged vector proxies.
 *
 * Proxies are used for modifying elements of a jagged vector.
 * This base class implements the parts that do not depend on payload type.
 */
class JaggedVecProxyBase
{
public:
  /// Type of a jagged vector element.
  using Elt_t = SG::JaggedVecEltBase;

  /// A span of elements.
  using Elt_span = std::span<Elt_t>;

  /// Type used for indexing into a jagged vector.
  using index_type = typename Elt_t::index_type;


  /**
   * @brief Constructor.
   * @param elts The elements of the jagged vector.
   * @param container The container holding this variable.
   * @param auxid The aux ID of this variable.
   */
  JaggedVecProxyBase (const AuxDataSpanBase& elts,
                      AuxVectorData& container,
                      SG::auxid_t auxid);


  /**
   * @brief Return one jagged vector element (non-const).
   * @param elt_index The index of the element.
   */
  Elt_t& elt (size_t elt_index) noexcept;


  /**
   * @brief Return one jagged vector element (const).
   * @param eltindex The index of the element.
   */
  const Elt_t& elt (size_t elt_index) const noexcept;


  /**
   * @brief Resize one jagged vector element.
   * @param elt_index The index of the element to resize.
   * @param n_new The size of the new element.
   *
   * Any added payload elemnets are default-initialized.
   */
  void resize1 (size_t elt_index, index_type n_new);


  /**
   * @brief Add or remove payload items from one jagged vector element.
   * @param elt_index The index of the element to resize.
   * @param index The index of the payload item within this jagged vector
   *              element after the insertion/deletion (in other words,
   *              the first element that moves as a result of the change).
   * @param n_add The number of elements to add.  May be negative
   *              to remove elements.
   */
  void adjust1 (size_t elt_index, index_type index, int n_add);


private:
  /// The elements of the jagged vector.
  Elt_span m_elts;

  /// The container holding the variable.
  AuxVectorData& m_container;

  // Holds either a pointer to the @c IAuxTypeVector the variable ID.
  std::variant<IAuxTypeVector*, SG::auxid_t> m_linkedVec;
};


//****************************************************************************


/**
 * @brief Adapter for holding the proxy base information by value.
 *
 * This is used when creating proxies individually.
 */
class JaggedVecProxyValBase
  : public std::ranges::view_base
{
public:
  /**
   * @brief Constructor.
   * @param elts The elements of the jagged vector.
   * @param container The container holding this variable.
   * @param auxid The aux ID of this variable.
   */
  JaggedVecProxyValBase (const AuxDataSpanBase& elts,
                         AuxVectorData& container,
                         SG::auxid_t auxid);


protected:
  friend void ::test_JaggedVecProxyValBase(); // For unit testing.

  /// The proxy base information.
  JaggedVecProxyBase m_base;
};


/**
 * @brief Adapter for holding the proxy base information by reference.
 *
 * This is used when creating proxies as part of a span.
 */
class JaggedVecProxyRefBase
  : public std::ranges::view_base
{
public:
  /**
   * @brief Constructor.
   * @param base The base information.
   */
  JaggedVecProxyRefBase (JaggedVecProxyBase& base);


protected:
  friend void ::test_JaggedVecProxyRefBase(); // For unit testing.

  /// The proxy base information.
  JaggedVecProxyBase& m_base;
};


//****************************************************************************


/**
 * @brief Proxy for jagged vectors.
 *
 * We want to be able to do things like:
 *@code
 *  SG::Accessor<SG::JaggedVecElt<int> > acc ("jvec");
 *  acc(*e)[1] = 3;
 @endcode
 *
 * That means that the @c Accessor needs to return a proxy object
 * that implements vector-like operations for modifying the
 * jagged vector data.  This is the purpose of this proxy class.
 *
 * In the case where we're dealing with a span over jagged vector elements,
 * we'd like each proxy to share the type-independent base data
 * (@c JaggedVectorProxyBase).  In that case, for example, we only need
 * to fetch the @c IAuxTypeVector pointer once for everything in the span.
 * But if the proxy is used by itself, then we don't gain anything
 * from that, but we would have to pay the price of another layer
 * of indirection.  So the dependency on this data is factored out
 * into the @c BASE base class, which can hold that data either
 * by reference or by value.
 */
template <class PAYLOAD_T, class BASE>
class JaggedVecProxyT
  : public BASE
{
public:
  /// The payload type.
  using Payload_t              = PAYLOAD_T;
  using Payload_span           = detail::AuxDataSpan<Payload_t>;

  /// Type of one jagged vector element.  We use the type-independent base.
  using Elt_t                  = JaggedVecProxyBase::Elt_t;
  using index_type             = typename Elt_t::index_type;
  using Elt_span               = JaggedVecProxyBase::Elt_span;
  static_assert (sizeof (Elt_t) == sizeof (SG::JaggedVecElt<Payload_t>));

  /// Standard type aliases.
  using element_type           = Payload_t;
  using value_type             = std::remove_cv_t<element_type>;
  using size_type              = std::size_t;
  using difference_type        = std::ptrdiff_t;
  using pointer                = element_type*;
  using const_pointer          = const element_type*;
  using reference              = element_type&;
  using const_reference        = const element_type&;

  /// Use pointers as iterators.
  using iterator               = pointer;
  using const_iterator         = const_pointer;
  using reverse_iterator       = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;


  /**
   * @brief Helper for passing iterators.
   *
   *  For convenience, we want to allow passing references to payload items
   *  both as STL-like iterators and integer indices.  But we run into
   *  a problem.  Since our iterators are pointers and there is an implicit
   *  conversion from `0' to pointers, we can run into ambiguities
   *  if these methods are called using the constant 0 as an index.
   *  To fix this, we pass iterators using this wrapper, which will
   *  not allow initialization from `0' (or nullptr for that matter).
   */
  struct nonnull_iterator
  {
    nonnull_iterator (iterator it) : m_it (it) {}
    nonnull_iterator (std::nullptr_t) = delete;
    nonnull_iterator (int) = delete;
    operator iterator() { return m_it; }
    iterator m_it;
  };


  /**
   * @brief Constructor.
   * @param elt_index Index of the jagged vector element.
   * @param payload Reference to a range over the payload vector.
   * @param elts The elements of the jagged vector.
   * @param container The container holding this variable.
   * @param auxid The aux ID of this variable.
   */
  JaggedVecProxyT (size_t elt_index,
                   Payload_span payload,
                   const AuxDataSpanBase& elts,
                   AuxVectorData& container,
                   SG::auxid_t auxid);


  /**
   * @brief Constructor.
   * @param elt_index Index of the jagged vector element.
   * @param payload Reference to a range over the payload vector.
   * @param base The base information.
   */
  JaggedVecProxyT (size_t elt_index,
                   Payload_span payload,
                   JaggedVecProxyBase& base);

  /**
   * @brief Return the size of this jagged vector element.
   */
  size_t size() const noexcept;


  /**
   * @brief Test if this jagged vector element is empty.
   */
  bool empty() const noexcept;


  /**
   * @brief Element access (non-const).
   * @param i Index within this element's payload.
   */
  reference operator[] (size_t i) noexcept;


  /**
   * @brief Element access (const).
   * @param i Index within this element's payload.
   */
  const_reference operator[] (size_t i) const noexcept;


  /**
   * @brief Element access (non-const, bounds-checked).
   * @param i Index within this element's payload.
   */
  reference at (size_t i);


  /**
   * @brief Element access (const, bounds-checked).
   * @param i Index within this element's payload.
   */
  const_reference at (size_t i) const;


  /**
   * @brief Return a reference to the first item in this element's payload.
   */
  reference front() noexcept;


  /**
   * @brief Return the first item in this element's payload.
   */
  const_reference front() const noexcept;


  /**
   * @brief Return a reference to the last item in this element's payload.
   */
  reference back() noexcept;


  /**
   * @brief Return the last item in this element's payload.
   */
  const_reference back() const noexcept;


  /**
   * @brief Return a (non-const) begin iterator.
   */
  iterator begin() noexcept;


  /**
   * @brief Return a (non-const) end iterator.
   */
  iterator end() noexcept;


  /**
   * @brief Return a (const) begin iterator.
   */
  const_iterator begin() const noexcept;


  /**
   * @brief Return a (const) end iterator.
   */
  const_iterator end() const noexcept;


  /**
   * @brief Return a (const) begin iterator.
   */
  const_iterator cbegin() const noexcept;


  /**
   * @brief Return a (const) end iterator.
   */
  const_iterator cend() const noexcept;


  /**
   * @brief Return a (non-const) begin reverse iterator.
   */
  reverse_iterator rbegin() noexcept;


  /**
   * @brief Return a (non-const) end reverse iterator.
   */
  reverse_iterator rend() noexcept;


  /**
   * @brief Return a (const) begin reverse iterator.
   */
  const_reverse_iterator rbegin() const noexcept;


  /**
   * @brief Return a (const) end reverse iterator.
   */
  const_reverse_iterator rend() const noexcept;


  /**
   * @brief Return a (const) begin reverse iterator.
   */
  const_reverse_iterator crbegin() const noexcept;


  /**
   * @brief Return a (const) end reverse iterator.
   */
  const_reverse_iterator crend() const noexcept;


  /**
   * @brief Assign this jagged vector element from a range.
   * @param range The range from which to assign.
   */
  template <CxxUtils::InputRangeOverT<PAYLOAD_T> RANGE>
  JaggedVecProxyT& operator= (const RANGE& range);


  /**
   * @brief Convert this jagged vector element to a vector.
   */
  std::vector<Payload_t> asVector() const;


  /**
   * @brief Convert this jagged vector element to a vector.
   */
  template <class VALLOC>
  operator std::vector<Payload_t, VALLOC>() const;


  /**
   * @brief Add an item to the end of this jagged vector element.
   * @param x The item to add.
   */
  void push_back (const Payload_t& x);


  /**
   * @brief Clear this jagged vector element.
   */
  void clear();


  /**
   * @brief Resize this jagged vector element.
   * @param n New size for the element.
   * @param x If the element is being enlarged, initialize the new
   *          elements with this value.
   */
  void resize (size_t n, const Payload_t& x = Payload_t());


  /**
   * @brief Erase one item from this jagged vector element.
   * @param pos Position within the element of the item to erase.
   */
  void erase (nonnull_iterator pos);


  /**
   * @brief Erase one item from this jagged vector element.
   * @param pos Index within the element of the item to erase.
   */
  void erase (index_type pos);


  /**
   * @brief Erase a range of items from this jagged vector element.
   * @param first Position within the element of the first item to erase.
   * @param last One past the position within the element of the last item to erase.
   */
  void erase (nonnull_iterator first, nonnull_iterator last);


  /**
   * @brief Erase a range of items from this jagged vector element.
   * @param first Index within the element of the first item to erase.
   * @param last One past the index within the element of the last item to erase.
   */
  void erase (index_type first, index_type last);


  /**
   * @brief Insert an item into this jagged vector element.
   * @param pos Position within the element at which to insert.
   * @param x The item to insert.
   */
  void insert (nonnull_iterator pos, const Payload_t& x);


  /**
   * @brief Insert an item into this jagged vector element.
   * @param pos Index within the element at which to insert.
   * @param x The item to insert.
   */
  void insert (index_type pos, const Payload_t& x);


  /**
   * @brief Insert a number of items into this jagged vector element.
   * @param pos Position within the element at which to insert.
   * @param n The number of items to insert.
   * @param x The value to insert.
   */
  void insert (nonnull_iterator pos, size_t n, const Payload_t& x);


  /**
   * @brief Insert a number of items into this jagged vector element.
   * @param pos Index within the element at which to insert.
   * @param n The number of items to insert.
   * @param x The value to insert.
   */
  void insert (index_type pos, size_t n, const Payload_t& x);


  /**
   * @brief Insert a range of items into this jagged vector element.
   * @param pos Position within the element at which to insert.
   * @param first Start of the range to insert.
   * @param last End of the range to insert.
   */
  template <CxxUtils::detail::InputValIterator<PAYLOAD_T> ITERATOR>
  void insert (nonnull_iterator pos, ITERATOR first, ITERATOR last);


  /**
   * @brief Insert a range of items into this jagged vector element.
   * @param pos Index within the element at which to insert.
   * @param first Start of the range to insert.
   * @param last End of the range to insert.
   */
  template <CxxUtils::detail::InputValIterator<PAYLOAD_T> ITERATOR>
  void insert (index_type pos, ITERATOR first, ITERATOR last);


  /**
   * @brief Insert a range of items into this jagged vector element.
   * @param pos Position within the element at which to insert.
   * @param range The range to insert.
   */
  template <CxxUtils::InputRangeOverT<PAYLOAD_T> RANGE>
  void insert_range (nonnull_iterator pos, const RANGE& range);


  /**
   * @brief Insert a range of items into this jagged vector element.
   * @param pos Index within the element at which to insert.
   * @param range The range to insert.
   */
  template <CxxUtils::InputRangeOverT<PAYLOAD_T> RANGE>
  void insert_range (index_type pos, const RANGE& range);


  /**
   * @brief Append a range of items to the end of this jagged vector element.
   * @param range The range to append.
   */
  template <CxxUtils::InputRangeOverT<PAYLOAD_T> RANGE>
  void append_range (const RANGE& range);


  /**
   * @brief Remove the last item from this jagged vector element.
   */
  void pop_back();


  /**
   * @brief Set this jagged vector element to a number of items.
   * @param n The number of items to assign.
   * @param x The value to assign.
   */
  void assign (size_t n, const Payload_t& x);


  /**
   * @brief Assign this jagged vector element from a range.
   * @param first Start of the range to assign.
   * @param last End of the range to assign.
   */
  template <CxxUtils::detail::InputValIterator<PAYLOAD_T> ITERATOR>
  void assign (ITERATOR first, ITERATOR last);


  /**
   * @brief Assign this jagged vector element from a range.
   * @param range The range from which to assign.
   */
  template <CxxUtils::InputRangeOverT<PAYLOAD_T> RANGE>
  void assign_range (const RANGE& range);


private:
  /**
   * @brief Return a reference to this proxy's element.
   */
  Elt_t& elt() noexcept;


  /**
   * @brief Return a (const) reference to this proxy's element.
   */
  const Elt_t& elt() const noexcept;


  /**
   * @brief Return the begin payload index of this proxy's element.
   */
  size_t elt_begin() const noexcept;


  /**
   * @brief Return the end payload index of this proxy's element.
   */
  size_t elt_end() const noexcept;


  /// Index of the element we're proxying.
  size_t m_index;

  /// Reference to a span over the payload vector.
  Payload_span m_payload;
};


//****************************************************************************


/**
 * @brief Helper: Make a span --- either read-only or writable ---
 *        from a jagged vector element.
 *        An instance of this class is constructed from an @c AuxDataSpan
 *        over the payload vector.  (This has a reference to the range
 *        held by the @c AuxTypeVector, so if the payload vector changes,
 *        we'll see it here.)  @c operator() then takes a @c JaggedVecElt
 *        and produces a span representing that element.
 */
template <class PAYLOAD_T>
class JaggedVecConverter
{
public:
  /// The type of the jagged vector payload.
  using Payload_t = PAYLOAD_T;

  /// An @c AuxDataSpan representing the payload vector.
  using Payload_span = detail::AuxDataSpan<Payload_t>;

  /// The type of the (read-only) span we produce.
  using element_type = CxxUtils::range_with_conv<typename AuxDataTraits<Payload_t>::const_span>;


  /// The type of the (writable) span we produce.
  using JVecProxyInSpan = JaggedVecProxyT<Payload_t, JaggedVecProxyRefBase>;
  using value_type = JVecProxyInSpan;


  /**
   * @brief Constructor.
   * @param container The container holding this variable.
   * @param auxid The aux ID of this variable.
   * @param linked_auxid The aux ID for the linked payload variable.
   */
  JaggedVecConverter (AuxVectorData& container,
                      auxid_t auxid,
                      auxid_t linked_auxid);


  /**
   * @brief Convert to a (read-only) span.
   * @param elt The jagged vector element to transform.
   */
  const element_type operator() (const SG::JaggedVecEltBase& elt) const;


  /**
   * @brief Convert to a (writable) span.
   * @param elt The jagged vector element to transform.
   */
  value_type operator() (SG::JaggedVecEltBase& elt);


private:
  /// The proxy base information.
  JaggedVecProxyBase m_base;

  /// The span over the payload vector.
  Payload_span m_payload;
};


} } // namespace SG::detail


#include "AthContainers/tools/JaggedVecConversions.icc"


#endif // not ATHCONTAINERS_JAGGEDVECCONVERSIONS_H
