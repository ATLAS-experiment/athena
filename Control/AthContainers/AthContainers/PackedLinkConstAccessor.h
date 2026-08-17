// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file AthContainers/PackedLinkConstAccessor.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Oct, 2023
 * @brief Helper class to provide constant type-safe access to aux data,
 *        specialized for @c PackedLink.
 */


#ifndef ATHCONTAINERS_PACKEDLINKCONSTACCESSOR_H
#define ATHCONTAINERS_PACKEDLINKCONSTACCESSOR_H


#include "AthContainersInterfaces/AuxTypes.h"
#include "AthContainers/PackedLinkImpl.h"
#include "AthContainers/ConstAccessor.h"
#include "AthContainers/AuxElement.h"
#include "AthContainers/tools/AuxDataTraits.h"
#include "AthContainers/tools/AuxElementConcepts.h"
#include "AthContainers/tools/LinkedVarAccessorBase.h"
#include "AthContainers/tools/PackedLinkConversions.h"
#include "AthLinks/ElementLink.h"
#include "AthLinks/DataLink.h"
#include "CxxUtils/range_with_conv.h"
#include <iterator>
#include <ranges>


class IProxyDict;


namespace SG {


/**
 * @brief Helper class to provide constant type-safe access to aux data,
 *        specialized for PackedLink.
 *
 * This is a version of @c ConstAccessor, specialized for @c PackedLink.
 *
 * Although the type argument is @c PackedLink<CONT>, the objects that
 * this accessor produces are @c ElementLink<CONT> (returned by value
 * rather than by const reference).  The @c getDataSpan method will
 * then produce a (read-only) span over @c ElementLink<CONT>.
 * (There are separate methods for returning spans over the
 * PackedLink/DataLink arrays themselves.)
 *
 * You might use this something like this:
 *
 *@code
 *   // Only need to do this once.
 *   using Cont_t = std::vector<int>;
 *   static const SG::ConstAccessor<SG::PackedLink<Cont_t> links ("links");
 *   ...
 *   const DataVector<MyClass>* v = ...;
 *   const Myclass* m = v->at(2);
 *   ElementLink<Cont_t> l = links (*m);
 *   auto span = links.getDataSpan (*v);
 *   ElementLink<Cont_t> l2 = span[2];
 @endcode
 *
 * This class can be used only for reading data.
 * To modify data, see the class @c Accessor (also specialized for PackedLink).
 */
template <class CONT, class ALLOC>
class ConstAccessor<SG::PackedLink<CONT>, ALLOC>
  : public detail::LinkedVarAccessorBase
{
public:
  // Aliases for the types we're dealing with.
  using Link_t = ElementLink<CONT>;
  using PLink_t = SG::PackedLink<CONT>;
  using DLink_t = DataLink<CONT>;
  using DLinkAlloc_t = typename std::allocator_traits<ALLOC>::template rebind_alloc<DLink_t>;


  /// Spans over the objects that are actually stored.
  using const_PackedLink_span = typename AuxDataTraits<PackedLink<CONT>, ALLOC>::const_span;
  using const_DataLink_span = typename AuxDataTraits<DataLink<CONT>, DLinkAlloc_t>::const_span;


  /// Converter from @c PackedLink -> @c ElementLink.
  using ConstConverter_t = detail::PackedLinkConstConverter<CONT>;


  /// Transform a span over @c PackedLink to a span over @c ElementLink.
  using const_span = std::ranges::transform_view<const_PackedLink_span,
                                                 ConstConverter_t>;


  /// Type the user sees.
  using element_type = Link_t;

  /// Type referencing an item.
  using const_reference_type = element_type;

  /// Not supported.
  using const_container_pointer_type = void;


  /**
   * @brief Constructor.
   * @param name Name of this aux variable.
   *
   * The name -> auxid lookup is done here.
   */
  ConstAccessor (const std::string& name);


  /**
   * @brief Constructor.
   * @param name Name of this aux variable.
   * @param clsname The name of its associated class.  May be blank.
   *
   * The name -> auxid lookup is done here.
   */
  ConstAccessor (const std::string& name, const std::string& clsname);


  /**
   * @brief Constructor taking an auxid directly.
   * @param auxid ID for this auxiliary variable.
   *
   * Will throw @c SG::ExcAuxTypeMismatch if the types don't match.
   */
  ConstAccessor (const SG::auxid_t auxid);


  /**
   * @brief Fetch the variable for one element.
   * @param e The element for which to fetch the variable.
   */
  template <IsConstAuxElement ELT>
  const Link_t operator() (const ELT& e) const;


  /**
   * @brief Fetch the variable for one element.
   * @param container The container from which to fetch the variable.
   * @param index The index of the desired element.
   *
   * This allows retrieving aux data by container / index.
   */
  const Link_t
  operator() (const AuxVectorData& container, size_t index) const;


  /**
   * @brief Get a pointer to the start of the array of @c PackedLinks.
   * @param container The container from which to fetch the variable.
   */
  const PLink_t*
  getPackedLinkArray (const AuxVectorData& container) const;


  /**
   * @brief Get a pointer to the start of the linked array of @c DataLinks.
   * @param container The container from which to fetch the variable.
   */
  const DLink_t*
  getDataLinkArray (const AuxVectorData& container) const;


  /**
   * @brief Get a span over the array of @c PackedLinks.
   * @param container The container from which to fetch the variable.
   */
  const_PackedLink_span
  getPackedLinkSpan (const AuxVectorData& container) const;


  /**
   * @brief Get a span over the array of @c DataLinks.
   * @param container The container from which to fetch the variable.
   */
  const_DataLink_span
  getDataLinkSpan (const AuxVectorData& container) const;


  /**
   * @brief Get a span of @c ElementLinks.
   * @param container The container from which to fetch the variable.
   */
  const_span
  getDataSpan (const AuxVectorData& container) const;


protected:
  /**
   * @brief Constructor.
   * @param name Name of this aux variable.
   * @param clsname The name of its associated class.  May be blank.
   * @param flags Optional flags qualifying the type.  See AuxTypeRegistry.
   *
   * The name -> auxid lookup is done here.
   */
  ConstAccessor (const std::string& name,
                 const std::string& clsname,
                 const SG::AuxVarFlags flags);


  /**
   * @brief Helper to resolve a @c PackedLink to an @c ElementLink.
   * @param container The container from which to fetch the variable.
   * @param index The index of the desired element.
   */
  Link_t
  resolveLink (const AuxVectorData& container, size_t index) const;
};


//************************************************************************


/**
 * @brief Helper class to provide constant type-safe access to aux data,
 *        specialized for a vector of PackedLink.
 *
 * This is a version of @c ConstAccessor,
 * specialized for @c std::vector<PackedLink>.
 *
 * Although the type argument is @c std::vector<PackedLink<CONT> >,
 * the objects that this accessor produces are spans over @c ElementLink<CONT>
 * (returned by value of course rather than by const reference).
 * The @c getDataSpan method will then produce a (read-only) span over
 * spans over @c ElementLink<CONT>.
 * (There are separate methods for returning spans over the
 * std::vector<PackedLink>/DataLink arrays themselves.)
 *
 * You might use this something like this:
 *
 *@code
 *   // Only need to do this once.
 *   using Cont_t = std::vector<int>;
 *   static const SG::ConstAccessor<std::vector<SG::PackedLink<Cont_t> > links ("links");
 *   ...
 *   const DataVector<MyClass>* v = ...;
 *   const Myclass* m = v->at(2);
 *   for (ElementLink<Cont_t> l : links(*m))
 *     ...
 *   auto span = links.getDataSpan (*v);
 *   ElementLink<Cont_t> l2 = span[2][1];
 @endcode
 *
 * This class can be used only for reading data.
 * To modify data, see the class @c Accessor (also specialized for PackedLink).
 */
template <class CONT, class ALLOC, class VALLOC>
class ConstAccessor<std::vector<SG::PackedLink<CONT>, VALLOC>, ALLOC>
  : public detail::LinkedVarAccessorBase
{
public:
  // Aliases for the types we're dealing with.
  using Link_t = ElementLink<CONT>;
  using PLink_t = SG::PackedLink<CONT>;
  using VElt_t = std::vector<SG::PackedLink<CONT>, VALLOC>;
  using DLink_t = DataLink<CONT>;
  using DLinkAlloc_t = typename std::allocator_traits<ALLOC>::template rebind_alloc<DLink_t>;

  /// Spans over the objects that are actually stored.
  using const_PackedLinkVector_span = typename AuxDataTraits<VElt_t, ALLOC>::const_span;
  using const_DataLink_span = typename AuxDataTraits<DataLink<CONT>, DLinkAlloc_t>::const_span;

  /// And a span over @c PackedLink objects.
  using const_PackedLink_span = typename AuxDataTraits<PackedLink<CONT>, VALLOC>::const_span;


  /// Converter from vector of @c PackedLink to a span over @c ElementLinks.
  using ConstVectorTransform_t = detail::PackedLinkVectorConstConverter<CONT>;
  using const_elt_span = typename ConstVectorTransform_t::value_type;


  /// Transform a span over vector of @c PackedLink to a
  /// span over span over @c ElementLink.
  using const_span = std::ranges::transform_view<const_PackedLinkVector_span,
                                                 ConstVectorTransform_t>;


  /// Type the user sees.
  using element_type = const_elt_span;

  /// Type referencing an item.
  using const_reference_type = element_type;

  /// Not supported.
  using const_container_pointer_type = void;


  /**
   * @brief Constructor.
   * @param name Name of this aux variable.
   *
   * The name -> auxid lookup is done here.
   */
  ConstAccessor (const std::string& name);


  /**
   * @brief Constructor.
   * @param name Name of this aux variable.
   * @param clsname The name of its associated class.  May be blank.
   *
   * The name -> auxid lookup is done here.
   */
  ConstAccessor (const std::string& name, const std::string& clsname);


  /**
   * @brief Constructor taking an auxid directly.
   * @param auxid ID for this auxiliary variable.
   *
   * Will throw @c SG::ExcAuxTypeMismatch if the types don't match.
   */
  ConstAccessor (const SG::auxid_t auxid);


  /**
   * @brief Fetch the variable for one element.
   * @param e The element for which to fetch the variable.
   *
   * This will return a range of @c ElementLinks.
   */
  template <IsConstAuxElement ELT>
  const_elt_span operator() (const ELT& e) const;


  /**
   * @brief Fetch the variable for one element.
   * @param container The container from which to fetch the variable.
   * @param index The index of the desired element.
   *
   * This allows retrieving aux data by container / index.
   *
   * This will return a range of @c ElementLinks.
   */
  const_elt_span
  operator() (const AuxVectorData& container, size_t index) const;


  /**
   * @brief Get a pointer to the start of the array of vectors of @c PackedLinks.
   * @param container The container from which to fetch the variable.
   */
  const VElt_t*
  getPackedLinkVectorArray (const AuxVectorData& container) const;


  /**
   * @brief Get a pointer to the start of the linked array of @c DataLinks.
   * @param container The container from which to fetch the variable.
   */
  const DLink_t*
  getDataLinkArray (const AuxVectorData& container) const;


  /**
   * @brief Get a span over the vector of @c PackedLinks for a given element.
   * @param e The element for which to fetch the variable.
   */
  template <IsConstAuxElement ELT>
  const_PackedLink_span
  getPackedLinkSpan (const ELT& e) const;


  /**
   * @brief Get a span over the vector of @c PackedLinks for a given element.
   * @param container The container from which to fetch the variable.
   * @param index The index of the desired element.
   */
  const_PackedLink_span
  getPackedLinkSpan (const AuxVectorData& container, size_t index) const;


  /**
   * @brief Get a span over the vectors of @c PackedLinks.
   * @param container The container from which to fetch the variable.
   */
  const_PackedLinkVector_span
  getPackedLinkVectorSpan (const AuxVectorData& container) const;


  /**
   * @brief Get a span over the array of @c DataLinks.
   * @param container The container from which to fetch the variable.
   */
  const_DataLink_span
  getDataLinkSpan (const AuxVectorData& container) const;


  /**
   * @brief Get a span over spans of @c ElementLinks.
   * @param container The container from which to fetch the variable.
   */
  const_span
  getDataSpan (const AuxVectorData& container) const;


protected:
  /**
   * @brief Constructor.
   * @param name Name of this aux variable.
   * @param clsname The name of its associated class.  May be blank.
   * @param flags Optional flags qualifying the type.  See AuxTypeRegistry.
   *
   * The name -> auxid lookup is done here.
   */
  ConstAccessor (const std::string& name,
                 const std::string& clsname,
                 const SG::AuxVarFlags flags);
};


} // namespace SG


#include "AthContainers/PackedLinkConstAccessor.icc"


#endif // not ATHCONTAINERS_PACKEDLINKCONSTACCESSOR_H
