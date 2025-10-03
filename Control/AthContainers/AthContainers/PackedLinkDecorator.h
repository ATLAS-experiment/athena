// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file AthContainers/PackedLinkDecorator.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Nov, 2023
 * @brief Helper class to provide type-safe access to aux data,
 *        specialized for @c PackedLink.
 */


#ifndef ATHCONTAINERS_PACKEDLINKDECORATOR_H
#define ATHCONTAINERS_PACKEDLINKDECORATOR_H


#include "AthContainersInterfaces/AuxTypes.h"
#include "AthContainersInterfaces/IAuxElement.h"
#include "AthContainers/Accessor.h"
#include "AthContainers/PackedLinkConstAccessor.h"
#include "AthContainers/PackedLinkImpl.h"
#include "AthContainers/tools/PackedLinkVectorFactory.h"
#include "AthContainers/AuxVectorData.h"
#include "AthContainers/AuxTypeRegistry.h"
#include "AthContainers/tools/ELProxy.h"
#include "AthContainers/tools/AuxDataTraits.h"
#include "AthContainers/tools/AuxElementConcepts.h"
#include "AthLinks/DataLink.h"
#include "CxxUtils/checker_macros.h"
#include <string>
#include <typeinfo>
#include <iterator>


namespace SG {


/**
 * @brief Helper class to provide type-safe access to aux data,
 *        specialized for PackedLink.
 *
 * This is a version of @c Decorator, specialized for packed links.
 *
 * This is like @c Accessor, except that it only `decorates' the container.
 * What this means is that this object can operate on a const container
 * and return a non-const reference.  However, if the container is locked,
 * this will only work if either this is a reference to a new variable,
 * in which case it is marked as a decoration, or it is a reference
 * to a variable already marked as a decoration.
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
 *   static const SG::Decorator<SG::PackedLink<Cont_t> links ("links");
 *   ...
 *   const DataVector<MyClass>* v = ...;
 *   const Myclass* m = v->at(2);
 *   links (*m) = ElementLink<Cont_t> (...);
 *   auto span = links.getDataSpan (*v);
 *   span[2] = ElementLink<Cont_t> (...);
 @endcode
*/
template <class CONT, class ALLOC>
class Decorator<PackedLink<CONT>, ALLOC>
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
  using PackedLink_span = typename AuxDataTraits<PackedLink<CONT>, ALLOC>::span;
  using DataLink_span = typename AuxDataTraits<DataLink<CONT>, DLinkAlloc_t>::span;


  /// Converter from @c PackedLink -> @c ElementLink.
  using ConstConverter_t = detail::PackedLinkConstConverter<CONT>;


  /// Transform a span over @c PackedLink to a span over @c ElementLink.
  using const_span =
    CxxUtils::transform_view_with_at<const_PackedLink_span, ConstConverter_t>;


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


  /**
   * @brief Constructor.
   * @param name Name of this aux variable.
   *
   * The name -> auxid lookup is done here.
   */
  Decorator (const std::string& name);


  /**
   * @brief Constructor.
   * @param name Name of this aux variable.
   * @param clsname The name of its associated class.  May be blank.
   *
   * The name -> auxid lookup is done here.
   */
  Decorator (const std::string& name, const std::string& clsname);


  /**
   * @brief Constructor taking an auxid directly.
   * @param auxid ID for this auxiliary variable.
   *
   * Will throw @c SG::ExcAuxTypeMismatch if the types don't match.
   */
  Decorator (const SG::auxid_t auxid);


  /**
   * @brief Fetch the variable for one element.
   * @param e The element for which to fetch the variable.
   *
   * Will return an @c ElementLink proxy, which may be converted to
   * or assigned from an @c ElementLink.
   *
   * If the container is locked, this will allow fetching only variables
   * that do not yet exist (in which case they will be marked as decorations)
   * or variables already marked as decorations.
   */
  template <IsConstAuxElement ELT>
  ELProxy operator() (const ELT& e) const;


  /**
   * @brief Fetch the variable for one element.
   * @param container The container from which to fetch the variable.
   * @param index The index of the desired element.
   *
   * This allows retrieving aux data by container / index.
   *
   * Will return an @c ElementLink proxy, which may be converted to
   * or assigned from an @c ElementLink.
   *
   * If the container is locked, this will allow fetching only variables
   * that do not yet exist (in which case they will be marked as decorations)
   * or variables already marked as decorations.
   */
  ELProxy
  operator() (const AuxVectorData& container, size_t index) const;


  /**
   * @brief Set the variable for one element.
   * @param e The element for which to set the variable.
   * @param l The @c ElementLink to set.
   */
  template <IsConstAuxElement ELT>
  void set (const ELT& e, const element_type& l) const;


  /**
   * @brief Set the variable for one element.
   * @param container The container for which to set the variable.
   * @param index The index of the desired element.
   * @param l The @c ElementLink to set.
   */
  void set (const AuxVectorData& container, size_t index, const Link_t& x) const;


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
   * @brief Get a pointer to the start of the array of @c PackedLinks,
   *        as a decoration.
   * @param container The container from which to fetch the variable.
   *
   * If the container is locked, this will allow fetching only variables
   * that do not yet exist (in which case they will be marked as decorations)
   * or variables already marked as decorations.
   */
  PLink_t*
  getPackedLinkDecorArray (const AuxVectorData& container) const;


  /**
   * @brief Get a pointer to the start of the linked array of @c DataLinks,
   *        as a decoration.
   * @param container The container from which to fetch the variable.
   *
   * If the container is locked, this will allow fetching only variables
   * that do not yet exist (in which case they will be marked as decorations)
   * or variables already marked as decorations.
   */
  DLink_t*
  getDataLinkDecorArray (const AuxVectorData& container) const;


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


  /**
   * @brief Get a span over the array of @c PackedLinks, as a decoration.
   * @param container The container from which to fetch the variable.
   *
   * If the container is locked, this will allow fetching only variables
   * that do not yet exist (in which case they will be marked as decorations)
   * or variables already marked as decorations.
   */
  PackedLink_span
  getPackedLinkDecorSpan (const AuxVectorData& container) const;


  /**
   * @brief Get a span over the array of @c DataLinks, as a decoration.
   * @param container The container from which to fetch the variable.
   *
   * If the container is locked, this will allow fetching only variables
   * that do not yet exist (in which case they will be marked as decorations)
   * or variables already marked as decorations.
   */
  DataLink_span
  getDataLinkDecorSpan (const AuxVectorData& container) const;


  /**
   * @brief Get a span of @c ElementLink proxies, as a decoration.
   * @param container The container from which to fetch the variable.
   *
   * The proxies may be converted to or assigned from @c ElementLink.
   *
   * If the container is locked, this will allow fetching only variables
   * that do not yet exist (in which case they will be marked as decorations)
   * or variables already marked as decorations.
   */
  span
  getDecorationSpan (const AuxVectorData& container) const;


  /**
   * @brief Test to see if this variable exists in the store and is writable.
   * @param e An element of the container in which to test the variable.
   */
  template <IsConstAuxElement ELT>
  bool isAvailableWritable (const ELT& e) const;


  /**
   * @brief Test to see if this variable exists in the store and is writable.
   * @param c The container in which to test the variable.
   */
  bool isAvailableWritable (const AuxVectorData& c) const;


protected:
  /**
   * @brief Constructor.
   * @param name Name of this aux variable.
   * @param clsname The name of its associated class.  May be blank.
   * @param flags Optional flags qualifying the type.  See AuxTypeRegsitry.
   *
   * The name -> auxid lookup is done here.
   */
  Decorator (const std::string& name,
             const std::string& clsname,
             const SG::AuxVarFlags flags);
};


//************************************************************************


/**
 * @brief Helper class to provide constant type-safe access to aux data,
 *        specialized for a vector of PackedLink.
 *
 * This is like @c Accessor, except that it only `decorates' the container.
 * What this means is that this object can operate on a const container
 * and return a non-const reference.  However, if the container is locked,
 * this will only work if either this is a reference to a new variable,
 * in which case it is marked as a decoration, or it is a reference
 * to a variable already marked as a decoration.
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
 *   static const SG::Decorator<std::vector<SG::PackedLink<Cont_t> > links ("links");
 *   ...
 *   const DataVector<MyClass>* v = ...;
 *   const Myclass* m = v->at(2);
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
class Decorator<std::vector<PackedLink<CONT>, VALLOC>, ALLOC>
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
  using PackedLinkVector_span = typename AuxDataTraits<VElt_t, ALLOC>::span;
  using DataLink_span = typename AuxDataTraits<DataLink<CONT>, DLinkAlloc_t>::span;

  /// And spans over @c PackedLink objects.
  using const_PackedLink_span = typename AuxDataTraits<PackedLink<CONT>, VALLOC>::const_span;
  using PackedLink_span = typename AuxDataTraits<PackedLink<CONT>, VALLOC>::span;


  /// Converter from vector of @c PackedLink to a span over @c ElementLinks.
  using ConstVectorTransform_t = detail::PackedLinkVectorConstConverter<CONT>;

  /// Transform a span over vector of @c PackedLink to a
  /// span over span over @c ElementLink.
  using const_span =
    CxxUtils::transform_view_with_at<const_PackedLinkVector_span, ConstVectorTransform_t>;


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


  /**
   * @brief Constructor.
   * @param name Name of this aux variable.
   *
   * The name -> auxid lookup is done here.
   */
  Decorator (const std::string& name);


  /**
   * @brief Constructor.
   * @param name Name of this aux variable.
   * @param clsname The name of its associated class.  May be blank.
   *
   * The name -> auxid lookup is done here.
   */
  Decorator (const std::string& name, const std::string& clsname);


  /**
   * @brief Constructor taking an auxid directly.
   * @param auxid ID for this auxiliary variable.
   *
   * Will throw @c SG::ExcAuxTypeMismatch if the types don't match.
   */
  Decorator (const SG::auxid_t auxid);


  /**
   * @brief Fetch the variable for one element.
   * @param e The element for which to fetch the variable.
   *
   * This will return a range of @c ElementLink proxies.
   * These proxies may be converted to or assigned from @c ElementLink.
   *
   * If the container is locked, this will allow fetching only variables
   * that do not yet exist (in which case they will be marked as decorations)
   * or variables already marked as decorations.
   */
  template <IsConstAuxElement ELT>
  elt_span operator() (const ELT& e) const;


  /**
   * @brief Fetch the variable for one element.
   * @param container The container from which to fetch the variable.
   * @param index The index of the desired element.
   *
   * This allows retrieving aux data by container / index.
   *
   * This will return a range of @c ElementLink proxies.
   * These proxies may be converted to or assigned from @c ElementLink.
   *
   * If the container is locked, this will allow fetching only variables
   * that do not yet exist (in which case they will be marked as decorations)
   * or variables already marked as decorations.
   */
  elt_span
  operator() (const AuxVectorData& container, size_t index) const;


  /**
   * @brief Set the variable for one element.
   * @param e The element for which to set the variable.
   * @param r The variable value to set, as a range over @c ElementLink.
   */
  template <IsConstAuxElement ELT, detail::ElementLinkRange<CONT> RANGE>
  void set (const ELT& e, const RANGE& x) const;


  /**
   * @brief Set the variable for one element.
   * @param container The container for which to set the variable.
   * @param index The index of the desired element.
   * @param r The variable value to set, as a range over @c ElementLink.
   */
  template <detail::ElementLinkRange<CONT> RANGE>
  void set (const AuxVectorData& container, size_t index, const RANGE& r) const;


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
   * @brief Get a pointer to the start of the array of vectors of @c PackedLinks,
   *        as a decoration.
   * @param container The container from which to fetch the variable.
   */
  VElt_t*
  getPackedLinkVectorDecorArray (const AuxVectorData& container) const;


  /**
   * @brief Get a pointer to the start of the linked array of @c DataLinks,
   *        as a decoration.
   * @param container The container from which to fetch the variable.
   */
  DLink_t*
  getDataLinkDecorArray (const AuxVectorData& container) const;


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


  /**
   * @brief Get a span over the vector of @c PackedLinks for a given element,
   *        as a decoration.
   * @param e The element for which to fetch the variable.
   *
   * If the container is locked, this will allow fetching only variables
   * that do not yet exist (in which case they will be marked as decorations)
   * or variables already marked as decorations.
   */
  template <IsConstAuxElement ELT>
  PackedLink_span
  getPackedLinkDecorSpan (const ELT& e) const;


  /**
   * @brief Get a span over the vector of @c PackedLinks for a given element,
   *        as a decoration.
   * @param container The container from which to fetch the variable.
   * @param index The index of the desired element.
   *
   * If the container is locked, this will allow fetching only variables
   * that do not yet exist (in which case they will be marked as decorations)
   * or variables already marked as decorations.
   */
  PackedLink_span
  getPackedLinkDecorSpan (const AuxVectorData& container, size_t index) const;


  /**
   * @brief Get a span over the vectors of @c PackedLinks,
   *        as a decoration.
   * @param container The container from which to fetch the variable.
   *
   * If the container is locked, this will allow fetching only variables
   * that do not yet exist (in which case they will be marked as decorations)
   * or variables already marked as decorations.
   */
  PackedLinkVector_span
  getPackedLinkVectorDecorSpan (const AuxVectorData& container) const;


  /**
   * @brief Get a span over the array of @c DataLinks, as a decoration.
   * @param container The container from which to fetch the variable.
   *
   * If the container is locked, this will allow fetching only variables
   * that do not yet exist (in which case they will be marked as decorations)
   * or variables already marked as decorations.
   */
  DataLink_span
  getDataLinkDecorSpan (const AuxVectorData& container) const;


  /**
   * @brief Get a span over spans of @c ElementLink proxies,
   *        as a decoration.
   * @param container The container from which to fetch the variable.
   *
   * The individual proxies may be converted to or assigned from @c ElementLink.
   * Each element may also be assigned from a range of @c ElementLink,
   * or converted to a vector of @c ElementLink.
   *
   * If the container is locked, this will allow fetching only variables
   * that do not yet exist (in which case they will be marked as decorations)
   * or variables already marked as decorations.
   */
  span
  getDecorationSpan (const AuxVectorData& container) const;


  /**
   * @brief Test to see if this variable exists in the store and is writable.
   * @param e An element of the container in which to test the variable.
   */
  template <IsConstAuxElement ELT>
  bool isAvailableWritable (const ELT& e) const;


  /**
   * @brief Test to see if this variable exists in the store and is writable.
   * @param c The container in which to test the variable.
   */
  bool isAvailableWritable (const AuxVectorData& e) const;


protected:
  /**
   * @brief Constructor.
   * @param name Name of this aux variable.
   * @param clsname The name of its associated class.  May be blank.
   * @param flags Optional flags qualifying the type.  See AuxTypeRegsitry.
   *
   * The name -> auxid lookup is done here.
   */
  Decorator (const std::string& name,
             const std::string& clsname,
             const SG::AuxVarFlags flags);
};


} // namespace SG


#include "AthContainers/PackedLinkDecorator.icc"



#endif // not ATHCONTAINERS_PACKEDLINKDECORATOR_H
