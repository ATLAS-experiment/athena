/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
  */
#ifndef PHASEII_RAWDATACONTAINERMT_H
#define PHASEII_RAWDATACONTAINERMT_H

#include "PhaseIIInDetRawDataContainer.h"
#include "ContainerRangeGuard.h"

namespace PhaseII {
/// @brief Extended Helper class to associate ranges of elements in multiple containers to a contiguous index
///        to be used in a MT context
/// The base class is extended by a container list which can be grown dynamically (mutex protected),
/// and the per module element range is atomic.
/// If the initial guess for the container list is conservative enough, then the only additional overhead
/// are atomic operations when registering an element range per module, the atomic operation to
/// the increment the container used counter. If the initial guess was too optimistic, the
/// container list needs to be grown which is mutex protected, but will be done in chunks.
template <class T_RawDataContainer>
class IndexedRangesMT : public PhaseII::IndexedRanges<T_RawDataContainer, std::atomic<PhaseII::DataRange> >
{
public:
   typedef std::true_type thread_safe; //  to allow usage  with update handle
   using BASE = PhaseII::IndexedRanges<T_RawDataContainer, std::atomic<PhaseII::DataRange> >;

   using ContainerPtr = typename DynamicContainerListHelper<T_RawDataContainer>::ContainerPtr;
   IndexedRangesMT(unsigned int n_ranges, unsigned int n_slots) : BASE(n_ranges,n_slots) {}

   /// @brief get an unused container to add new RDOs.
   /// @return will return an used preallocated container or grow the container list and return a newly
   ///         allocated container.
   /// If the container list is not yet exhausted this only requires an atomic operation, otherwise
   /// new storage will need to be allocated which is mutex protected.
   ContainerPtr getNewContainerPtr() {
      return m_containerListHelper.getNewContainer(this->m_containers);
   }

   /// @brief Add a new container to the list of containers and return its index.
   /// if the container list capacity is not exhausted a new container list will be created with
   /// a larger capacity and the contents of the current list will be copied to it. All future
   /// requests will use that new list, but the old list is kept arround since there may still
   /// be users, and there is no reference counting.
   unsigned int getNewContainerIndex() {
      ContainerPtr container_ptr = getNewContainerPtr();
      unsigned int container_index = container_ptr.containerId();
      return container_index < this->slotsMax() ? container_index : std::numeric_limits<unsigned int>::max();
   }

private:
   DynamicContainerListHelper<T_RawDataContainer> m_containerListHelper;
};

/// @brief convenience method to add data to an RDO container, add a new RDO container, copy the data added
///        for the current module if the capacity of the original container is exhausted.
template <typename T_RawDataContainerCollection, typename T_RawDataContainerPtr, typename T_RangeType, typename T_Coordinates>
void addDataForModule(T_RawDataContainerCollection &rdo_container_collection,
                      ContainerRangeGuard<T_RawDataContainerPtr, T_RangeType> &range_guard,
                      T_Coordinates &&coordinates,
                      std::uint32_t data_word);

}
#include "PhaseIIInDetRawDataContainerMT.icc"
#endif
