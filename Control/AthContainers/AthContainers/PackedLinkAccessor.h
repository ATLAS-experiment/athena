// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file AthContainers/PackedLinkAccessor.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Oct, 2023
 * @brief Helper class to provide type-safe access to aux data,
 *        specialized for @c PackedLink.
 */


#ifndef ATHCONTAINERS_PACKEDLINKACCESSOR_H
#define ATHCONTAINERS_PACKEDLINKACCESSOR_H


#include "AthContainersInterfaces/AuxTypes.h"
#include "AthContainersInterfaces/IAuxElement.h"
#include "AthContainers/PackedLinkConstAccessor.h"
#include "AthContainers/Accessor.h"
#include "AthContainers/AuxVectorData.h"
#include "AthContainers/AuxTypeRegistry.h"
#include "AthContainers/tools/ELProxy.h"
#include "AthContainers/tools/AuxDataTraits.h"
#include "CxxUtils/checker_macros.h"
#include <string>
#include <typeinfo>


namespace SG {


/**
 * @brief Helper class to provide constant type-safe access to aux data,
 *        specialized for PackedLink.
 *
 * This is a version of @c Accessor, specialized for @c PackedLink.
 *
 * Although the type argument is @c PackedLink<CONT>, the objects that
 * this accessor produces are @c ElementLink<CONT> (returned by value
 * rather than by const reference).  The @c getDataSpan method will
 * then produce a (writable) span over @c ElementLink<CONT>.
 * (There are separate methods for returning spans over the
 * PackedLink/DataLink arrays themselves.)
 *
 * You might use this something like this:
 *
 *@code
 *   // Only need to do this once.
 *   using Cont_t = std::vector<int>;
 *   static const SG::Accessor<SG::PackedLink<Cont_t> links ("links");
 *   ...
 *   DataVector<MyClass>* v = ...;
 *   Myclass* m = v->at(2);
 *   links (*m) = ElementLink<Cont_t> (...);
 *   auto span = links.getDataSpan (*v);
 *   span[2] = ElementLink<Cont_t> (...);
 @endcode
 *
 * You can also use this to define getters/setters in your class:
 *
 *@code
 *  class Myclass {
 *    ...
 *    ElementLink<Cont_t> get_links() const
 *    { const static SG::ConstAccessor<SG::PackedLink<Cont_t> > acc ("links");
 *      return acc (*this); }
 *    void set_links (const ElementLink<Cont_t>& el)
 *    { const static SG::Accessor<SG::PackedLink<Cont_t> > acc ("links");
 *      acc (*this) = el; }
 @endcode
*/
template <class CONT, class ALLOC>
class Accessor<PackedLink<CONT>, ALLOC>
  : public ConstAccessor<PackedLink<CONT>, ALLOC>
{
public:
  // Aliases for the types we're dealing with.
  using Base = ConstAccessor<PackedLink<CONT>, ALLOC>;
  using Link_t = ElementLink<CONT>;
  using PLink_t = SG::PackedLink<CONT>;
  using DLink_t = DataLink<CONT>;
  using DLinkAlloc_t = typename std::allocator_traits<ALLOC>::template rebind_alloc<DLink_t>;


  /// Spans over the objects that are actually stored.
  using PackedLink_span = typename AuxDataTraits<PackedLink<CONT>, ALLOC>::span;
  using DataLink_span = typename AuxDataTraits<DataLink<CONT>, DLinkAlloc_t>::span;


  /// Writable proxy for @c PackedLink appearing like an @c ElementLink.
  using ELProxy = detail::ELProxyT<detail::ELProxyValBase<CONT> >;


  /// Transform a non-const span of @c PackedLink to a range
  /// of @c ElementLink proxies.
  using span = CxxUtils::transform_view_with_at<PackedLink_span,
                                                detail::ELProxyInSpanConverter<CONT> >;


  /// Type the user sees.
  using element_type = Link_t;

  /// Type referencing an item.
  using reference_type = ELProxy;

  /// Not supported.
  using container_pointer_type = void;
  using const_container_pointer_type = void;


  /// Take these from the base class.
  using Base::operator();
  using Base::getPackedLinkArray;
  using Base::getDataLinkArray;
  using Base::getPackedLinkSpan;
  using Base::getDataLinkSpan;
  using Base::getDataSpan;


  /**
   * @brief Constructor.
   * @param name Name of this aux variable.
   *
   * The name -> auxid lookup is done here.
   */
  Accessor (const std::string& name);


  /**
   * @brief Constructor.
   * @param name Name of this aux variable.
   * @param clsname The name of its associated class.  May be blank.
   *
   * The name -> auxid lookup is done here.
   */
  Accessor (const std::string& name, const std::string& clsname);


  /**
   * @brief Constructor taking an auxid directly.
   * @param auxid ID for this auxiliary variable.
   *
   * Will throw @c SG::ExcAuxTypeMismatch if the types don't match.
   */
  Accessor (const SG::auxid_t auxid);


  /**
   * @brief Fetch the variable for one element.
   * @param e The element for which to fetch the variable.
   *
   * Will return an @c ElementLink proxy, which may be converted to
   * or assigned from an @c ElementLink.
   */
  template <IsAuxElement ELT>
  ELProxy operator() (ELT& e) const;


  /**
   * @brief Fetch the variable for one element.
   * @param container The container from which to fetch the variable.
   * @param index The index of the desired element.
   *
   * This allows retrieving aux data by container / index.
   *
   * Will return an @c ElementLink proxy, which may be converted to
   * or assigned from an @c ElementLink.
   */
  ELProxy operator() (AuxVectorData& container, size_t index) const;


  /**
   * @brief Set the variable for one element.
   * @param e The element for which to fetch the variable.
   * @param l The @c ElementLink to set.
   */
  void set (AuxElement& e, const Link_t& l) const;


  /**
   * @brief Set the variable for one element.
   * @param container The container from which to fetch the variable.
   * @param index The index of the desired element.
   * @param l The @c ElementLink to set.
   */
  void set (AuxVectorData& container, size_t index, const Link_t& x) const;


  /**
   * @brief Get a pointer to the start of the array of @c PackedLinks.
   * @param container The container from which to fetch the variable.
   */
  PLink_t*
  getPackedLinkArray (AuxVectorData& container) const;


  /**
   * @brief Get a pointer to the start of the linked array of @c DataLinks.
   * @param container The container from which to fetch the variable.
   */
  DLink_t*
  getDataLinkArray (AuxVectorData& container) const;


  /**
   * @brief Get a span over the array of @c PackedLinks.
   * @param container The container from which to fetch the variable.
   */
  PackedLink_span
  getPackedLinkSpan (AuxVectorData& container) const;


  /**
   * @brief Get a span over the array of @c DataLinks.
   * @param container The container from which to fetch the variable.
   */
  DataLink_span
  getDataLinkSpan (AuxVectorData& container) const;


  /**
   * @brief Get a span of @c ElementLink proxies.
   * @param container The container from which to fetch the variable.
   *
   * The proxies may be converted to or assigned from @c ElementLink.
   */
  span
  getDataSpan (AuxVectorData& container) const;
    

  /**
   * @brief Test to see if this variable exists in the store and is writable.
   * @param e An element of the container in which to test the variable.
   */
  bool isAvailableWritable (AuxElement& e) const;
    

  /**
   * @brief Test to see if this variable exists in the store and is writable.
   * @param c The container in which to test the variable.
   */
  bool isAvailableWritable (AuxVectorData& c) const;
};


//************************************************************************


/**
 * @brief Helper class to provide constant type-safe access to aux data,
 *        specialized for a vector of PackedLink.
 *
 * This is a version of @c Accessor, specialized for @c std::vector<PackedLink>.
 *
 * Although the type argument is @c std::vector<PackedLink<CONT> >,
 * the objects that this accessor produces are spans over @c ElementLink<CONT>
 * (returned by value of course rather than by const reference).
 * The @c getDataSpan method will then produce a (writable) span over
 * spans over @c ElementLink<CONT>.
 * (There are separate methods for returning spans over the
 * std::vector<PackedLink>/DataLink arrays themselves.)
 *
 * You might use this something like this:
 *
 *@code
 *   // Only need to do this once.
 *   using Cont_t = std::vector<int>;
 *   static const SG::Accessor<std::vector<SG::PackedLink<Cont_t> > links ("links");
 *   ...
 *   DataVector<MyClass>* v = ...;
 *   Myclass* m = v->at(2);
 *   links (*m).push_back (ElementLink<Cont_t> (...));
 *   auto span = links.getDataSpan (*v);
 *   span[2][1] = ElementLink<Cont_t> (...);
 @endcode
 *
 * You can also use this to define getters/setters in your class:
 *
 *@code
 *  class Myclass {
 *    ...
 *    auto get_links() const
 *    { const static ConstAccessor<SG::PackedLink<Cont_t> > acc ("links");
 *      return acc (*this); }
 *    void set_links (const std::vector<ElementLink<Cont_t> >& elv)
 *    { const static Accessor<std::vector<SG::PackedLink<Cont_t> > > acc ("links");
 *      acc (*this) = elv; }
 @endcode
*/
template <class CONT, class ALLOC, class VALLOC>
class Accessor<std::vector<PackedLink<CONT>, VALLOC>, ALLOC>
  : public ConstAccessor<std::vector<PackedLink<CONT>, VALLOC>, ALLOC>
{
public:
  // Aliases for the types we're dealing with.
  using Base = ConstAccessor<std::vector<PackedLink<CONT>, VALLOC>, ALLOC>;
  using Link_t = ElementLink<CONT>;
  using PLink_t = SG::PackedLink<CONT>;
  using VElt_t = std::vector<SG::PackedLink<CONT>, VALLOC>;
  using DLink_t = DataLink<CONT>;
  using DLinkAlloc_t = typename std::allocator_traits<ALLOC>::template rebind_alloc<DLink_t>;

  /// Spans over the objects that are actually stored.
  using PackedLinkVector_span = typename AuxDataTraits<VElt_t, ALLOC>::span;
  using DataLink_span = typename AuxDataTraits<DataLink<CONT>, DLinkAlloc_t>::span;

  /// And a span over @c PackedLink objects.
  using PackedLink_span = typename AuxDataTraits<PackedLink<CONT>, VALLOC>::span;

  /// Presents a vector of @c PackedLink as a range of @c ElementLink proxies.
  using elt_span = detail::ELSpanProxy<CONT, VALLOC>;


  /// Transform a span over vector of @c PackedLink to a
  /// span over span over @c ElementLink proxies.
  using ELSpanConverter = detail::ELSpanConverter<CONT, VALLOC>;
  using span =
    CxxUtils::transform_view_with_at<PackedLinkVector_span, ELSpanConverter >;


  /// Type the user sees.
  using element_type = elt_span;

  /// Type referencing an item.
  using reference_type = elt_span;

  /// Not supported.
  using container_pointer_type = void;
  using const_container_pointer_type = void;


  /// Take these from the base class.
  using Base::operator();
  using Base::getPackedLinkVectorArray;
  using Base::getDataLinkArray;
  using Base::getPackedLinkSpan;
  using Base::getPackedLinkVectorSpan;
  using Base::getDataSpan;
  using Base::getDataLinkSpan;


  /**
   * @brief Constructor.
   * @param name Name of this aux variable.
   *
   * The name -> auxid lookup is done here.
   */
  Accessor (const std::string& name);


  /**
   * @brief Constructor.
   * @param name Name of this aux variable.
   * @param clsname The name of its associated class.  May be blank.
   *
   * The name -> auxid lookup is done here.
   */
  Accessor (const std::string& name, const std::string& clsname);


  /**
   * @brief Constructor taking an auxid directly.
   * @param auxid ID for this auxiliary variable.
   *
   * Will throw @c SG::ExcAuxTypeMismatch if the types don't match.
   */
  Accessor (const SG::auxid_t auxid);


  /**
   * @brief Fetch the variable for one element.
   * @param e The element for which to fetch the variable.
   *
   * This will return a range of @c ElementLink proxies.
   * These proxies may be converted to or assigned from @c ElementLink.
   */
  template <IsAuxElement ELT>
  elt_span operator() (ELT& e) const;


  /**
   * @brief Fetch the variable for one element.
   * @param container The container from which to fetch the variable.
   * @param index The index of the desired element.
   *
   * This allows retrieving aux data by container / index.
   *
   * This will return a range of @c ElementLink proxies.
   * These proxies may be converted to or assigned from @c ElementLink.
   */
  elt_span operator() (AuxVectorData& container, size_t index) const;


  /**
   * @brief Set the variable for one element.
   * @param e The element for which to set the variable.
   * @param r The variable value to set, as a range over @c ElementLink.
   */
  template <detail::ElementLinkRange<CONT> RANGE>
  void set (AuxElement& e, const RANGE& r) const;


  /**
   * @brief Set the variable for one element.
   * @param container The container for which to set the variable.
   * @param index The index of the desired element.
   * @param r The variable value to set, as a range over @c ElementLink.
   */
  template <detail::ElementLinkRange<CONT> RANGE>
  void set (AuxVectorData& container, size_t index, const RANGE& x) const;


  /**
   * @brief Get a pointer to the start of the array of vectors of @c PackedLinks.
   * @param container The container from which to fetch the variable.
   */
  VElt_t*
  getPackedLinkVectorArray (AuxVectorData& container) const;


  /**
   * @brief Get a pointer to the start of the linked array of @c DataLinks.
   * @param container The container from which to fetch the variable.
   */
  DLink_t*
  getDataLinkArray (AuxVectorData& container) const;


  /**
   * @brief Get a span over the vector of @c PackedLinks for a given element.
   * @param e The element for which to fetch the variable.
   */
  PackedLink_span
  getPackedLinkSpan (AuxElement& e) const;


  /**
   * @brief Get a span over the vector of @c PackedLinks for a given element.
   * @param container The container from which to fetch the variable.
   * @param index The index of the desired element.
   */
  PackedLink_span
  getPackedLinkSpan (AuxVectorData& container, size_t index) const;


  /**
   * @brief Get a span over the vectors of @c PackedLinks.
   * @param container The container from which to fetch the variable.
   */
  PackedLinkVector_span
  getPackedLinkVectorSpan (AuxVectorData& container) const;


  /**
   * @brief Get a span over the array of @c DataLinks.
   * @param container The container from which to fetch the variable.
   */
  DataLink_span
  getDataLinkSpan (AuxVectorData& container) const;


   /**
   * @brief Get a span over spans of @c ElementLink proxies.
   * @param container The container from which to fetch the variable.
   *
   * The individual proxies may be converted to or assigned from @c ElementLink.
   * Each element may also be assigned from a range of @c ElementLink,
   * or converted to a vector of @c ElementLink.
   */
  span
  getDataSpan (AuxVectorData& container) const;
    

  /**
   * @brief Test to see if this variable exists in the store and is writable.
   * @param e An element of the container in which to test the variable.
   */
  bool isAvailableWritable (AuxElement& e) const;
    

  /**
   * @brief Test to see if this variable exists in the store and is writable.
   * @param c The container in which to test the variable.
   */
  bool isAvailableWritable (AuxVectorData& e) const;
};


} // namespace SG


#include "AthContainers/PackedLinkAccessor.icc"


#endif // not ATHCONTAINERS_PACKEDLINKACCESSOR_H
