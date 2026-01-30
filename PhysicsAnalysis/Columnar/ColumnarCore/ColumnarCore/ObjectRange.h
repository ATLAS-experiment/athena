/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_CORE_OBJECT_RANGE_H
#define COLUMNAR_CORE_OBJECT_RANGE_H

#include <ColumnarCore/ContainerId.h>
#include <ColumnarCore/ObjectId.h>
#include <CxxUtils/checker_macros.h>
#include <exception>

namespace columnar
{
  /// @brief a class representing a continuous sequence of objects (a.k.a. a container)
  template<ContainerIdConcept CI,typename CM> class ObjectRange;




  template<ContainerIdConcept CI,typename IteratorType> class ObjectRangeIteratorXAODContainer;

  template<ContainerIdConcept CI> class ObjectRange<CI,ColumnarModeXAOD> final
  {
    /// Common Public Members
    /// =====================
  public:

    using xAODContainer = typename CI::xAODObjectRangeType;
    using CM = ColumnarModeXAOD;

    ObjectRange (xAODContainer& val_container) noexcept
      : m_container (&val_container)
    {}

    [[nodiscard]] xAODContainer& getXAODObject () const noexcept {
      // This object should ever be held within the context of a
      // single thread (and generally on the stack), so the associated
      // check is meaningless.
      auto *container ATLAS_THREAD_SAFE = m_container;
      return *container;}

    // a version of `getXAODObject` that only exists when it is `noexcept`
    [[nodiscard]] xAODContainer& getXAODObjectNoexcept () const noexcept {
      // This object should ever be held within the context of a
      // single thread (and generally on the stack), so the associated
      // check is meaningless.
      auto *container ATLAS_THREAD_SAFE = m_container;
      return *container;}

    auto begin () const noexcept {
      // This object should ever be held within the context of a
      // single thread (and generally on the stack), so the associated
      // check is meaningless.
      auto *container ATLAS_THREAD_SAFE = m_container;
      return ObjectRangeIteratorXAODContainer<CI,decltype(container->begin())> (container->begin());}
    auto end () const noexcept {
      // This object should ever be held within the context of a
      // single thread (and generally on the stack), so the associated
      // check is meaningless.
      auto *container ATLAS_THREAD_SAFE = m_container;
      return ObjectRangeIteratorXAODContainer<CI,decltype(container->end())> (container->end());}
    auto rbegin () const noexcept {
      // This object should ever be held within the context of a
      // single thread (and generally on the stack), so the associated
      // check is meaningless.
      auto *container ATLAS_THREAD_SAFE = m_container;
      return ObjectRangeIteratorXAODContainer<CI,decltype(container->rbegin())> (container->rbegin());}
    auto rend () const noexcept {
      // This object should ever be held within the context of a
      // single thread (and generally on the stack), so the associated
      // check is meaningless.
      auto *container ATLAS_THREAD_SAFE = m_container;
      return ObjectRangeIteratorXAODContainer<CI,decltype(container->rend())> (container->rend());}

    [[nodiscard]] bool empty () const noexcept {
      return m_container->empty();}

    [[nodiscard]] std::size_t size () const noexcept {
      return m_container->size();}

    [[nodiscard]] ObjectId<CI,CM> operator [] (std::size_t index) const noexcept {
      return ObjectId<CI,CM> (*(*m_container)[index]);}

    template<typename Acc,typename... Args>
      requires std::invocable<Acc,ObjectRange<CI,ColumnarModeXAOD>,Args...>
    [[nodiscard]] decltype(auto) operator() (Acc& acc, Args&&... args) const {
      return acc (*this, std::forward<Args> (args)...);}

    /// get the index inside the given range
    [[nodiscard]] std::size_t getIndexInRange (const ObjectId<CI,CM>& obj) const {
      return obj.getXAODObjectNoexcept().index(); }



    /// Private Members
    /// ===============
  private:

    xAODContainer *m_container = nullptr;
  };

  template<ContainerIdConcept CI,typename IteratorType> class ObjectRangeIteratorXAODContainer final
  {
  public:

    using CM = ColumnarModeXAOD;

    ObjectRangeIteratorXAODContainer (IteratorType&& val_iterator) noexcept
      : m_iterator (std::move (val_iterator)) {}

    ObjectId<CI,CM> operator * () const noexcept {
      return ObjectId<CI,CM> (**m_iterator);
    }

    ObjectRangeIteratorXAODContainer<CI,IteratorType>& operator ++ () noexcept {
      ++ m_iterator; return *this;}

    bool operator == (const ObjectRangeIteratorXAODContainer<CI,IteratorType>& that) const noexcept {
      return m_iterator == that.m_iterator;}
    bool operator != (const ObjectRangeIteratorXAODContainer<CI,IteratorType>& that) const noexcept {
      return m_iterator != that.m_iterator;}

  private:
    IteratorType m_iterator;
  };



  template<ContainerIdConcept CI> class ObjectRangeIteratorXAODSinglet;

  // template specialization for EventInfo objects (and potentially other singlet objects)
  template<ContainerIdConcept CI>
      requires (std::is_same_v<typename CI::xAODObjectRangeType,typename CI::xAODObjectIdType>)
  class ObjectRange<CI,ColumnarModeXAOD> final
  {
    /// Common Public Members
    /// =====================
  public:

    using xAODContainer = typename CI::xAODObjectRangeType;
    using CM = ColumnarModeXAOD;

    ObjectRange (xAODContainer& val_singlet) noexcept
      : m_singlet (&val_singlet)
    {}

    [[nodiscard]] xAODContainer& getXAODObject () const noexcept {
      return *m_singlet;}

    // a version of `getXAODObject` that only exists when it is `noexcept`
    [[nodiscard]] xAODContainer& getXAODObjectNoexcept () const noexcept {
      return *m_singlet;}

    auto begin () const noexcept {
      return ObjectRangeIteratorXAODSinglet<CI> (m_singlet);}
    auto end () const noexcept {
      return ObjectRangeIteratorXAODSinglet<CI> (nullptr);}
    auto rbegin () const noexcept {
      return ObjectRangeIteratorXAODSinglet<CI> (m_singlet);}
    auto rend () const noexcept {
      return ObjectRangeIteratorXAODSinglet<CI> (nullptr);}

    [[nodiscard]] bool empty () const noexcept {
      return false;}

    [[nodiscard]] std::size_t size () const noexcept {
      return 1;}

    [[nodiscard]] ObjectId<CI,CM> operator [] (std::size_t /*index*/) const noexcept {
      return ObjectId<CI,CM> (*m_singlet);
    }

    template<typename Acc,typename... Args>
      requires std::invocable<Acc,ObjectRange<CI,ColumnarModeXAOD>,Args...>
    [[nodiscard]] decltype(auto) operator() (Acc& acc, Args&&... args) const {
      return acc (*this, std::forward<Args> (args)...);}



    /// Private Members
    /// ===============
  private:

    xAODContainer *m_singlet = nullptr;
  };

  template<ContainerIdConcept CI> class ObjectRangeIteratorXAODSinglet final
  {
  public:

    using CM = ColumnarModeXAOD;
    using XAODObjectType = typename CI::xAODObjectIdType;

    ObjectRangeIteratorXAODSinglet (XAODObjectType *val_object) noexcept
      : m_object (val_object) {}

    ObjectId<CI,CM> operator * () const noexcept {
      return *m_object;}

    ObjectRangeIteratorXAODSinglet<CI>& operator ++ () noexcept {
      m_object = nullptr; return *this;}

    bool operator == (const ObjectRangeIteratorXAODSinglet<CI>& that) const noexcept {
      return m_object == that.m_object;}
    bool operator != (const ObjectRangeIteratorXAODSinglet<CI>& that) const noexcept {
      return m_object != that.m_object;}

  private:
    XAODObjectType *m_object = nullptr;
  };




  template<ContainerIdConcept CI,int stepSize,ColumnarArrayMode CM>
  class ObjectRangeIteratorArray;

  template<ContainerIdConcept CI,ColumnarArrayMode CM> class ObjectRange<CI,CM> final
  {
    /// Common Public Members
    /// =====================
  public:

    using xAODContainer = typename CI::xAODObjectRangeType;

    ObjectRangeIteratorArray<CI,1,CM> begin () const noexcept {
      return ObjectRangeIteratorArray<CI,1,CM> (m_data, m_beginIndex);}
    ObjectRangeIteratorArray<CI,1,CM> end () const noexcept {
      return ObjectRangeIteratorArray<CI,1,CM> (m_data, m_endIndex);}
    ObjectRangeIteratorArray<CI,-1,CM> rbegin () const noexcept {
      // note that as a reverse iterator, the meaning of begin and end
      // is reversed, and the new "end" can be -1.
      return ObjectRangeIteratorArray<CI,-1,CM> (m_data, m_endIndex-1);}
    ObjectRangeIteratorArray<CI,-1,CM> rend () const noexcept {
      // note that as a reverse iterator, the meaning of begin and end
      // is reversed, and the new "end" can be -1.
      return ObjectRangeIteratorArray<CI,-1,CM> (m_data, m_beginIndex-1);}

    [[nodiscard]] std::size_t beginIndex () const noexcept {
      return m_beginIndex;}
    [[nodiscard]] std::size_t endIndex () const noexcept {
      return m_endIndex;}

    // Whatever you do: Do not remove this function. Yes, it will always
    // throw. It is meant to throw in this template specialization, and
    // only do something useful in the xAOD mode specialization. If you
    // remove it you break the columnar mode.
    ObjectRange (const xAODContainer& /*val_container*/)
    {
      throw std::logic_error ("can't call xAOD function in columnar mode");
    }

    [[nodiscard]] bool empty () const noexcept {
      return m_beginIndex == m_endIndex;}

    [[nodiscard]] std::size_t size () const noexcept {
      return m_endIndex - m_beginIndex;}

    // Whatever you do: Do not remove this function. Yes, it will always
    // throw. It is meant to throw in this template specialization, and
    // only do something useful in the xAOD mode specialization. If you
    // remove it you break the columnar mode.
    [[nodiscard]] xAODContainer& getXAODObject () const {
      throw std::logic_error ("can't call xAOD function in columnar mode");}

    [[nodiscard]] ObjectId<CI,CM> operator [] (std::size_t index) const noexcept {
      return ObjectId<CI,CM> (m_data, index + m_beginIndex);
    }

    template<typename Acc,typename... Args>
      requires std::invocable<Acc,ObjectRange<CI,CM>,Args...>
    [[nodiscard]] decltype(auto) operator() (Acc& acc, Args&&... args) const {
      return acc (*this, std::forward<Args> (args)...);}

    /// get the index inside the given range
    [[nodiscard]] std::size_t getIndexInRange (const ObjectId<CI,CM>& obj) const {
      return obj.getIndex() - m_beginIndex; }



    /// Mode-Specific Public Members
    /// ============================
  public:

    explicit ObjectRange (void **val_data, std::size_t val_beginIndex,
                          std::size_t val_endIndex) noexcept
      : m_data (val_data), m_beginIndex (val_beginIndex), m_endIndex (val_endIndex)
    {}

    [[nodiscard]] void **getData () const noexcept {
      return m_data;}



    /// Private Members
    /// ===============
  private:

    void **m_data = nullptr;
    std::size_t m_beginIndex = 0u;
    std::size_t m_endIndex = 0u;
  };

  /// @brief an iterator over objects in an @ref ObjectRange
  ///
  /// This is primarily to allow the use of range-for for ObjectRange

  template<ContainerIdConcept CI,int stepSize,ColumnarArrayMode CM>
  class ObjectRangeIteratorArray final
  {
  public:

    ObjectRangeIteratorArray (void **val_data, std::size_t val_index) noexcept
      : m_data (val_data), m_index (val_index) {}

    ObjectId<CI,CM> operator * () const noexcept {
      return ObjectId<CI,CM> (m_data, m_index);
    }

    ObjectRangeIteratorArray<CI,stepSize,CM>& operator ++ () noexcept {
      m_index += stepSize; return *this;}

    bool operator == (const ObjectRangeIteratorArray<CI,stepSize,CM>& that) const noexcept {
      return m_index == that.m_index;}
    bool operator != (const ObjectRangeIteratorArray<CI,stepSize,CM>& that) const noexcept {
      return m_index != that.m_index;}

  private:
    void **m_data = nullptr;
    std::size_t m_index = 0u;
  };
}

#endif
