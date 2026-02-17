/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
  */
#ifndef CONTAINERLIST_H
#define CONTAINERLIST_H

#include <span>
#include <vector>
#include <atomic>
#include <mutex>

template <typename T_Container>
struct DynamicContainerListHelper;

/// @brief Helper class which provides a possibly dynamically growing list of containers
///        suitable for a multi-threaded context.
/// The idea is that there is a reasonable estimate of the total number of needed
/// containers, however if the estimate was too optimistic the list can be grown.
/// The operation to grow the list is mutex protected.
template <typename T_Container>
class ContainerList
{
private:
   using T_ContainerPtr = T_Container *;
   using ContainerPtrList = T_ContainerPtr *;
public:
   friend struct DynamicContainerListHelper<T_Container>;

   /// @brief Create a container list for a certain number of containers.
   /// @param expected_max_container the number of expected containers this list will have (in most cases this should rather be too large than too small).
   /// If the initial guess was too optimistic the list will be grown,
   /// which adds some computational overhead (lock congestion).
   ContainerList(unsigned int expected_max_container)
   {
      m_container.emplace_back(expected_max_container); // create expected_max_container containers
      // and create and populate the container list.
      m_containerList = fillContainerList( 0u,
                                           m_container.back(),
                                           expected_max_container <=1
                                           ? std::span<T_ContainerPtr>(&m_singleElementContainerList,1u)
                                           : std::span<T_ContainerPtr>(m_containerListHistory.emplace_back(expected_max_container) ));
   }

   /// @brief The number of containers in this list.
   std::size_t size() const { return m_containerList.size(); }
   /// @brief Return the container with the given container index (read-write).a
   T_Container &operator[](unsigned int idx) {
      assert( idx < m_containerList.size() && m_containerList[idx]);
      return *(m_containerList[idx]);
   }
   /// @brief Return the container with the given container index (read-only).
   const T_Container &operator[](unsigned int idx) const {
      assert( idx < m_containerList.size() && m_containerList[idx]);
      return *(m_containerList[idx]);
   }

private:
   /// @brief Write pointers to containers in the given vector to a list starting at begin_index
   /// @param begin_index the pointer of the first element in container will be written to to container_list[begin_index] and so forth.
   /// @param container a vector of new containers which are to be registered in the given container list
   /// @param container_list a contiguous list of all the containers.
   static std::span<T_ContainerPtr> fillContainerList( unsigned int begin_idx,
                                                       std::vector<T_Container> &container,
                                                       std::span<T_ContainerPtr> container_list) {
      unsigned int end_idx=begin_idx+container.size();
      assert( end_idx <= container_list.size());
      for (unsigned int idx=begin_idx; idx < end_idx; ++idx) {
         container_list[idx]=&container[idx-begin_idx];
      }
      return container_list;
   }


   /// A container "list" which can only contain a single container
   T_Container                                              *m_singleElementContainerList{};
   /// The current container list
   std::span<T_ContainerPtr>                                 m_containerList;

   /// All containers
   std::vector< std::vector<T_Container> >    m_container;
   /// The history of the container lists where each subsequent element is an extended copy of
   /// the preceding list.
   std::vector< std::vector<T_ContainerPtr> > m_containerListHistory;
};

/// @brief Helper class which allows to dynamically grow the container list.
template <typename T_Container>
struct DynamicContainerListHelper {
   using T_ContainerPtr = T_Container *;

   /// @brief Helper which provides a pointer to a container and the index of the container in the container list.
   struct ContainerPtr {
      friend struct DynamicContainerListHelper;
   protected:
      ContainerPtr(T_Container *ptr, unsigned int container_idx) : m_container(ptr),m_containerIdx(container_idx) {}
   public:
      ContainerPtr(const ContainerPtr &a) = default;
      ContainerPtr(ContainerPtr &&a) = default;

      ContainerPtr &operator=(const ContainerPtr &a) = default;
      ContainerPtr &operator=(ContainerPtr &&a) = default;

      T_Container &operator*() { assert( m_container != nullptr); return *m_container; }
      const T_Container &operator*() const { assert( m_container != nullptr); return *m_container; }
      T_Container *operator->() { assert( m_container != nullptr); return m_container; }
      const T_Container *operator->() const { assert( m_container != nullptr); return m_container; }

      /// @brief Get the index of container in the container list.
      unsigned int containerId() const {
         return m_containerIdx;
      }

   private:
      T_Container *m_container;
      unsigned int m_containerIdx;
   };

   /// @brief If the specified entry does not exist in the  container list create an extended copy which contains this entry.
   /// @param container_idx The index which specifies the container in the container list
   /// @param container_list the container list.
   /// If container_idx specifies a container beyond this list, the current list is copied and extended to
   /// contain the specified container. The operation is mutex protected.
   void addLargerContainerList(unsigned int container_idx, ContainerList<T_Container> &container_list) {
      std::lock_guard<std::mutex> container_list_lock(m_containerListMutex);
      if (container_idx >= container_list.m_containerList.size()) {
         // create new container list with min_elements extra elements i.e. grow in chunks
         static constexpr std::size_t min_elments = (sizeof(T_ContainerPtr)>=32u ? 1ul : 64ul/sizeof(T_ContainerPtr)-1);
         const std::size_t new_size=(m_used+min_elments+1) & (~(min_elments));
         unsigned int n=new_size - container_list.m_containerList.size();
         // add a new container_list and add new empty containers
         container_list.m_container.emplace_back( n );
         container_list.m_containerListHistory.emplace_back( new_size );
         std::vector<T_ContainerPtr> &new_container_list = container_list.m_containerListHistory.back();
         std::vector<T_Container> &extra_container = container_list.m_container.back();
         assert(new_container_list.size()>=container_list.m_containerList.size());
         // copy pointers from original container list to new container list
         std::copy(container_list.m_containerList.begin(),container_list.m_containerList.end(),new_container_list.begin());
         // set pointers of new container_list
         container_list.m_containerList = ContainerList<T_Container>::fillContainerList(container_list.m_containerList.size(),
                                                                                        extra_container,
                                                                                        std::span(new_container_list) );
      }
   }

   /// @brief Get an unused container from the given container list.
   /// If the preallocated list of containers is exceeded. The container list will be extended.
   ContainerPtr getNewContainer(ContainerList<T_Container> &container_list) {
      unsigned int container_idx = m_used++;
      if (container_idx >= container_list.m_containerList.size()) {
         addLargerContainerList(container_idx, container_list);
      }
      return ContainerPtr(container_list.m_containerList[container_idx],container_idx);
   }

   std::mutex                m_containerListMutex;
   std::atomic<unsigned int> m_used{};
};

#endif
