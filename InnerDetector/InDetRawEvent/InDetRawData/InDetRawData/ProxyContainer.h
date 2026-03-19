/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
  */
#ifndef UTILS_PROXY_CONTAINER_H
#define UTILS_PROXY_CONTAINER_H

#include <cassert>
#include <utility>
#include <type_traits>

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

/// @brief Indicate whether a container provides read only or read write access
enum class AccessPolicy {
   Const,
   Mutable
};

template <bool isConst>
struct AccessPolicyHelper;

template<>
struct AccessPolicyHelper<true> {
   static constexpr AccessPolicy accessPolicy = AccessPolicy::Const;
};

template<>
struct AccessPolicyHelper<false> {
   static constexpr AccessPolicy accessPolicy = AccessPolicy::Mutable;
};

template <typename Container,  AccessPolicy accessPolicy>
struct ContainerAccessHelper {
   using ContainerNonConst = std::remove_cvref_t<Container>;
   using ContainerPtr = std::conditional<accessPolicy==AccessPolicy::Const , const ContainerNonConst *, ContainerNonConst *>::type;
   using ContainerType = std::conditional<accessPolicy==AccessPolicy::Const , const ContainerNonConst, ContainerNonConst>::type;
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
          typename RangeType=RootNodeIndex>
struct ContainerProxy;


// @brief Possible base class for an element proxy.
// This base class is used by the proxy container iterators
template <class Container,
          typename ElementIndexType>
struct ElementProxyBase {
   static constexpr bool isConst = std::is_const_v<Container>;
   using index_t = ElementIndexType;
   using ContainerNonConst = std::remove_cvref_t<Container>;
   using ContainerPtr = std::conditional<isConst , const ContainerNonConst *, ContainerNonConst *>::type;

public:
   ElementProxyBase(const ContainerNonConst *container, const  index_t &index)  // create a read-only element proxy
      requires(isConst)
      : m_container(container), m_index(index)
   {}
   ElementProxyBase(ContainerNonConst *container, const index_t &index)         // create a read-write element proxy
      : m_container(container), m_index(index)
   {}
   ElementProxyBase(const ContainerNonConst *container, index_t &&index)        // create a read only element proxy, and moves the provided "index"
      requires(isConst)
      : m_container(container), m_index(index)
   {}
   ElementProxyBase(ContainerNonConst *container, index_t &&index)              // create a read-write element proxy, and moves the provided "index"
      : m_container(container), m_index(index)
   {}
   template <class OtherContainer>
   ElementProxyBase(const ElementProxyBase<OtherContainer, ElementIndexType> &other)// create a read-write element proxy, and moves the provided "index"
      requires(std::is_same_v<ContainerNonConst, typename OtherContainer::ContainerNonConst>
               && (!OtherContainer::isConst || isConst) )
      : m_container(other.m_container), m_index(other.m_index)
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
   const ContainerNonConst &container() const
   { assert( m_container != nullptr); return *m_container; }
   /// @return A non const pointer of the container which contains the elements this proxy refers to
   /// if the access policy permits read-write access
   ContainerNonConst &container()             requires(!isConst)
   { assert( m_container != nullptr); return *m_container; }
protected:
   const ContainerNonConst *cptr() const {
      return &container();
   }
   ContainerPtr m_container; // refers to the original container the parent proxy refers to
   index_t m_index;                               // an "index" which indicates the element this proxy refers to
};

/// @brief Base class of a container proxy
/// @tparam Container the container type this proxy refers to e.g. CellData
/// @tparam ElementIndexType the index type to refer to  an element of the container proxy e.g. unsigned int,
///                    but it could be an object with associated data e.g. an index to the parent
///                    element or on index offset for its child elements
/// The base class provides an access controlled pointer to the container this proxy is referring to.
template <class Container,
          typename ElementIndexType>
struct ContainerProxyBase {
   static constexpr bool isConst = std::is_const_v<Container>;
   using element_index_t = ElementIndexType;
   using ContainerNonConst = std::remove_cvref_t<Container>;
   using ContainerPtr = std::conditional<isConst , const ContainerNonConst *, ContainerNonConst *>::type;

   template <class OtherContainer>
   ContainerProxyBase(const ContainerProxyBase<OtherContainer,ElementIndexType> &a)
      requires(std::is_same_v<ContainerNonConst, typename OtherContainer::ContainerNonConst> || !OtherContainer::isConst)
   : m_container(a.m_container) {}
   ContainerProxyBase(const ContainerNonConst *container) requires(isConst)
     : m_container(container) {}  // creates a proxy which provides read-only access to its elements
   ContainerProxyBase(ContainerNonConst *container)
     : m_container(container) {}  // creates a proxy which provides read-write access to its elements

   /// @brief return a const pointer of the container which contains the elements this proxy refers to.
   const ContainerNonConst &container() const
   { assert(m_container != nullptr); return *m_container; }
   /// @brief Return a non const pointer of the container which contains the elements this proxy refers to
   ///        provided the access policy permits read-write access.
   ContainerNonConst &container()             requires(!isConst)
   { assert( m_container != nullptr); return *m_container; }

protected:
   const ContainerNonConst *cptr() const {
      return m_container;
   }
   ContainerNonConst *ptr() requires(!isConst) {
      return m_container;
   }

   ContainerPtr m_container;                         // pointer to the container this proxy refers to
};

/// @brief Base class of iterators to iterate over the elements of this proxy container
template <class Container,
          class T_Derived,
          class ElementProxy>
struct ProxyIteratorBase : ElementProxyBase<Container, typename ElementProxy::index_t> {
   using element_index_t = typename ElementProxy::index_t;
   using BASE=ElementProxyBase<Container, element_index_t>;
   using BASE::BASE;

   // Go to the next element of this container proxy.
   ProxyIteratorBase &operator++() {
      this->m_index = T_Derived::nextElementIndex(this->cptr(), std::move(this->m_index));
      return *this;
   }
   ProxyIteratorBase &operator+=(std::size_t offset) requires(hasAddition<element_index_t>) {
      this->m_index = this->m_index + offset;
      return *this;
   }

   // Create a proxy object to access the element this iterator refers to.
   auto operator*()
   {
      return this->createElementProxy(this->m_container,
                                      typename BASE::index_t(this->m_index));
   }

   // Test whether two iterators of this proxy container refer to the same element.
   // The result is undefined if the iterators refer to different containers.
   template<class OtherContainer>
   bool operator==(const ProxyIteratorBase<OtherContainer, T_Derived, ElementProxy> &other) const
      requires(std::is_same_v<typename BASE::ContainerNonConst,
                              typename ProxyIteratorBase<OtherContainer, T_Derived, ElementProxy>::ContainerNonConst>)
   {
      assert ( this->m_container == other.m_container);
      return this->m_index == other.m_index;
   }

   /// @brief Create a proxy for one element of the "container" this proxy represents (read-only access).
   /// @param ptr A pointer to the container which contains the element data
   /// @param element_index An index which identifies the element in the given container.
   /// @return A new proxy which represents the specified element.
   /// This method will be called if the element proxy provides a create method.
   /// @note to create a const element proxy from a non-const element proxy requires that the element
   ///   proxy provides a create method which takes a const contaner as argument.
   static auto createElementProxy(const BASE::ContainerNonConst *ptr, element_index_t &&element_index)
      requires(hasCreateProxy<ElementProxy, decltype(ptr), element_index_t> )
   {
      return ElementProxy::create(ptr,std::move(element_index));
   }
   /// @brief Create a proxy for one element of the "container" this proxy represents (read-only access).
   /// @param ptr A pointer to the container which contains the element data
   /// @param element_index An index which identifies the element in the given container.
   /// @return A new proxy which represents the specified element.
   /// This method will be called if the element proxy does not provide a create method.
   /// In this case the const-ness of the element proxy will be inherited from the parent
   /// proxy.
   static auto createElementProxy(const BASE::ContainerNonConst *ptr, element_index_t &&element_index)
      requires(ElementProxy::isConst && !hasCreateProxy<ElementProxy, decltype(ptr), element_index_t> )
   {
      return ElementProxy(ptr,std::move(element_index));
   }

   /// @brief Create a proxy for one element of the "container" this proxy represents (read-write access).
   /// @param ptr A pointer to the container which contains the element data
   /// @param element_index An index which identifies the element in the given container.
   /// @return A new proxy which represents the specified element.
   static auto createElementProxy(BASE::ContainerNonConst *ptr, element_index_t &&element_index)
      requires (!BASE::isConst) {
      if constexpr(hasCreateProxy<ElementProxy, decltype(ptr), element_index_t> ) {
         return ElementProxy::create(ptr, std::move(element_index));
      }
      else {
         return ElementProxy(ptr, std::move(element_index));
      }
   }

};

// extension of the ProxyIteratorBase to make it usable in std::ranges::subrange
template <class Container,
          class T_Derived,
          class ElementProxy>
struct ProxyIterator : ProxyIteratorBase<Container,T_Derived, ElementProxy> {
public:
   using BASE=ProxyIteratorBase<Container,T_Derived, ElementProxy>;
   using BASE::BASE;

   // default constructor needed to use these iterators in a std::range::subrange
   ProxyIterator() : BASE(nullptr,typename BASE::element_index_t{}) {}

   ProxyIterator &operator++() {
      (void) BASE::operator++();
      return *this;
   }
   ProxyIterator &operator+=(std::size_t offset) requires(hasAddition<typename BASE::element_index_t>) {
      (void) BASE::operator+=(offset);
      return *this;
   }

   // also define post-fix iterator to allow these iterators to be used to create a std::ranges::subrange
   ProxyIterator operator++(int) {
      ProxyIterator clone(*this);
      this->m_index = T_Derived::nextElementIndex(this->cptr(), std::move(this->m_index));
      return clone;
   }

   template<class OtherContainer>
   bool operator!=(const ProxyIteratorBase<OtherContainer, T_Derived, ElementProxy> &other) const
      requires(std::is_same_v<typename BASE::ContainerNonConst, typename ProxyIteratorBase<OtherContainer, T_Derived, ElementProxy>::ContainerNonConst>)
   {
      return !(this->operator==(other));
   }

   // the type resulting from dereferencing the iterator
   using value_type = decltype( std::declval<BASE>().operator *());
private:
   // helper class to define the type representing the differences of two iterators
   // e.g. used to compute the size of std::ranges::subrange
   template <typename T>
   struct diff_type_helper {
      // for integral types assume that a 64 bit signed integer is a good choice
      static std::int64_t diff(const T &a, const T &b)
         requires(std::is_integral_v<T>)
      {return a-b;}

      // if a difference operation is defined for the index then use the return type
      static auto  diff(const T &a, const T &b)
         requires(!std::is_integral_v<T> && hasDifference<T>)
      {return a-b;}

      // otherwise use a type which is not a valid difference type what concerns std::ranges::subrange
      static void  diff([[maybe_unused]] const T &a, [[maybe_unused]] const T &b)
         requires(!std::is_integral_v<T> && !hasDifference<T>)
      { }
   };
public:
   using difference_type = decltype( diff_type_helper<typename BASE::element_index_t>::diff(std::declval<typename BASE::element_index_t>(),
                                                                                            std::declval<typename BASE::element_index_t>()) );
};


/// @brief The proxy container object which provides the means to iterate over its elements and create element proxy objects
/// @tparm Container     The container type this proxy refers to e.g. CellData, the proxy will not allow modifications if the contains is const.
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
template <class Container,
          class T_Derived,
          class ElementProxy,
          typename IndexType>
struct ContainerProxy : ContainerProxyBase<Container, typename ElementProxy::index_t> {
   // the base class
   using BASE = ContainerProxyBase<Container, typename ElementProxy::index_t>;
   using ContainerNonConst = BASE::ContainerNonConst;
   using value_type = ElementProxy;
   using index_t = IndexType;
   using element_index_t = typename ElementProxy::index_t;
   index_t m_index;

   /// @brief Create a root container proxy with read-write access to its elements
   ///        where a root container proxy refers to the top node of a container hierarchy
   ContainerProxy(ContainerNonConst *container)
      requires ( std::is_same_v<index_t,RootNodeIndex> && hasSize<Container> )
   : BASE(container), m_index() {}

   /// @brief Create a root container proxy with read-only access to its elements which is only possible
   ///        if the container is not const  this proxy permits non-const access to the container
   ContainerProxy(const ContainerNonConst *container)
      requires ( std::is_same_v<index_t,RootNodeIndex> && hasSize<Container> )
      : BASE(container), m_index() {}

   /// @brief Create a sub-container proxy with read-write access to its elements
   ///        where a sub-container proxy is a container proxy which is an element of a parent
   ///        container proxy.
   ContainerProxy(ContainerNonConst *container, const index_t &index) : BASE(container), m_index(index) {}
   /// @brief Create a sub-container proxy with read-only access to its elements which is only possible
   ///        if the container is const, this proxy will only allow const access to the container.
   ContainerProxy(const ContainerNonConst *container, const index_t &index) : BASE(container), m_index(index) {}

   /// @brief  Create a read only element proxy from a read write proxy
   template <typename T_RWProxy>
   ContainerProxy(const T_RWProxy &other)
      requires(BASE::isConst
               && isConvertableToReadOnlyProxy<ContainerProxy<Container, T_Derived, ElementProxy, IndexType>, T_RWProxy> )
   : BASE(&other.container()), m_index(other.index())
   {}




   // the iterator class to iterator over elements of this proxy container with read write element access
   using iterator = ProxyIterator<ContainerNonConst,T_Derived, ElementProxy>;
   // the iterator class to iterator over elements of this proxy container with read only element access
   using const_iterator = ProxyIterator<const ContainerNonConst,T_Derived, ElementProxy>;

   /// @brief The index of this proxy container which identifies this proxy container within the parent container.
   /// For the top-level proxy container the index will be the empty RootNodeIndex struct. The index is not
   /// necessarily an index that can be used as argument for the element access operator of the parent proxy
   /// to recover this proxy i.e. parent_proxy[index] is not necessarily this proxy. For the latter
   /// use @ref computeChildElementIndex.
   const index_t &index() const { return m_index; }

   /// @brief Compute the "index" of the given element which can be used to recover this element via the access operator [].
   element_index_t computeChildElementIndex(const ElementProxy &element_proxy) const
      requires (   !hasCreateProxy<ElementProxy, const ContainerNonConst *, element_index_t>
                && !hasCreateProxy<ElementProxy, ContainerNonConst *, element_index_t>)
   {
      return element_proxy.index() - T_Derived::beginIndex(this->cptr(), m_index);
   }

   /// @brief compute the "index" of the given element which can be used to recover this element via the access operator [].
   template <typename T_ElementProxy>
   element_index_t computeChildElementIndex(const T_ElementProxy &element_proxy) const
      requires (   hasCreateProxy<ElementProxy, const ContainerNonConst *, element_index_t>
                || hasCreateProxy<ElementProxy, ContainerNonConst *, element_index_t>)
   {
      return ElementProxy::getOriginalElementIndex(element_proxy.index())  - T_Derived::beginIndex(this->cptr(), m_index);
   }

   /// @brief Get the begin iterator of this proxy container for read write element access provided the access policy permits it.
   iterator begin() requires ( !BASE::isConst ) {
      return iterator(this->ptr(), T_Derived::beginIndex(this->cptr(), m_index) );
   }
   /// @brief Get the end iterator of this proxy container for read write element access provided the access policy permits it.
   iterator end() requires ( !BASE::isConst ) {
      return iterator(this->ptr(), T_Derived::endIndex(this->cptr(), m_index) );
   }

   /// @brief Get the begin iterator of this proxy container for read only element access.
   const_iterator begin() const {
      return const_iterator(this->cptr(), T_Derived::beginIndex(this->cptr(), m_index) );
   }
   /// @brief Get the end iterator of this proxy container for read only element access.
   const_iterator end() const {
      return const_iterator(this->cptr(), T_Derived::endIndex(this->cptr(), m_index) );
   }

   /// @brief Element access operator (read-only access)
   /// @param element_count is the "child_index", a consecutive number which is 0 for the first element and size()-1 for the last element
   /// @return Proxy representing the specified element
   auto operator[](std::size_t element_count) const
   { assert( element_count < size() );
     return const_iterator::createElementProxy(this->cptr(), T_Derived::elementIndexAt(this->cptr(), this->m_index,element_count)); }
   /// @brief Element access operator (read-write access)
   /// @param element_count is the "child_index", a consecutive number which is 0 for the first element and size()-1 for the last element
   /// @return Proxy representing the specified element
   auto operator[](std::size_t element_count) requires (!BASE::isConst)
   { assert( element_count < size() );
      return iterator::createElementProxy(this->ptr(), T_Derived::elementIndexAt(this->cptr(), this->m_index,element_count));}

   /// @brief Get a proxy for the first child element (read-only)
   /// The operation is undefined if there are no child elements.
   auto front() const
   { assert( !empty() );
     return const_iterator::createElementProxy(this->cptr(), T_Derived::beginIndex(this->cptr(), this->m_index));}
   /// @brief Get a proxy for the first child element (read-write).
   /// The operation is undefined if there are no child elements.
   auto front() requires (!BASE::isConst)
   { assert( !empty() );
     return iterator::createElementProxy(this->ptr(), T_Derived::beginIndex(this->cptr(), this->m_index));}

   /// @brief Get a proxy for the last child element (read-only).
   /// The operation is undefined if there are no child elements.
   auto back() const
   { assert(this->size() > 0);
     std::size_t element_count = this->size()-1;
     return const_iterator::createElementProxy(this->cptr(), T_Derived::elementIndexAt(this->cptr(), this->m_index,element_count));
   }
   /// @brief Get a proxy for the last child element (read-write).
   /// The operation is undefined if there are no child elements.
   auto back() requires (!BASE::isConst)
   { assert(this->size() > 0);
     //coverity[INTEGER_OVERFLOW]
     std::size_t element_count = this->size()-1;
     return iterator::createElementProxy(this->ptr(), T_Derived::elementIndexAt(this->ptr(), this->m_index,element_count));
   }

   /// @brief Default implementation to get the index of the first element of this proxy container
   /// For a full range container proxy e.g. the root proxy container.
   /// this requires that the first element this proxy container refers to is identified by a simple
   /// integer index of 0u
   static element_index_t beginIndex([[maybe_unused]] const ContainerNonConst *container, [[maybe_unused]] const index_t &this_index)
      requires (std::is_same_v<index_t,RootNodeIndex> && std::is_convertible_v<std::size_t, element_index_t> && hasSize<ContainerNonConst> )
   {
      return static_cast<element_index_t>(0u);
   }
   /// @brief Default implementation to get the index after the last element of this proxy container
   /// For a full range container proxy e.g. the root proxy container.
   /// this requires that the last element this proxy container refers to is identified by a simple
   /// integer index which is provided by the size method of the container this proxy container refers to.
   static element_index_t endIndex([[maybe_unused]] const ContainerNonConst *container, [[maybe_unused]] const index_t &this_index)
      requires (std::is_same_v<index_t,RootNodeIndex> && std::is_convertible_v<std::size_t, element_index_t> && hasSize<Container>)
   {
      assert( !container || container->size() < std::numeric_limits<element_index_t>::max());
      return (container ? static_cast<element_index_t>(container->size()) : 0u);
   }
   // @brief Default implementation to get the next index following the element identified by the given element index.
   // In most cases the next element is computed by the prefix increment operator
   static element_index_t nextElementIndex([[maybe_unused]] const ContainerNonConst *container, element_index_t &&element_index)
   {
      assert( container );
      return ++element_index;
   }

   /// @brief Default implementation to get the full index of a certain element
   /// @param container The container which contains the element data,
   /// @param this_index The index of this proxy.
   /// @param element_counter the number of iterations from the first element to reach this element.
   /// @return the full index of the specified element which fully identifies this element in the given container.
   static element_index_t elementIndexAt(const ContainerNonConst *container, const index_t &this_index, std::size_t element_counter)
      requires (hasAddition<element_index_t> )
   {  return T_Derived::beginIndex(container,this_index) + element_counter; }

   /// @brief Default implementation to compute the number of elements this proxy container contains/refers to
   /// The default implementation simply computes the differences between the end index and begin index, thus
   /// the IndexType has to implement the subtraction operator.
   std::size_t size() const
      requires (hasDifference<element_index_t>)
   {
      return (this->cptr()
              ? T_Derived::endIndex(this->cptr(), m_index)-T_Derived::beginIndex(this->cptr(), m_index)
              : 0u);
   }

   /// @brief Default implementation to test whether the container does not contain elements.
   /// The default implementation simply tests whether the begin and end index are identical.
   std::size_t empty() const
   {
      return (this->cptr()
              ? T_Derived::endIndex(this->cptr(), m_index)==T_Derived::beginIndex(this->cptr(), m_index)
              : true);
   }
};
}
#endif
