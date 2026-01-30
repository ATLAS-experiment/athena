/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_VARIANT_VARIANT_OBJECT_ID_H
#define COLUMNAR_VARIANT_VARIANT_OBJECT_ID_H

#include <ColumnarCore/OptObjectId.h>
#include <ColumnarVariant/VariantDef.h>

namespace columnar
{
  template<ContainerIdConcept... CIList>
  class ObjectId<VariantContainerId<CIList...>,ColumnarModeXAOD> final
  {
    /// Common Public Members
    /// =====================
  public:

    using CI = VariantContainerId<CIList...>;
    using CM = ColumnarModeXAOD;
    using xAODObject = typename CI::xAODObjectIdType;

    ObjectId (xAODObject& val_object) noexcept
      : m_object (&val_object)
    {}

    ObjectId (const ObjectId<VariantContainerId<CIList...>,ColumnarModeXAOD>& that) noexcept = default;

    template<ContainerIdConcept CI2>
      requires (CI::template isValidContainer<CI2>())
    ObjectId (const ObjectId<CI2,CM>& that) noexcept
      : m_object (&that.getXAODObject())
    {}

    ObjectId& operator = (const ObjectId<VariantContainerId<CIList...>,ColumnarModeXAOD>& that) noexcept = default;

    [[nodiscard]] xAODObject& getXAODObject () const noexcept {
      // This object should ever be held within the context of a
      // single thread (and generally on the stack), so the associated
      // check is meaningless.
      auto *result ATLAS_THREAD_SAFE = m_object;
      return *result;}

    // a version of `getXAODObject` that only exists when it is `noexcept`
    [[nodiscard]] xAODObject& getXAODObjectNoexcept () const noexcept {
      // This object should ever be held within the context of a
      // single thread (and generally on the stack), so the associated
      // check is meaningless.
      auto *result ATLAS_THREAD_SAFE = m_object;
      return *result;}

    template<ContainerIdConcept CI2>
      requires (CI::template isValidContainer<CI2>())
    [[nodiscard]] OptObjectId<CI2,CM> tryGetVariant () const
    {
      return OptObjectId<CI2,CM> (dynamic_cast<typename CI2::xAODObjectIdType*>(m_object));
    }

    template<typename Acc,typename... Args>
      requires std::invocable<Acc,ObjectId<VariantContainerId<CIList...>,ColumnarModeXAOD>,Args...>
    [[nodiscard]] decltype(auto) operator() (Acc& acc, Args&&... args) const {
      return acc (*this, std::forward<Args> (args)...);}


    /// Mode-Specific Public Members
    /// ============================
  public:

    [[nodiscard]] ObjectId<typename CI::baseId,CM> getBaseObject () const
    {
      return ObjectId<typename CI::baseId,CM> (*m_object);
    }


    /// Private Members
    /// ===============
  private:

    xAODObject *m_object = nullptr;
  };

  template<ContainerIdConcept... CIList>
  std::ostream& operator<< (std::ostream& str, const ObjectId<VariantContainerId<CIList...>,ColumnarModeXAOD>& obj)
  {
    return str << &obj.getXAODObjectNoexcept() << "/" << obj.getXAODObjectNoexcept().index();
  }

  template<ContainerIdConcept... CIList>
  bool operator== (const ObjectId<VariantContainerId<CIList...>,ColumnarModeXAOD>& lhs, const ObjectId<VariantContainerId<CIList...>,ColumnarModeXAOD>& rhs)
  {
    return &lhs.getXAODObjectNoexcept() == &rhs.getXAODObjectNoexcept();
  }

  template<ContainerIdConcept... CIList>
  bool operator!= (const ObjectId<VariantContainerId<CIList...>,ColumnarModeXAOD>& lhs, const ObjectId<VariantContainerId<CIList...>,ColumnarModeXAOD>& rhs)
  {
    return &lhs.getXAODObjectNoexcept() != &rhs.getXAODObjectNoexcept();
  }




  template<ContainerIdConcept... CIList, ColumnarArrayMode CM>
  class ObjectId<VariantContainerId<CIList...>,CM> final
  {
    /// Common Public Members
    /// =====================
  public:

    using CI = VariantContainerId<CIList...>;
    using xAODObject = typename CI::xAODObjectIdType;

    // Whatever you do: Do not remove this function. Yes, it will always
    // throw. It is meant to throw in this template specialization, and
    // only do something useful in the xAOD mode specialization. If you
    // remove it you break the columnar mode.
    ObjectId (xAODObject& /*val_object*/)
    {
      throw std::logic_error ("can't call xAOD function in columnar mode");
    }

    ObjectId (const ObjectId<VariantContainerId<CIList...>,CM>& that) noexcept = default;

    template<ContainerIdConcept CI2>
      requires (CI2::regularObjectId && CI::template isValidContainer<CI2>())
    ObjectId (const ObjectId<CI2,CM>& that) noexcept
      : m_data (that.getData()), m_variantIndex (CI::template getVariantIndex<CI2>()), m_objectIndex (that.getIndex())
    {}

    ObjectId& operator = (const ObjectId<VariantContainerId<CIList...>,CM>& that) noexcept = default;
    // Whatever you do: Do not remove this function. Yes, it will always
    // throw. It is meant to throw in this template specialization, and
    // only do something useful in the xAOD mode specialization. If you
    // remove it you break the columnar mode.
    [[nodiscard]] xAODObject& getXAODObject () const {
      throw std::logic_error ("can't call xAOD function in columnar mode");}

    template<ContainerIdConcept CI2>
      requires (CI::template isValidContainer<CI2>())
    [[nodiscard]] OptObjectId<CI2,CM> tryGetVariant () const
    {
      if (m_variantIndex == CI::template getVariantIndex<CI2>())
        return OptObjectId<CI2,CM> (m_data, m_objectIndex);
      else
        return OptObjectId<CI2,CM> ();
    }

    template<typename Acc,typename... Args>
      requires std::invocable<Acc,ObjectId<VariantContainerId<CIList...>,CM>,Args...>
    [[nodiscard]] decltype(auto) operator() (Acc& acc, Args&&... args) const {
      return acc (*this, std::forward<Args> (args)...);}



    /// Mode-Specific Public Members
    /// ============================
  public:

    explicit ObjectId (void **val_data, std::size_t val_variantIndex, std::size_t val_objectIndex) noexcept
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
    std::size_t m_variantIndex = 0u;
    std::size_t m_objectIndex = 0u;
  };

  template<ContainerIdConcept... CIList, ColumnarArrayMode CM>
  std::ostream& operator<< (std::ostream& str, const ObjectId<VariantContainerId<CIList...>,CM>& obj)
  {
    using CI = VariantContainerId<CIList...>;
    return str << CI::idNameArray.at(obj.getVariantIndex()) << "/" << obj.getObjectIndex();
  }

  template<ContainerIdConcept... CIList, ColumnarArrayMode CM>
  bool operator== (const ObjectId<VariantContainerId<CIList...>,CM>& lhs, const ObjectId<VariantContainerId<CIList...>,CM>& rhs)
  {
    return lhs.getVariantIndex() == rhs.getVariantIndex() && lhs.getObjectIndex() == rhs.getObjectIndex();
  }

  template<ContainerIdConcept... CIList, ColumnarArrayMode CM>
  bool operator!= (const ObjectId<VariantContainerId<CIList...>,CM>& lhs, const ObjectId<VariantContainerId<CIList...>,CM>& rhs)
  {
    return lhs.getVariantIndex() != rhs.getVariantIndex() || lhs.getObjectIndex() != rhs.getObjectIndex();
  }
}

#endif
