// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file AthContainers/JaggedVecAccessor.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Apr, 2024
 * @brief Helper class to provide type-safe access to aux data,
 *        specialized for @c JaggedVecElt.
 */


#ifndef ATHCONTAINERS_JAGGEDVECACCESSOR_H
#define ATHCONTAINERS_JAGGEDVECACCESSOR_H


#include "AthContainersInterfaces/AuxTypes.h"
#include "AthContainersInterfaces/IAuxElement.h"
#include "AthContainers/JaggedVecConstAccessor.h"
#include "AthContainers/Accessor.h"
#include "AthContainers/AuxVectorData.h"
#include "AthContainers/AuxTypeRegistry.h"
#include "CxxUtils/ranges.h"
#include "CxxUtils/checker_macros.h"
#include "CxxUtils/range_with_at.h"
#include <string>
#include <typeinfo>


namespace SG {


/**
 * @brief Helper class to provide type-safe access to aux data,
 *        specialized for @c JaggedVecElt.
 *
 * This is a version of @c Accessor, specialized for jagged vectors.
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
 *   static const SG::Accessor<SG::JaggedVecElt<Cont_t> jvec ("jvec");
 *   ...
 *   DataVector<MyClass>* v = ...;
 *   Myclass* m = v->at(2);
 *   jvec (*m)[3] = 10;
 *   for (auto sp : jvec.getDataSpan (*v)) {
 *     for (int& x : sp) ...
 *   }
 @endcode
 *
 * You can also use this to define getters/setters in your class:
 *
 *@code
 *  class Myclass {
 *    ...
 *    SG::ConstAccessor<SG::JaggedVecElt<int> >::element_type
 *    get_vec() const
 *    { const static SG::ConstAccessor<SG::JaggedVecElt<int> > acc ("vec");
 *      return acc (*this); }
 *    void set_vec (const std::vector<int>& v)
 *    { const static SG::Accessor<SG::JaggedVecElt<int> > acc ("vec");
 *      acc (*this) = v; }
 @endcode
*/
template <class PAYLOAD_T, class ALLOC>
class Accessor<JaggedVecElt<PAYLOAD_T>, ALLOC>
  : public ConstAccessor<JaggedVecElt<PAYLOAD_T>, ALLOC>
{
public:
  using Base = ConstAccessor<JaggedVecElt<PAYLOAD_T>, ALLOC>;

  /// Payload type.
  using Payload_t = typename Base::Payload_t;

  /// Allocator to use for the payload vector.
  using PayloadAlloc_t = typename std::allocator_traits<ALLOC>::template rebind_alloc<Payload_t>;

  /// One element of the jagged vector.
  using Elt_t = typename Base::Elt_t;

  /// Writable proxies for a jagged vector element: both a single proxy
  /// and one used as part of a span.
  using JVecProxy = detail::JaggedVecProxyT<Payload_t, detail::JaggedVecProxyValBase>;
  using JVecProxyInSpan = detail::JaggedVecProxyT<Payload_t, detail::JaggedVecProxyRefBase>;

  /// Converter from @c JaggedVecElt to a (writable) span.
  using Converter_t = detail::JaggedVecConverter<Payload_t>;

  /// Spans over the objects that are actually stored.
  using Elt_span = typename AuxDataTraits<Elt_t, ALLOC>::span;
  using Payload_span = typename AuxDataTraits<Payload_t, PayloadAlloc_t>::span;

  /// Span over the entire jagged vector.
  using span = CxxUtils::transform_view_with_at<Elt_span, Converter_t>;

  /// The writable type we return.
  using reference_type = JVecProxy;

  /// Not supported.
  using container_pointer_type = void;
  using const_container_pointer_type = void;

  /// Get the const implementations of these from the base class.
  using Base::operator();
  using Base::getDataSpan;
  using Base::getEltArray;
  using Base::getPayloadArray;
  using Base::getEltSpan;
  using Base::getPayloadSpan;


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
   * @brief Fetch the variable for one element, as a non-const reference.
   * @param e The element for which to fetch the variable.
   *
   * Will return a proxy object, which will allow treating this
   * jagged vector element as a vector.
   */
  template <IsAuxElement ELT>
  reference_type operator() (ELT& e) const;


  /**
   * @brief Fetch the variable for one element, as a non-const reference.
   * @param container The container from which to fetch the variable.
   * @param index The index of the desired element.
   *
   * This allows retrieving aux data by container / index.
   *
   * Will return a proxy object, which will allow treating this
   * jagged vector element as a vector.
   */
  reference_type operator() (AuxVectorData& container, size_t index) const;


  /**
   * @brief Set the variable for one element.
   * @param e The element for which to fetch the variable.
   * @param x The variable value to set.
   */
  template <CxxUtils::InputRangeOverT<PAYLOAD_T> RANGE>
  void set (AuxElement& e, const RANGE& x) const;


  /**
   * @brief Set the variable for one element.
   * @param container The container from which to fetch the variable.
   * @param index The index of the desired element.
   * @param x The variable value to set.
   */
  template <CxxUtils::InputRangeOverT<PAYLOAD_T> RANGE>
  void set (AuxVectorData& container, size_t index, const RANGE& x) const;


  /**
   * @brief Get a pointer to the start of the array of @c JaggedVecElt objects.
   * @param container The container from which to fetch the variable.
   */
  Elt_t* getEltArray (AuxVectorData& container) const;


  /**
   * @brief Get a pointer to the start of the payload array.
   * @param container The container from which to fetch the variable.
   */
  Payload_t* getPayloadArray (AuxVectorData& container) const;


  /**
   * @brief Get a span over the array of @c JaggedVecElt objects.
   * @param container The container from which to fetch the variable.
   */
  Elt_span getEltSpan (AuxVectorData& container) const;


  /**
   * @brief Get a span over the payload vector.
   * @param container The container from which to fetch the variable.
   */
  Payload_span getPayloadSpan (AuxVectorData& container) const;


  /**
   * @brief Get a span over spans representing the jagged vector.
   * @param container The container from which to fetch the variable.
   */
  span getDataSpan (AuxVectorData& container) const;


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


} // namespace SG


#include "AthContainers/JaggedVecAccessor.icc"


#endif // not ATHCONTAINERS_JAGGEDVECACCESSOR_H
