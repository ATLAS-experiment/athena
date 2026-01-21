/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_VARIANT_VARIANT_OPT_OBJECT_ID_H
#define COLUMNAR_VARIANT_VARIANT_OPT_OBJECT_ID_H

#include <ColumnarCore/OptObjectId.h>
#include <ColumnarVariant/VariantDef.h>
#include <ColumnarVariant/VariantObjectId.h>

namespace columnar
{
  template<ContainerIdConcept... CIList>
  class OptObjectId<VariantContainerId<CIList...>,ColumnarModeXAOD> final
  {
    /// Common Public Members
    /// =====================
  public:

    using CI = VariantContainerId<CIList...>;
    using CM = ColumnarModeXAOD;
    using xAODObject = typename CI::xAODObjectIdType;

    OptObjectId () noexcept = default;

    OptObjectId (std::nullopt_t) noexcept {}

    OptObjectId (const ObjectId<CI,ColumnarModeXAOD>& val_object) noexcept
      : m_object (&val_object.getXAODObjectNoexcept())
    {}

    OptObjectId (xAODObject *val_object) noexcept
      : m_object (val_object)
    {}

    template<ContainerIdConcept CI2>
      requires (CI::template isValidContainer<CI2>())
    OptObjectId (ObjectId<CI2,ColumnarModeXAOD> val_object) noexcept
      : m_object (&val_object.getXAODObjectNoexcept())
    {}

    template<ContainerIdConcept CI2>
      requires (CI::template isValidContainer<CI2>())
    OptObjectId (OptObjectId<CI2,ColumnarModeXAOD> val_object) noexcept
      : m_object (val_object.getXAODObjectNoexcept())
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

  template<ContainerIdConcept... CIList>
  std::ostream& operator<< (std::ostream& str, const OptObjectId<VariantContainerId<CIList...>,ColumnarModeXAOD>& obj)
  {
    return str << &obj.getXAODObjectNoexcept() << "/" << obj.getXAODObjectNoexcept().index();
  }

  template<ContainerIdConcept... CIList>
  bool operator== (const OptObjectId<VariantContainerId<CIList...>,ColumnarModeXAOD>& lhs, const OptObjectId<VariantContainerId<CIList...>,ColumnarModeXAOD>& rhs)
  {
    return &lhs.getXAODObjectNoexcept() == &rhs.getXAODObjectNoexcept();
  }

  template<ContainerIdConcept... CIList>
  bool operator!= (const OptObjectId<VariantContainerId<CIList...>,ColumnarModeXAOD>& lhs, const OptObjectId<VariantContainerId<CIList...>,ColumnarModeXAOD>& rhs)
  {
    return &lhs.getXAODObjectNoexcept() != &rhs.getXAODObjectNoexcept();
  }




  template<ContainerIdConcept... CIList, ColumnarArrayMode CM>
  class OptObjectId<VariantContainerId<CIList...>,CM> final
  {
    /// Common Public Members
    /// =====================
  public:

    using CI = VariantContainerId<CIList...>;
    using xAODObject = typename CI::xAODObjectIdType;
    static constexpr std::size_t invalidVariantIndex = CI::numVariants;

    OptObjectId () noexcept = default;

    OptObjectId (std::nullopt_t) noexcept {}

    OptObjectId (ObjectId<CI,CM> val_object) noexcept
      : m_data (val_object.getData()), m_variantIndex (val_object.getVariantIndex()), m_objectIndex (val_object.getObjectIndex())
    {}

    template<ContainerIdConcept CI2>
      requires (CI::template isValidContainer<CI2>())
    OptObjectId (ObjectId<CI2,CM> val_object) noexcept
      : m_data (val_object.getData()), m_variantIndex (CI::template getVariantIndex<CI2>()), m_objectIndex (val_object.getIndex())
    {}

    template<ContainerIdConcept CI2>
      requires (CI::template isValidContainer<CI2>())
    OptObjectId (OptObjectId<CI2,CM> val_object) noexcept
      : m_data (val_object.getData())
    {
      if (val_object.has_value())
      {
        m_variantIndex = CI::template getVariantIndex<CI2>();
        m_objectIndex = val_object.getIndex();
      } else
      {
        m_variantIndex = invalidVariantIndex;
        m_objectIndex = invalidObjectIndex;
      }
    }

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
      return m_variantIndex != invalidVariantIndex;}

    [[nodiscard]] bool has_value () const noexcept {
      return m_variantIndex != invalidVariantIndex;}

    [[nodiscard]] ObjectId<CI,CM> value () const {
      if (m_variantIndex == invalidVariantIndex)
        throw std::bad_optional_access();
      return ObjectId<CI,CM> (m_data, m_variantIndex, m_objectIndex);}

    [[nodiscard]] ObjectId<CI,CM> operator * () const {
      if (m_variantIndex == invalidVariantIndex)
        throw std::bad_optional_access();
      return ObjectId<CI,CM> (m_data, m_variantIndex, m_objectIndex);
    }

    [[nodiscard]] bool operator == (const OptObjectId<CI,CM>& that) const noexcept {
      return m_variantIndex == that.m_variantIndex && m_objectIndex == that.m_objectIndex;
    }



    /// Mode-Specific Public Members
    /// ============================
  public:

    explicit OptObjectId (void **val_data, std::size_t val_variantIndex, std::size_t val_objectIndex) noexcept
      : m_data (val_data), m_variantIndex (val_variantIndex), m_objectIndex (val_objectIndex)
    {}

    [[nodiscard]] std::size_t getVariantIndex () const noexcept {
      return m_variantIndex;}

    [[nodiscard]] std::size_t getObjectIndex () const noexcept {
      return m_objectIndex;}

    [[nodiscard]] void **getData () const noexcept {
      return m_data;}



    /// Private Members
    /// ===============
  private:

    void **m_data = nullptr;
    std::size_t m_variantIndex = invalidVariantIndex;
    std::size_t m_objectIndex = invalidObjectIndex;
  };

  template<ContainerIdConcept... CIList, ColumnarArrayMode CM>
  std::ostream& operator<< (std::ostream& str, const OptObjectId<VariantContainerId<CIList...>,CM>& obj)
  {
    using CI = VariantContainerId<CIList...>;
    return str << CI::idNameArray.at(obj.getVariantIndex()) << "/" << obj.getObjectIndex();
  }

  template<ContainerIdConcept... CIList, ColumnarArrayMode CM>
  bool operator== (const OptObjectId<VariantContainerId<CIList...>,CM>& lhs, const OptObjectId<VariantContainerId<CIList...>,CM>& rhs)
  {
    return lhs.getVariantIndex() == rhs.getVariantIndex() && lhs.getObjectIndex() == rhs.getObjectIndex();
  }

  template<ContainerIdConcept... CIList, ColumnarArrayMode CM>
  bool operator!= (const OptObjectId<VariantContainerId<CIList...>,CM>& lhs, const OptObjectId<VariantContainerId<CIList...>,CM>& rhs)
  {
    return lhs.getVariantIndex() != rhs.getVariantIndex() || lhs.getObjectIndex() != rhs.getObjectIndex();
  }
}

#endif
