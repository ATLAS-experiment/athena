/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
  */
#ifndef UTILS_PROXY_CONTAINER_H
#define UTILS_PROXY_CONTAINER_H

#include <limits>
#include <cassert>
// Helper classes to construct proxy container objects
// to handle iteration over containers like jagged vectors.
// A use case would be a container which contains containers
// of objects where the elements of the lowest level objects
// of all containers are contained in a single buffer, or
// in a SOA like arrangement, and the sub-containers are realized
// by index ranges. Example, for such a hierarchy:
//    container[module_i][cluster_i][cell_i].cellId()
//    container[module_i][cluster_i][cell_i].cellPosition()
// where the underlying container has the following structure:
//  struct CellData {
//      std::vector<int>   m_cellId;
//      std::vector<float> m_cellPosition;
//      std:vector<std::pair<unsigned int, unsigned int> > m_clusterCellIndexRange;
//      std:vector<std::pair<unsigned int, unsigned int> > m_moduleClusterIndexRange;
//  };

namespace Utils {

/// @breif Indicate whether a container provides read only or read write access
enum class AccessPolicy {
   ReadOnly,
   ReadWrite
};

// test whether a container has a size() member function
template <typename Container>
concept hasSize = requires(Container a) { a.size(); };

// test whether the difference can be computed for a certain index type
template <typename IndexType>
concept hasDifference = requires(IndexType a, IndexType b) { a-b; };

// test whether a count can be added to a certain index type
template <typename IndexType>
concept hasAddition = requires(IndexType a, std::size_t count) { a+count; };

// test whether a proxy is meant to be temporary and provides a method to convert to the actual proxy to be used.
// Such a temporary proxy has to provide the member functions:
// class ElementProxy ... { ..
//       static ActualProxy ElementProxy::create(ContainerType *container, IndexType index);
//       static ActualProxy ElementProxy::create(const ContainerType *container, IndexType index);
// ... };
// For such cases iterators, or the element access operators ([], front, back), would yield the
// type "ActualProxy". The Element Proxy should define at the same time also a method getOriginalElementIndex
// to "compute" the "child" index from the ActualProxy, where the child index is the index that can be
// used to to recover an element proxy from the parent proxy using the access operator[].
// class ElementProxy ... { ..
//       static ActualProxy ElementProxy::create(ContainerType *container, IndexType index);
//       static ActualProxy ElementProxy::create(const ContainerType *container, IndexType index);
//       static IndexType getOriginalElementIndex(const ActualProxy &converted_element_proxy);
// ... };
// auto child_proxy = parent_proxy[index];
// auto index_restore = ParentProxy::computeChildElementIndex(child_proxy);
// assert( index == index_restore )
template <typename T_Proxy, typename T_ContainerPtr, typename T_Index>
concept hasCreateProxy = requires(T_Proxy &&a, T_ContainerPtr ptr, T_Index index) { T_Proxy::create(ptr, index); };

template <typename T_Proxy>
concept hasReadWriteProxy = requires( typename T_Proxy::ReadWriteProxy a) { T_Proxy(a);};

template <typename T_DestProxy, typename T_SrcProxy>
concept isConvertableToReadOnlyProxy = requires( const T_SrcProxy &a) { T_DestProxy(&(a.container()), a.index());};


/// @brief Helper struct to indicate the "index" of a top level container proxy.
/// such a proxy does not have an index because there is only a single top
/// level element
struct RootNodeIndex {
};

// Forward declaration of the actual container proxy class
template <class Container,
          class T_Derived,
          class ElementProxy,
          typename RangeType=RootNodeIndex,
          AccessPolicy accessPolicy=AccessPolicy::ReadOnly>
struct ContainerProxy;

/// @brief Base class of a container proxy
/// @tparam Container the container type this proxy refers to e.g. CellData
/// @tparam ElementIndexType the index type to refer to  an element of the container proxy e.g. unsigned int,
///                    but it could be an object with associated data e.g. an index to the parent
///                    element or on index offset for its child elements
/// The base class provides an access controlled pointer to the container this proxy is referring to.
template <class Container,
          typename ElementIndexType,
          AccessPolicy accessPolicy=AccessPolicy::ReadOnly>
struct ContainerProxyBase {
   using element_index_t = ElementIndexType;

   // helper class to provide access control to a pointer i.e. read/write or read only.
   template <AccessPolicy ptrAccessPolicy=accessPolicy>
   struct ContainerPtrBase {
      template <AccessPolicy IteratorAccessPolicy>
      friend class iterator_base;

      using ContainerPtr = std::conditional_t< ptrAccessPolicy == AccessPolicy::ReadOnly, const Container *, Container *>;

      template <class _Container,
                class _T_Derived,
                class _ElementProxy,
                typename _RangeType,
                AccessPolicy _accessPolicy>
      friend struct ContainerProxy;

      template <AccessPolicy otherAccessPolicy>
      ContainerPtrBase(ContainerPtrBase<otherAccessPolicy> container) requires (   otherAccessPolicy==ptrAccessPolicy
                                                                                || otherAccessPolicy == AccessPolicy::ReadWrite)
         : m_container(container.m_container) {}

      ContainerPtrBase(const Container *container) requires (ptrAccessPolicy == AccessPolicy::ReadOnly)
         : m_container(container) {}

      ContainerPtrBase(Container *container)
         : m_container(container) {}

      // test whether the pointer is not nullptr
      bool isValid() const { return m_container != nullptr; }

      // return a const reference of the container
      // results are "undefined" if isValid has not been checked before
      const Container &container() const                                                      { assert(isValid()); return *m_container; }
      // return a non-const reference of the container if the access policy permits read write access.
      // Results are "undefined" if isValid has not been checked before
      Container &container()             requires(ptrAccessPolicy == AccessPolicy::ReadWrite) { assert(isValid()); return *m_container; }

      // return the const pointer of the container.
      // The  result can be a nullptr.
      const Container *cptr() const                                                      {return m_container; }
      // return a non-const pointer if the access policy permits read write access. The result can be a nullptr
      Container *ptr()              requires(ptrAccessPolicy == AccessPolicy::ReadWrite) {return m_container; }

      // convenience method to return const or non const pointer depending on
      // an external access policy.
      template <AccessPolicy otherAccessPolicy>
      auto accessPtr() requires(ptrAccessPolicy==AccessPolicy::ReadWrite) {
         if constexpr(otherAccessPolicy == AccessPolicy::ReadWrite) {
            return this->ptr();
         }
         else {
            return this->cptr();
         }
      }
      // convenience method to return const or non const pointer depending on
      // an external access policy, if access is restricted to read only access.
      template <AccessPolicy otherAccessPolicy>
      auto accessPtr() const
         requires(otherAccessPolicy==AccessPolicy::ReadOnly)
      {
         return this->cptr();
      }

      // true if the pointer is not nullptr
      operator bool() const { return isValid(); }

      // return a const reference of the container
      // Result is "undefined" if container is not valid;
      const Container &operator*() const                                                       { return container(); }
      // return a non const reference of the container if the access policy
      // permits read-write access. Result is "undefined" if container is not valid;
      Container &operator*()              requires(ptrAccessPolicy == AccessPolicy::ReadWrite) { return container(); }

      // test whether the two container pointers point to the same container
      template <AccessPolicy otherAccessPolicy>
      bool operator==(const ContainerPtrBase<otherAccessPolicy> &other) const  { return m_container == other.m_container; }
   private:
      ContainerPtr m_container;
   };

   using ContainerPtr = ContainerPtrBase<AccessPolicy::ReadWrite>;     // non-const access controlled container pointer
   using ConstContainerPtr = ContainerPtrBase<AccessPolicy::ReadOnly>; // const container pointer
   ContainerPtrBase<accessPolicy> m_container;                         // pointer to the container this proxy refers to

   ContainerProxyBase(const Container *container) : m_container(container) {}  // creates a proxy which provides read-only access to its elements
   ContainerProxyBase(Container *container)       : m_container(container) {}  // creates a proxy which provides read-write access to its elements

   // @brief Possible base class for an element proxy.
   // This base class is used by the proxy container iterators
   template <AccessPolicy ptrAccessPolicy=accessPolicy>
   struct ElementProxyBase {
      using index_t = ElementIndexType;

   protected:
      ContainerPtrBase<ptrAccessPolicy> m_container; // refers to the original container the parent proxy refers to
      index_t m_index;                               // an "index" which indicates the element this proxy refers to
   public:
      ElementProxyBase(const Container *container, const  index_t &index)  // create a read-only element proxy
         : m_container(container), m_index(index)
      {}
      ElementProxyBase(Container *container, const index_t &index)         // create a read-write element proxy
         : m_container(container), m_index(index)
      {}
      ElementProxyBase(const Container *container, index_t &&index)        // create a read only element proxy, and moves the provided "index"
         : m_container(container), m_index(index)
      {}
      ElementProxyBase(Container *container, index_t &&index)              // create a read-write element proxy, and moves the provided "index"
         : m_container(container), m_index(index)
      {}
      ElementProxyBase(const ElementProxyBase<ptrAccessPolicy> &other)     // create a read-write element proxy, and moves the provided "index"
         : m_container(other.m_container), m_index(other.m_index)
      {}
      ElementProxyBase(const ElementProxyBase<AccessPolicy::ReadWrite> &other)   // create a read-write element proxy, and moves the provided "index"
          requires(ptrAccessPolicy == AccessPolicy::ReadOnly)
        : m_container(other.container().cptr()), m_index(other.m_index)
       {}

      template <typename T_RWProxy>
      ElementProxyBase(const T_RWProxy &other)   // create a read-write element proxy, and moves the provided "index"
         requires(ptrAccessPolicy == AccessPolicy::ReadOnly && isConvertableToReadOnlyProxy<ElementProxyBase<ptrAccessPolicy>, T_RWProxy> )
      : m_container(&other.container()), m_index(other.index())
      {}

      /// @param Return the  "index" which identifies the element this proxy refers to within
      ///        the container returned by container.
      /// This may or may not be the index which was used to create this proxy from
      /// the parent proxy i.e. in
      /// auto child_proxy=parent_proxy[index]
      /// it may be that child_proxy.index() != index.
      /// To recover "index" use auto index_recover = ParentProxy::getOriginalElementIndex(child_proxy);
      index_t index() const { return m_index; }

      /// @return A const pointer of the container which contains the elements this proxy refers to.
      /// The value returned by index identifies the element within this container.
      const Container &container() const                                                    { return m_container.container(); }
      /// @return A non const pointer of the container which contains the elements this proxy refers to
      /// if the access policy permits read-write access
      Container &container()             requires(ptrAccessPolicy==AccessPolicy::ReadWrite) { return m_container.container(); }
   };

   /// @brief return a const pointer of the container which contains the elements this proxy refers to.
   const Container &container() const                                                    { return m_container.container(); }
   /// @brief Return a non const pointer of the container which contains the elements this proxy refers to
   ///        provided the access policy permits read-write access.
   Container &container()             requires(accessPolicy==AccessPolicy::ReadWrite) { return m_container.container(); }
};

/// @brief The proxy container object which provides the means to iterate over its elements and create element proxy objects
/// @tparm Container     The container type this proxy refers to e.g. CellData
/// @tparm  T_Derived    The derived class of this proxy container class, which may have to define
///                      static methods to enable the iteration:
///                      element_index_t beginIndex(const container *, IndexType this_container_index);
///                      element_index_t endIndex(const container *, IndexType this_container_index);
///                      element_index_t nextIndex(const container *, element_index_t element_index);
/// @tparam ElementProxy The class to be used for element proxy objects, which can be a derived class of
///                      another ContainerProxy, or an ElementProxyBase
/// @tparam IndexType    The index type which is used to identify the element this container proxy refers to
///                      to if this container proxy is en element of a parent container proxy. The IndexType
///                      has to implement at least the equality operator. In most cases also
///                      a prefix increment operator is needed, which could also be implemented
///                      in the static method nextElementIndex of the parent container proxy.
/// @tparam accessPolicy To choose read-only or read-write access (read-only is default)
template <class Container,
          class T_Derived,
          class ElementProxy,
          typename IndexType,
          AccessPolicy accessPolicy>
struct ContainerProxy : ContainerProxyBase<Container, typename ElementProxy::index_t, accessPolicy> {
   using value_type = ElementProxy;
   using index_t = IndexType;
   using element_index_t = typename ElementProxy::index_t;
   index_t m_index;

   // the base class
   using BASE = ContainerProxyBase<Container, typename ElementProxy::index_t, accessPolicy>;

   /// @brief Create a root container proxy with read-write access to its elements
   ///        where a root container proxy refers to the top node of a container hierarchy
   ContainerProxy(Container *container)
      requires ( std::is_same_v<index_t,RootNodeIndex> && hasSize<Container> )
   : BASE(container), m_index() {}

   /// @brief Create a root container proxy with read-only access to its elements which is only possible
   ///        if the accessPolicy of this proxy container class permits read-write access.
   ContainerProxy(const Container *container)
      requires ( std::is_same_v<index_t,RootNodeIndex> && hasSize<Container> )
      : BASE(container), m_index() {}

   /// @brief Create a sub-container proxy with read-write access to its elements
   ///        where a sub-container proxy is a container proxy which is an element of a parent
   ///        container proxy.
   ContainerProxy(Container *container, const index_t &index) : BASE(container), m_index(index) {}
   /// @brief Create a sub-container proxy with read-only access to its elements which is only possible
   ///        if the accessPolicy of this proxy container class permits read-write access.
   ContainerProxy(const Container *container, const index_t &index) : BASE(container), m_index(index) {}

   /// @brief  Create a read only element proxy from a read write proxy
   template <typename T_RWProxy>
   ContainerProxy(const T_RWProxy &other)
      requires(accessPolicy == AccessPolicy::ReadOnly
               && isConvertableToReadOnlyProxy<ContainerProxy<Container, T_Derived, ElementProxy, IndexType, accessPolicy>, T_RWProxy> )
   : BASE(&other.container()), m_index(other.index())
   {}

   /// @brief Create a proxy for one element of the "container" this proxy represents (read-only access).
   /// @param ptr A pointer to the container which contains the element data
   /// @param element_index An index which identifies the element in the given container.
   /// @return A new proxy which represents the specified element.
   static auto createElementProxy(const Container *ptr, element_index_t &&element_index) {
      if constexpr(hasCreateProxy<ElementProxy, decltype(ptr), element_index_t> ) {
         return ElementProxy::create(ptr, std::move(element_index));
      }
      else {
         return ElementProxy(ptr, std::move(element_index));
      }
   }

   /// @brief Create a proxy for one element of the "container" this proxy represents (read-write access).
   /// @param ptr A pointer to the container which contains the element data
   /// @param element_index An index which identifies the element in the given container.
   /// @return A new proxy which represents the specified element.
   static auto createElementProxy(Container *ptr, element_index_t &&element_index) requires (accessPolicy == AccessPolicy::ReadWrite) {
      if constexpr(hasCreateProxy<ElementProxy, decltype(ptr), element_index_t> ) {
         return ElementProxy::create(ptr, std::move(element_index));
      }
      else {
         return ElementProxy(ptr, element_index);
      }
   }

   // base class of proxy objects referring to elements of this container proxy
   template <AccessPolicy elementAccessPolicy>
   using ElementProxyBase = typename  BASE::ElementProxyBase<elementAccessPolicy>;

   /// @brief Base class of iterators to iterate over the elements of this proxy container
   template <AccessPolicy iteratorAccessPolicy>
   struct iterator_base : ElementProxyBase<iteratorAccessPolicy> {
      using ElementProxyBase<iteratorAccessPolicy>::ElementProxyBase;

      // Go to the next element of this container proxy.
      iterator_base &operator++() {
         this->m_index = T_Derived::nextElementIndex(this->m_container.cptr(), std::move(this->m_index));
         return *this;
      }

      // Create a proxy object to access the element this iterator refers to.
      auto operator*()
      {
         return createElementProxy(this->m_container.template accessPtr<iteratorAccessPolicy>(),
                                   typename ElementProxyBase<iteratorAccessPolicy>::index_t(this->m_index));
      }

      // Test whether two iterators of this proxy container refer to the same element.
      // The result is undefined if the iterators refer to different containers.
      template<AccessPolicy otherIteratorAccessPolicy>
      bool operator==(const iterator_base<otherIteratorAccessPolicy> &other) const {
         assert ( this->m_container == other.m_container);
         return this->m_index == other.m_index;
      }
   };

   // the iterator class to iterator over elements of this proxy container with read write element access
   using iterator = iterator_base<AccessPolicy::ReadWrite>;
   // the iterator class to iterator over elements of this proxy container with read only element access
   using const_iterator = iterator_base<AccessPolicy::ReadOnly>;

   /// @brief The index of this proxy container which identifies this proxy container within the parent container.
   /// For the top-level proxy container the index will be the empty RootNodeIndex struct. The index is not
   /// necessarily an index that can be used as argument for the element access operator of the parent proxy
   /// to recover this proxy i.e. parent_proxy[index] is not necessarily this proxy. For the latter
   /// use @ref computeChildElementIndex.
   const index_t &index() const { return m_index; }

   /// @brief Compute the "index" of the given element which can be used to recover this element via the access operator [].
   element_index_t computeChildElementIndex(const ElementProxy &element_proxy) const
      requires (   !hasCreateProxy<ElementProxy, const Container *, element_index_t>
                && !hasCreateProxy<ElementProxy, Container *, element_index_t>)
   {
      return element_proxy.index() - T_Derived::beginIndex(this->m_container.cptr(), m_index);
   }

   /// @brief compute the "index" of the given element which can be used to recover this element via the access operator [].
   template <typename T_ElementProxy>
   element_index_t computeChildElementIndex(const T_ElementProxy &element_proxy) const
      requires (   hasCreateProxy<ElementProxy, const Container *, element_index_t>
                || hasCreateProxy<ElementProxy, Container *, element_index_t>)
   {
      return ElementProxy::getOriginalElementIndex(element_proxy.index())  - T_Derived::beginIndex(this->m_container.cptr(), m_index);
   }

   /// @brief Get the begin iterator of this proxy container for read write element access provided the access policy permits it.
   iterator begin() requires ( accessPolicy == AccessPolicy::ReadWrite ) {
      return iterator(this->m_container.ptr(), T_Derived::beginIndex(this->m_container.cptr(), m_index) );
   }
   /// @brief Get the end iterator of this proxy container for read write element access provided the access policy permits it.
   iterator end() requires ( accessPolicy == AccessPolicy::ReadWrite ) {
      return iterator(this->m_container.ptr(), T_Derived::endIndex(this->m_container.cptr(), m_index) );
   }

   /// @brief Get the begin iterator of this proxy container for read only element access.
   const_iterator begin() const {
      return const_iterator(this->m_container.cptr(), T_Derived::beginIndex(this->m_container.cptr(), m_index) );
   }
   /// @brief Get the end iterator of this proxy container for read only element access.
   const_iterator end() const {
      return const_iterator(this->m_container.cptr(), T_Derived::endIndex(this->m_container.cptr(), m_index) );
   }

   /// @brief Element access operator (read-only access)
   /// @param element_count is the "child_index", a consecutive number which is 0 for the first element and size()-1 for the last element
   /// @return Proxy representing the specified element
   auto operator[](std::size_t element_count) const
   { assert( element_count < size() );
     return createElementProxy(this->m_container.cptr(), T_Derived::elementIndexAt(this->m_container.cptr(), this->m_index,element_count)); }
   /// @brief Element access operator (read-write access)
   /// @param element_count is the "child_index", a consecutive number which is 0 for the first element and size()-1 for the last element
   /// @return Proxy representing the specified element
   auto operator[](std::size_t element_count) requires (accessPolicy == AccessPolicy::ReadWrite)
   { assert( element_count < size() );
     return createElementProxy(this->m_container.ptr(), T_Derived::elementIndexAt(this->m_container.cptr(), this->m_index,element_count));}

   /// @brief Get a proxy for the first child element (read-only)
   /// The operation is undefined if there are no child elements.
   auto front() const
   { assert( !empty() );
     return createElementProxy(this->m_container.cptr(), T_Derived::beginIndex(this->m_container.cptr(), this->m_index));}
   /// @brief Get a proxy for the first child element (read-write).
   /// The operation is undefined if there are no child elements.
   auto front() requires (accessPolicy == AccessPolicy::ReadWrite)
   { assert( !empty() );
     return createElementProxy(this->m_container.ptr(), T_Derived::beginIndex(this->m_container.cptr(), this->m_index));}

   /// @brief Get a proxy for the last child element (read-only).
   /// The operation is undefined if there are no child elements.
   auto back() const
   { assert(this->size() > 0);
     std::size_t element_count = this->size()-1;
     return createElementProxy(this->m_container.cptr(), T_Derived::elementIndexAt(this->m_container.cptr(), this->m_index,element_count));
   }
   /// @brief Get a proxy for the last child element (read-write).
   /// The operation is undefined if there are no child elements.
   auto back() requires (accessPolicy == AccessPolicy::ReadWrite)
   { assert(this->size() > 0);
     std::size_t element_count = this->size()-1;
     return createElementProxy(this->m_container.ptr(), T_Derived::elementIndexAt(this->m_container.ptr(), this->m_index,element_count));
   }

   /// @brief Default implementation to get the index of the first element of this proxy container
   /// For a full range container proxy e.g. the root proxy container.
   /// this requires that the first element this proxy container refers to is identified by a simple
   /// integer index of 0u
   static element_index_t beginIndex([[maybe_unused]] const Container *container, [[maybe_unused]] const index_t &this_index)
      requires (std::is_same_v<index_t,RootNodeIndex> && std::is_convertible_v<std::size_t, element_index_t> && hasSize<Container> )
   {
      return static_cast<element_index_t>(0u);
   }
   /// @brief Default implementation to get the index after the last element of this proxy container
   /// For a full range container proxy e.g. the root proxy container.
   /// this requires that the last element this proxy container refers to is identified by a simple
   /// integer index which is provided by the size method of the container this proxy container refers to.
   static element_index_t endIndex([[maybe_unused]] const Container *container, [[maybe_unused]] const index_t &this_index)
      requires (std::is_same_v<index_t,RootNodeIndex> && std::is_convertible_v<std::size_t, element_index_t> && hasSize<Container>)
   {
      assert( !container || container->size() < std::numeric_limits<element_index_t>::max());
      return (container ? static_cast<element_index_t>(container->size()) : 0u);
   }
   // @brief Default implementation to get the next index following the element identified by the given element index.
   // In most cases the next element is computed by the prefix increment operator
   static element_index_t nextElementIndex([[maybe_unused]] const Container *container, element_index_t &&element_index)
   {
      assert( container );
      return ++element_index;
   }

   /// @brief Default implementation to get the full index of a certain element
   /// @param container The container which contains the element data,
   /// @param this_index The index of this proxy.
   /// @param element_counter the number of iterations from the first element to reach this element.
   /// @return the full index of the specified element which fully identifies this element in the given container.
   static element_index_t elementIndexAt(const Container *container, const index_t &this_index, std::size_t element_counter)
      requires (hasAddition<element_index_t> )
   {  return T_Derived::beginIndex(container,this_index) + element_counter; }


   /// @brief Default implementation to compute the number of elements this proxy container contains/refers to
   /// The default implementation simply computes the differences between the end index and begin index, thus
   /// the IndexType has to implement the subtraction operator.
   std::size_t size() const
      requires (hasDifference<element_index_t>)
   {
      return (this->m_container.cptr()
              ? T_Derived::endIndex(this->m_container.cptr(), m_index)-T_Derived::beginIndex(this->m_container.cptr(), m_index)
              : 0u);
   }

   /// @brief Default implementation to test whether the container does not contain elements.
   /// The default implementation simply tests whether the begin and end index are identical.
   std::size_t empty() const
   {
      return (this->m_container.cptr()
              ? T_Derived::endIndex(this->m_container.cptr(), m_index)==T_Derived::beginIndex(this->m_container.cptr(), m_index)
              : true);
   }

};
}
#endif
