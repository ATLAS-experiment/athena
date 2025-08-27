/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_CORE_OBJECT_ID_H
#define COLUMNAR_CORE_OBJECT_ID_H

#include <ColumnarCore/ContainerId.h>
#include <CxxUtils/checker_macros.h>
#include <iostream>
#include <stdexcept>

namespace columnar
{
  /// @brief a class representing a single object (electron, muons, etc.)
  template<ContainerIdConcept CI, typename CM> class ObjectId;





  template<ContainerIdConcept CI> class ObjectId<CI,ColumnarModeXAOD> final
  {
    /// Common Public Members
    /// =====================
  public:

    using xAODObject = typename CI::xAODObjectIdType;

    ObjectId (xAODObject& val_object) noexcept
      : m_object (&val_object)
    {}

    ObjectId (const ObjectId<CI,ColumnarModeXAOD>& that) noexcept = default;

    template<ContainerIdConcept CI2> requires (CI2::isMutable && std::is_same_v<typename CI2::constId,CI>)
    ObjectId (const ObjectId<CI2,ColumnarModeXAOD>& that) noexcept
      : m_object (&that.getXAODObjectNoexcept())
    {}

    ObjectId& operator = (const ObjectId<CI,ColumnarModeXAOD>& that) noexcept = default;

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

      template<typename Acc,typename... Args>
      requires std::invocable<Acc,ObjectId<CI,ColumnarModeXAOD>,Args...>
    [[nodiscard]] decltype(auto) operator() (Acc& acc, Args&&... args) const {
      return acc (*this, std::forward<Args> (args)...);}



    /// Private Members
    /// ===============
  private:

    xAODObject *m_object = nullptr;
  };

  template<ContainerIdConcept CI>
  std::ostream& operator<< (std::ostream& str, const ObjectId<CI,ColumnarModeXAOD>& obj)
  {
    return str << &obj.getXAODObjectNoexcept() << "/" << obj.getXAODObjectNoexcept().index();
  }

  template<ContainerIdConcept CI>
  bool operator== (const ObjectId<CI,ColumnarModeXAOD>& lhs, const ObjectId<CI,ColumnarModeXAOD>& rhs)
  {
    return &lhs.getXAODObjectNoexcept() == &rhs.getXAODObjectNoexcept();
  }

  template<ContainerIdConcept CI>
  bool operator!= (const ObjectId<CI,ColumnarModeXAOD>& lhs, const ObjectId<CI,ColumnarModeXAOD>& rhs)
  {
    return &lhs.getXAODObjectNoexcept() != &rhs.getXAODObjectNoexcept();
  }




  template<ContainerIdConcept CI> class ObjectId<CI,ColumnarModeArray> final
  {
    /// Common Public Members
    /// =====================
  public:

    using CM = ColumnarModeArray;
    using xAODObject = typename CI::xAODObjectIdType;

    // Whatever you do: Do not remove this function. Yes, it will always
    // throw. It is meant to throw in this template specialization, and
    // only do something useful in the xAOD mode specialization. If you
    // remove it you break the columnar mode.
    ObjectId (xAODObject& /*val_object*/)
    {
      throw std::logic_error ("can't call xAOD function in columnar mode");
    }

    ObjectId (const ObjectId<CI,ColumnarModeArray>& that) noexcept = default;

    template<ContainerIdConcept CI2> requires (CI2::isMutable && std::is_same_v<typename CI2::constId,CI>)
    ObjectId (const ObjectId<CI2,ColumnarModeArray>& that) noexcept
      : m_data (that.getData()), m_index (that.getIndex())
    {}

    ObjectId& operator = (const ObjectId<CI,ColumnarModeArray>& that) noexcept = default;

    // Whatever you do: Do not remove this function. Yes, it will always
    // throw. It is meant to throw in this template specialization, and
    // only do something useful in the xAOD mode specialization. If you
    // remove it you break the columnar mode.
    [[nodiscard]] xAODObject& getXAODObject () const {
      throw std::logic_error ("can't call xAOD function in columnar mode");}

    template<typename Acc,typename... Args>
      requires std::invocable<Acc,ObjectId<CI,ColumnarModeArray>,Args...>
    [[nodiscard]] decltype(auto) operator() (Acc& acc, Args&&... args) const {
      return acc (*this, std::forward<Args> (args)...);}



    /// Mode-Specific Public Members
    /// ============================
  public:

    explicit ObjectId (void **val_data, std::size_t val_index) noexcept
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
    std::size_t m_index = 0u;
  };

  template<ContainerIdConcept CI>
  std::ostream& operator<< (std::ostream& str, const ObjectId<CI,ColumnarModeArray>& obj)
  {
    return str << CI::idName << "/" << obj.getIndex();
  }

  template<ContainerIdConcept CI>
  bool operator== (const ObjectId<CI,ColumnarModeArray>& lhs, const ObjectId<CI,ColumnarModeArray>& rhs)
  {
    return lhs.getIndex() == rhs.getIndex();
  }

  template<ContainerIdConcept CI>
  bool operator!= (const ObjectId<CI,ColumnarModeArray>& lhs, const ObjectId<CI,ColumnarModeArray>& rhs)
  {
    return lhs.getIndex() != rhs.getIndex();
  }
}

#endif
