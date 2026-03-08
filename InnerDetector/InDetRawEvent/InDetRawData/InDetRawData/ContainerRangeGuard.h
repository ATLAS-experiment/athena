/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
  */
#ifndef PHASEII_CONTAINERRANGEGUARD_H
#define PHASEII_CONTAINERRANGEGUARD_H

namespace PhaseII {

// test whether the given container collection allows to ask for a new container
template <typename T_ContainerCollection>
concept hasDynamicContainerList = requires(T_ContainerCollection &a) { a.getNewContainerPtr(); };

// test whether the given container ptr has an associated container id.
template <typename T_ContainerPtr>
concept hasContainerId = requires(T_ContainerPtr &a) { a.containerId(); };
   
/// @brief Helper class to keep track of a range of elements added to the end of a container
///
/// The element range is defined by the size of the container when this guard is
/// constructed and the size of the container when the range is requested.
/// The container may additionally be identified by an id in the container
/// collection
/// Usage:
/// <verbatim>
/// ContainerRangeGuard<DataRange, ContainerPtr> range_guard(container_ptr);
/// for (const auto input : input_container) {
/// /*... add elements to container_ptr e.g. addDataForModule(container_collection, range_guard, ... ); */
/// }
/// if (!range_guard.empty()) { /* ... register range ... */ }
/// </verbatim>
template <typename T_RangeType, typename T_ContainerPtr>
class ContainerRangeGuard {
public:
   ContainerRangeGuard(T_ContainerPtr ptr) : m_ptr(ptr), m_startIndex(m_ptr->size()) {}
   /// @brief create a range for the elements added to the container since the guard was created.
   T_RangeType range() const {
      if constexpr(hasContainerId<T_ContainerPtr>) {
         return T_RangeType::makeDataRange(m_startIndex, m_ptr->size(),m_ptr.containerId());
      }
      else {
         return T_RangeType::makeDataRange(m_startIndex, m_ptr->size(),0u);
      }
   }
   std::size_t startIndex() const {
      return m_startIndex;
   }
   /// @brief return the pointer to the container which contains the element range of this range guard
   T_ContainerPtr &ptr() {
      return m_ptr;
   }
   /// @brief return the pointer to the container which contains the element range of this range guard (read only)
   const T_ContainerPtr &ptr() const {
      return m_ptr;
   }

   /// @brief return true if no elements have been added to the container since the construction of this guard.
   bool empty() const {
      return m_ptr->size() == m_startIndex;
   }
private:
   T_ContainerPtr m_ptr;
   std::size_t m_startIndex;
};
}

#endif
