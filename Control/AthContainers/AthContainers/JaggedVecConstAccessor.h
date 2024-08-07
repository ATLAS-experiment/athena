// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file AthContainers/JaggedVecConstAccessor.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Apr, 2024
 * @brief Helper class to provide constant type-safe access to aux data,
 *        specialized for @c JaggedVecElt.
 */


#ifndef ATHCONTAINERS_JAGGEDVECCONSTACCESSOR_H
#define ATHCONTAINERS_JAGGEDVECCONSTACCESSOR_H


#include "AthContainersInterfaces/AuxTypes.h"
#include "AthContainers/JaggedVecImpl.h"
#include "AthContainers/tools/JaggedVecVectorFactory.h"
#include "AthContainers/ConstAccessor.h"
#include "AthContainers/AuxElement.h"
#include "AthContainers/tools/AuxDataTraits.h"
#include "AthContainers/tools/AuxElementConcepts.h"
#include "AthContainers/tools/LinkedVarAccessorBase.h"
#include "AthContainers/tools/JaggedVecConversions.h"
#include "CxxUtils/range_with_at.h"
#include "CxxUtils/range_with_conv.h"
#include <iterator>


namespace SG {


/**
 * @brief Helper class to provide constant type-safe access to aux data,
 *        specialized for @c JaggedVecElt.
 *
 * This is a version of @c ConstAccessor, specialized for jagged vectors.
 *
 * Although the type argument is @c JaggedVecElt<PAYLOAD_T>, the objects that
 * this accessor produces are spans over elements of type @c PAYLOAD_T.
 * The @c getDataSpan method will then produce a (read-only) span over
 * spans of @c PAYLOAD_T.
 * (There are separate methods for returning spans over the
 * element/payload arrays themselves.)
 *
 * You might use this something like this:
 *
 *@code
 *   // Only need to do this once.
 *   using Cont_t = std::vector<int>;
 *   static const SG::ConstAccessor<SG::JaggedVecElt<Cont_t> jvec ("jvec");
 *   ...
 *   const DataVector<MyClass>* v = ...;
 *   const Myclass* m = v->at(2);
 *   int x = jvec (*m)[3];
 *   for (auto sp : jvec.getDataSpan (*v)) {
 *     for (int x : sp) ...
 *   }
 @endcode
 *
 * This class can be used only for reading data.
 * To modify data, see the class @c Accessor (also specialized for PackedLink).
 */
template <class PAYLOAD_T, class ALLOC>
class ConstAccessor<SG::JaggedVecElt<PAYLOAD_T>, ALLOC>
  : public detail::LinkedVarAccessorBase
{
public:
  /// Payload type.
  using Payload_t = PAYLOAD_T;

  /// Allocator to use for the payload vector.
  using PayloadAlloc_t = typename std::allocator_traits<ALLOC>::template rebind_alloc<Payload_t>;

  /// One element of the jagged vector.
  using Elt_t = SG::JaggedVecElt<Payload_t>;

  /// Converter from @c JaggedVecElt to a span.
  using ConstConverter_t = detail::JaggedVecConstConverter<Payload_t>;

  /// Span resulting from the conversion.  This is the type the user sees.
  using element_type = typename ConstConverter_t::element_type;

  /// Type referencing an item.
  using const_reference_type = element_type;

  /// Not supported.
  using const_container_pointer_type = void;


  /// Spans over the objects that are actually stored.
  using const_Elt_span = typename AuxDataTraits<Elt_t, ALLOC>::const_span;
  using const_Payload_span = typename AuxDataTraits<Payload_t>::const_span;


  /// Transform a span over elements to a span of spans.
  using const_span =
    CxxUtils::transform_view_with_at<const_Elt_span, ConstConverter_t>;


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
   * @brief Fetch the variable for one element, as a const reference.
   * @param e The element for which to fetch the variable.
   */
  template <IsConstAuxElement ELT>
  const element_type operator() (const ELT& e) const;


  /**
   * @brief Fetch the variable for one element, as a const reference.
   * @param container The container from which to fetch the variable.
   * @param index The index of the desired element.
   *
   * This allows retrieving aux data by container / index.
   */
  const element_type
  operator() (const AuxVectorData& container, size_t index) const;


  /**
   * @brief Get a pointer to the start of the array of @c JaggedVecElt objects.
   * @param container The container from which to fetch the variable.
   */
  const Elt_t*
  getEltArray (const AuxVectorData& container) const;


  /**
   * @brief Get a pointer to the start of the payload array.
   * @param container The container from which to fetch the variable.
   */
  const Payload_t*
  getPayloadArray (const AuxVectorData& container) const;


  /**
   * @brief Get a span over the array of @c JaggedVecElt objects.
   * @param container The container from which to fetch the variable.
   */
  const_Elt_span
  getEltSpan (const AuxVectorData& container) const;


  /**
   * @brief Get a span over the payload vector.
   * @param container The container from which to fetch the variable.
   */
  const_Payload_span
  getPayloadSpan (const AuxVectorData& container) const;


  /**
   * @brief Get a span over spans representing the jagged vector.
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
   * @brief Constructor.
   * @param b Another accessor from which to copy the auxids.
   */
  ConstAccessor (const detail::LinkedVarAccessorBase& b);
};


} // namespace SG


#include "AthContainers/JaggedVecConstAccessor.icc"


#endif // not ATHCONTAINERS_JAGGEDVECCONSTACCESSOR_H
