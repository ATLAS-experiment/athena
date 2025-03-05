// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file AthContainers/JaggedVecDecorator.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Apr, 2024
 * @brief Helper class to provide type-safe access to aux data,
 *        specialized for @c JaggedVecElt.
 */


#ifndef ATHCONTAINERS_JAGGEDVECDECORATOR_H
#define ATHCONTAINERS_JAGGEDVECDECORATOR_H


#include "AthContainersInterfaces/AuxTypes.h"
#include "AthContainersInterfaces/IAuxElement.h"
#include "AthContainers/Decorator.h"
#include "AthContainers/JaggedVecConstAccessor.h"
#include "AthContainers/JaggedVecImpl.h"
#include "AthContainers/tools/JaggedVecVectorFactory.h"
#include "AthContainers/AuxVectorData.h"
#include "AthContainers/AuxTypeRegistry.h"
#include "AthContainers/tools/AuxElementConcepts.h"
#include "AthContainers/tools/JaggedVecConversions.h"
#include "CxxUtils/ranges.h"
#include "CxxUtils/checker_macros.h"
#include "CxxUtils/range_with_at.h"
#include "CxxUtils/range_with_conv.h"
#include <string>
#include <typeinfo>
#include <iterator>


namespace SG {


/**
 * @brief Helper class to provide type-safe access to aux data,
 *        specialized for @c JaggedVecElt.
 *
 * This is a version of @c Decorator, specialized for jagged vectors.
 *
 * This is like @c Accessor, except that it only `decorates' the container.
 * What this means is that this object can operate on a const container
 * and return a non-const reference.  However, if the container is locked,
 * this will only work if either this is a reference to a new variable,
 * in which case it is marked as a decoration, or it is a reference
 * to a variable already marked as a decoration.
 *
 * Although the type argument is @c JaggedVecElt<PAYLOAD_T>, the objects that
 * this accessor produces are spans over elements of type @c PAYLOAD_T.
 * The @c getDataSpan method will then produce a (writable) span over
 * spans of @c PAYLOAD_T.
 * (There are separate methods for returning spans over the
 * element/payload arrays themselves.)
 *
 * You might use this something like this:
 *
 *@code
 *   // Only need to do this once.
 *   using Cont_t = std::vector<int>;
 *   static const SG::Decorator<SG::JaggedVecElt<Cont_t> jvec ("jvec");
 *   ...
 *   const DataVector<MyClass>* v = ...;
 *   const Myclass* m = v->at(2);
 *   jvec (*m)[3] = 10;
 *   for (auto sp : jvec.getDataSpan (*v)) {
 *     for (int& x : sp) ...
 *   }
 @endcode
*/
template <class PAYLOAD_T, class ALLOC>
class Decorator<JaggedVecElt<PAYLOAD_T>, ALLOC>
  : public detail::LinkedVarAccessorBase
{
public:
  /// Payload type.
  using Payload_t = PAYLOAD_T;

  /// Allocator to use for the payload vector.
  using PayloadAlloc_t = typename std::allocator_traits<ALLOC>::template rebind_alloc<Payload_t>;

  /// One element of the jagged vector.
  using Elt_t = SG::JaggedVecElt<PAYLOAD_T>;

  /// Writable proxies for a jagged vector element: both a single proxy
  /// and one used as part of a span.
  using JVecProxy = detail::JaggedVecProxyT<Payload_t, detail::JaggedVecProxyValBase>;
  using JVecProxyInSpan = detail::JaggedVecProxyT<Payload_t, detail::JaggedVecProxyRefBase>;

  /// Converter from @c JaggedVecElt to a (writable) span.
  using Converter_t = detail::JaggedVecConverter<Payload_t>;
  
  /// Converter from @c JaggedVecElt to a span.
  using ConstConverter_t = detail::JaggedVecConstConverter<Payload_t>;

  /// Spans over the objects that are actually stored.
  using const_Elt_span = typename AuxDataTraits<JaggedVecElt<Payload_t>, ALLOC>::const_span;
  using const_Payload_span = typename AuxDataTraits<Payload_t, PayloadAlloc_t>::const_span;
  using Elt_span = typename AuxDataTraits<JaggedVecElt<Payload_t>, ALLOC>::span;
  using Payload_span = typename AuxDataTraits<Payload_t, PayloadAlloc_t>::span;

  /// Spans over the entire jagged vector, both writable and not.
  using const_span = CxxUtils::transform_view_with_at<const_Elt_span, ConstConverter_t>;
  using span = CxxUtils::transform_view_with_at<Elt_span, Converter_t>;

  /// Const span resulting from the conversion.
  using element_type = typename ConstConverter_t::element_type;

  /// The writable type we return.
  using reference_type = JVecProxy;

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
   * @brief Fetch the variable for one element, as a non-const reference.
   * @param e The element for which to fetch the variable.
   *
   * Will return a proxy object, which will allow treating this
   * jagged vector element as a vector.
   *
   * If the container is locked, this will allow fetching only variables
   * that do not yet exist (in which case they will be marked as decorations)
   * or variables already marked as decorations.
   */
  template <IsConstAuxElement ELT>
  reference_type operator() (const ELT& e) const;


  /**
   * @brief Fetch the variable for one element, as a non-const reference.
   * @param container The container from which to fetch the variable.
   * @param index The index of the desired element.
   *
   * This allows retrieving aux data by container / index.
   *
   * Will return a proxy object, which will allow treating this
   * jagged vector element as a vector.
   *
   * If the container is locked, this will allow fetching only variables
   * that do not yet exist (in which case they will be marked as decorations)
   * or variables already marked as decorations.
   */
  reference_type
  operator() (const AuxVectorData& container, size_t index) const;


  /**
   * @brief Set the variable for one element.
   * @param e The element for which to fetch the variable.
   * @param x The variable value to set.
   */
  template <IsConstAuxElement ELT,
            CxxUtils::InputRangeOverT<PAYLOAD_T> RANGE>
  void set (const ELT& e, const RANGE& x) const;


  /**
   * @brief Set the variable for one element.
   * @param container The container from which to fetch the variable.
   * @param index The index of the desired element.
   * @param x The variable value to set.
   */
  template <CxxUtils::InputRangeOverT<PAYLOAD_T> RANGE>
  void set (const AuxVectorData& container, size_t index, const RANGE& x) const;


  /**
   * @brief Get a pointer to the start of the array of @c JaggedVecElt objects.
   * @param container The container from which to fetch the variable.
   */
  const Elt_t* getEltArray (const AuxVectorData& container) const;


  /**
   * @brief Get a pointer to the start of the payload array.
   * @param container The container from which to fetch the variable.
   */
  const Payload_t* getPayloadArray (const AuxVectorData& container) const;


  /**
   * @brief Get a pointer to the start of the array of @c JaggedVecElt objects,
   *        as a decoration.
   * @param container The container from which to fetch the variable.
   *
   * If the container is locked, this will allow fetching only variables
   * that do not yet exist (in which case they will be marked as decorations)
   * or variables already marked as decorations.
   */
  Elt_t* getEltDecorArray (const AuxVectorData& container) const;


  /**
   * @brief Get a pointer to the start of the payload array,
   *        as a decoration.
   * @param container The container from which to fetch the variable.
   *
   * If the container is locked, this will allow fetching only variables
   * that do not yet exist (in which case they will be marked as decorations)
   * or variables already marked as decorations.
   */
  Payload_t* getPayloadDecorArray (const AuxVectorData& container) const;


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


  /**
   * @brief Get a span over the array of @c JaggedVecElt objects,
   *        as a decoration.
   * @param container The container from which to fetch the variable.
   *
   * If the container is locked, this will allow fetching only variables
   * that do not yet exist (in which case they will be marked as decorations)
   * or variables already marked as decorations.
   */
  Elt_span
  getEltDecorSpan (const AuxVectorData& container) const;


  /**
   * @brief Get a span over the payload vector,
   *        as a decoration.
   * @param container The container from which to fetch the variable.
   *
   * If the container is locked, this will allow fetching only variables
   * that do not yet exist (in which case they will be marked as decorations)
   * or variables already marked as decorations.
   */
  Payload_span
  getPayloadDecorSpan (const AuxVectorData& container) const;


  /**
   * @brief Get a span over spans representing the jagged vector,
   *        as a decoration.
   * @param container The container from which to fetch the variable.
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


} // namespace SG


#include "AthContainers/JaggedVecDecorator.icc"


#endif // not ATHCONTAINERS_JAGGEDVECDECORATOR_H
