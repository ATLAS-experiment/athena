/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_CORE_OPT_OBJECT_ID_H
#define COLUMNAR_CORE_OPT_OBJECT_ID_H

#include <ColumnarInterfaces/IColumnarTool.h>
#include <ColumnarCore/ObjectId.h>

namespace columnar
{
  /// @brief a class representing a single optional object (electron, muons, etc.)
  ///
  /// This essentially behaves like an `std::optional<ObjectId>`, and is
  /// used in cases in which a given object may or may not exist.  For
  /// xAOD only code this is typically handled by a pointer with
  /// `nullptr` taking the empty value.  This is its own type both for
  /// compactness and to allow a slightly more efficient representation
  /// internally.
  template<ContainerIdConcept CI, typename CM> class OptObjectId;





  template<ContainerIdConcept CI> class OptObjectId<CI,ColumnarModeXAOD> final
  {
    /// Common Public Members
    /// =====================
  public:

    using xAODObject = typename CI::xAODObjectIdType;

    OptObjectId () noexcept = default;

    OptObjectId (std::nullopt_t) noexcept {}

    OptObjectId (ObjectId<CI,ColumnarModeXAOD> val_object) noexcept
      : m_object (&val_object.getXAODObjectNoexcept())
    {}

    OptObjectId (xAODObject *val_object) noexcept
      : m_object (val_object)
    {}

    OptObjectId (const OptObjectId<CI,ColumnarModeXAOD>& that) noexcept = default;

    OptObjectId& operator = (const OptObjectId<CI,ColumnarModeXAOD>& that) noexcept = default;

    explicit operator bool () const noexcept {
      return m_object != nullptr;}

    [[nodiscard]] bool has_value () const noexcept {
      return m_object != nullptr;}

    [[nodiscard]] ObjectId<CI,ColumnarModeXAOD> value () const {
      if (m_object == nullptr)
        throw std::bad_optional_access();
      // This object should ever be held within the context of a
      // single thread (and generally on the stack), so the associated
      // check is meaningless.
      auto *result ATLAS_THREAD_SAFE = m_object;
      return ObjectId<CI,ColumnarModeXAOD> (*result);}

    [[nodiscard]] ObjectId<CI,ColumnarModeXAOD> operator * () const {
      if (m_object == nullptr)
        throw std::bad_optional_access();
      return ObjectId<CI,ColumnarModeXAOD> (*m_object);}

    [[nodiscard]] xAODObject *getXAODObject () const noexcept {
      return m_object;}

    // a version of `getXAODObject` that only exists when it is `noexcept`
    [[nodiscard]] xAODObject *getXAODObjectNoexcept () const noexcept {
      return m_object;}

    [[nodiscard]] bool operator == (const OptObjectId<CI,ColumnarModeXAOD>& that) const noexcept {
      return m_object == that.m_object;}



    /// Private Members
    /// ===============
  private:

    xAODObject *m_object = nullptr;
  };

  template<ContainerIdConcept CI>
  bool operator== (const OptObjectId<CI,ColumnarModeXAOD>& lhs, const OptObjectId<CI,ColumnarModeXAOD>& rhs)
  {
    return lhs.getXAODObjectNoexcept() == rhs.getXAODObjectNoexcept();
  }

  template<ContainerIdConcept CI>
  bool operator!= (const OptObjectId<CI,ColumnarModeXAOD>& lhs, const OptObjectId<CI,ColumnarModeXAOD>& rhs)
  {
    return lhs.getXAODObjectNoexcept() != rhs.getXAODObjectNoexcept();
  }




  template<ContainerIdConcept CI,ColumnarArrayMode CM> class OptObjectId<CI,CM> final
  {
    /// Common Public Members
    /// =====================
  public:

    using xAODObject = typename CI::xAODObjectIdType;

    OptObjectId () noexcept = default;

    OptObjectId (std::nullopt_t) noexcept {}

    OptObjectId (ObjectId<CI,CM> val_object) noexcept
      : m_data (val_object.getData()), m_index (val_object.getIndex())
    {}

    // Whatever you do: Do not remove this function. Yes, it will always
    // throw. It is meant to throw in this template specialization, and
    // only do something useful in the xAOD mode specialization. If you
    // remove it you break the columnar mode.
    OptObjectId (xAODObject * /*val_object*/)
    {
      throw std::logic_error ("can't call xAOD function in columnar mode");
    }

    OptObjectId (const OptObjectId<CI,CM>& that) noexcept = default;

    OptObjectId& operator = (const OptObjectId<CI,CM>& that) noexcept = default;

    // Whatever you do: Do not remove this function. Yes, it will always
    // throw. It is meant to throw in this template specialization, and
    // only do something useful in the xAOD mode specialization. If you
    // remove it you break the columnar mode.
    [[nodiscard]] xAODObject *getXAODObject () const {
      throw std::logic_error ("can't call xAOD function in columnar mode");}

    explicit operator bool () const noexcept {
      return m_index != invalidObjectIndex;}

    [[nodiscard]] bool has_value () const noexcept {
      return m_index != invalidObjectIndex;}

    [[nodiscard]] ObjectId<CI,CM> value () const {
      if (m_index == invalidObjectIndex)
        throw std::bad_optional_access();
      return ObjectId<CI,CM> (m_data, m_index);}

    [[nodiscard]] ObjectId<CI,CM> operator * () const {
      if (m_index == invalidObjectIndex)
        throw std::bad_optional_access();
      return ObjectId<CI,CM> (m_data, m_index);}
  
    [[nodiscard]] bool operator == (const OptObjectId<CI,CM>& that) const noexcept {
      return m_index == that.m_index;}



    /// Mode-Specific Public Members
    /// ============================
  public:

    explicit OptObjectId (void **val_data, int val_index) noexcept
      : m_data (val_data), m_index (val_index)
    {}

    explicit OptObjectId (void **val_data, unsigned val_index) noexcept
      : m_data (val_data), m_index (val_index)
    {}

    explicit OptObjectId (void **val_data, std::size_t val_index) noexcept
      : m_data (val_data), m_index (val_index)
    {}

    [[nodiscard]] std::size_t getIndex () const noexcept {
      return m_index;}

    [[nodiscard]] void **getData () const noexcept {
      return m_data;}



    /// Private Members
    /// ===============
  private:

    void **m_data = nullptr;
    std::size_t m_index = invalidObjectIndex;
  };

  template<ContainerIdConcept CI, ColumnarArrayMode CM>
  bool operator== (const OptObjectId<CI,CM>& lhs, const OptObjectId<CI,CM>& rhs)
  {
    return lhs.getIndex() == rhs.getIndex();
  }

  template<ContainerIdConcept CI, ColumnarArrayMode CM>
  bool operator!= (const OptObjectId<CI,CM>& lhs, const OptObjectId<CI,CM>& rhs)
  {
    return lhs.getIndex() != rhs.getIndex();
  }
}

#endif
