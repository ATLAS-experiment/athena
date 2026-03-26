/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
  */
#ifndef PHASEII_RAWDATACONTAINER_H
#define PHASEII_RAWDATACONTAINER_H

#include <array>
#include <vector>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <atomic>

#include "ProxyContainer.h"
#include "ContainerList.h"

namespace PhaseII {

   template <typename Type>
   concept isAtomic = requires( Type a, typename Type::value_type b ) { a.compare_exchange_weak(b,b); };

   // test whether the given container type has an error container
   template <typename T_DataContainer>
   concept hasErrors = requires(T_DataContainer &a, unsigned int idx) { a.errors()[idx]; };

   template <typename T>
   struct GetValue {
      static auto value(const T &value) requires ( !isAtomic<T> ) { return value; }
      static auto value(const T &value) requires (  isAtomic<T> ) { return value.load(); }
   };

   /// @brief Type describing a range of elements living in a one of N containers.
   ///
   /// The types of the index of the first element, number of elements, and container index
   /// are chosen such that the resulting struct does not exceed 64 bits to allow for an atomic
   /// update of a range.
   /// @TODO make members private ?
   struct  DataRange {
      using RangeBeginIndex_t = std::uint32_t;
      using RangeSize_t = std::uint16_t;
      using ContainerIndex_t = std::uint16_t;
      // wrap members of the DataRange into union to convince clang to
      // use this type as a value for a lock free atomic.
      union {
         struct Range {
            RangeBeginIndex_t m_beginIndex{};    ///< the index of the first element in the range in the container defined by the container index
            RangeSize_t m_n{};                   ///< the number of elements in this range.
            ContainerIndex_t m_containerIndex{}; ///< the index which identifies the the container within a container collection
         } m_range;
         std::uint64_t m_compactRange;
      } m_payload;

      DataRange() : m_payload{.m_compactRange = std::uint64_t{} }{}
      DataRange(std::uint64_t compact_range) : m_payload{.m_compactRange = compact_range }{}
      DataRange(unsigned int  begin_val, unsigned int n, unsigned int idx)
         : m_payload{ .m_range = {static_cast<std::uint32_t>(begin_val), static_cast<std::uint16_t>(n), static_cast<std::uint16_t>(idx)} }
      {
         assert(begin_val<std::numeric_limits<std::uint32_t>::max());
         assert(n<std::numeric_limits<std::uint16_t>::max());
         assert(idx<std::numeric_limits<std::uint16_t>::max());
      }
      DataRange(std::uint32_t begin_val, std::uint16_t  n, std::uint16_t idx)
         : m_payload{ .m_range= {begin_val, n, idx} }
      {
      }
      std::uint64_t makeCompact() const {
         static_assert( sizeof(m_payload.m_range) == sizeof(std::uint64_t));
         return m_payload.m_compactRange;
      }

      void setSize(RangeSize_t new_size) { m_payload.m_range.m_n=new_size; }
      std::uint16_t containerIndex() const { return m_payload.m_range.m_containerIndex; }
      unsigned int beginIndex() const { return m_payload.m_range.m_beginIndex;}
      unsigned int endIndex() const { return m_payload.m_range.m_beginIndex + m_payload.m_range.m_n; }
      std::uint16_t size() const { return m_payload.m_range.m_n; }
      bool empty() const { return m_payload.m_range.m_n==0u; }
      template <typename T_ElementIndex, typename T_ContainerIndex>
      static DataRange makeDataRange(T_ElementIndex begin_index,
                                     T_ElementIndex end_index,
                                     T_ContainerIndex container_index) {
         // ensure that the inputs are within the allowed range in (debug build only!)
         // @TODO check some of these always during runtime ?
         //       Replace this convenience method into a constructor ? But an exception could be thrown here.
         assert( end_index >= begin_index ) ;
         assert( static_cast<std::uint32_t>(begin_index) == begin_index);
         assert( static_cast<std::size_t>(end_index -  begin_index) < std::numeric_limits<std::uint16_t>::max());
         assert( static_cast<std::uint16_t>(container_index) == container_index);
         return DataRange(static_cast<std::uint32_t>(begin_index),
                          static_cast<std::uint16_t>(end_index - begin_index),
                          static_cast<std::uint16_t>(container_index));
      }
   };


   /// @brief Helper class to associate ranges of elements in multiple containers to a contiguous index.
   ///
   /// The elements are stored in containers of a container collection.
   /// The range provides an element range and a container index which identifies the container within
   /// the collection, the The idea is that a contiguous range of elements is added at the end of a
   /// one container of the collection. Once all element of this range are added to the container, the
   /// range is registered for a certain index which identifies the element range e.g. an IdentifierHash
   /// If there is already a range registered for that index, the elements that just had been added
   /// to the end of the container and still constitute the end of the container are erased.
   /// The range can be either an atomic range i.e. provides compare_exchange_weak(is_range, new_range)
   /// or a simple value.
   /// The container has to allow for erasing an element range at the end of the container (erase_back)
   /// @TODO introduce concepts for T_DataContainer and T_RangeType ?
   template <class T_DataContainer, class T_RangeType>
   class IndexedRanges {
   public:
      using DataContainerType = T_DataContainer;
      using T_RangeTypeBase = decltype( GetValue<T_RangeType>::value(T_RangeType{}));

      /// @brief create a container collection for n_ranges and a certain maximum number of containers.
      /// @param n_ranges the maximum number of element ranges which can be registered e.g. PixelID::wafer_hash_max
      /// @param container_list_size the maximum number of containers in the container collection e.g. 1
      IndexedRanges(unsigned int n_ranges, unsigned int container_list_size)
         : m_containers(container_list_size), m_range(n_ranges) {}

      /// @brief register an element range or erase the elements of this range.
      /// @param index and index which identifies the range i.e. the group of elements of this range
      /// @param new_range the element range that should be be registered.
      /// The element range will be registered for the given index unless a range has been
      /// registered already. In the latter case the element of this new range well be
      /// erased.
      bool registerOrEraseNewData(unsigned int index, const T_RangeTypeBase &new_range)
      {
         assert (index < m_range.size());
         assert( new_range.containerIndex() < m_containers.size() );
         T_RangeTypeBase is_range = m_range[index];
         if (is_range.size() != new_range.size()) {
            assert(is_range.empty());
            if (update(index, is_range, new_range)) {
               return true;
            }
         }
         // it is assumed that data is always added at the end and then registered.
         assert( new_range.beginIndex() + new_range.size() == m_containers[new_range.containerIndex()].size());
         m_containers[new_range.containerIndex()].erase_back(new_range.beginIndex());
         return false;
      }
      /// @brief update a range (version for an "atomic" range)
      // @TODO declare protected ?
      bool update(unsigned int index, T_RangeTypeBase & is_range, const T_RangeTypeBase &new_range) requires( isAtomic<T_RangeType> )
      {
         assert( index < m_range.size());
         return m_range[index].compare_exchange_weak(is_range, new_range);
      }
      /// @brief update a range (version for a "non-atomic" range)
      // @TODO declare protected ?
      bool update(unsigned int index, [[maybe_unused]] T_RangeTypeBase & is_range, const T_RangeTypeBase &new_range) requires(!isAtomic<T_RangeType>)
      {
         assert( index < m_range.size());
         m_range[index]=new_range;
         return true;
      }

      /// @brief Get an existing container from the container collection (read only).
      /// @param container_index the index which identifies the container in the container collection.
      const T_DataContainer &data(unsigned int container_index) const
         { assert(container_index<m_containers.size()); return m_containers[container_index]; }

      /// @brief Get an existing container from the container collection.
      /// @param container_index the index which identifies the container in the container collection.
      T_DataContainer &data(unsigned int container_index)
         { assert(container_index<m_containers.size()); return m_containers[container_index]; }

      /// @brief Get a specific element range
      /// @param index the index which identifies the element range
      /// the element range will in addition to the actual element range also provide the
      /// index of the container in container collection which contains these elements.
      const T_RangeType &range(unsigned int index) const {
         assert(index < m_range.size());
         return m_range[index];
      }

      /// @brief return the total number of element ranges.
      std::size_t size() const { return m_range.size(); }
      /// @brief return true there are no element ranges.
      std::size_t empty() const { return m_range.empty(); }

      //// @brief return the maximum number of containers the collection could store without the need to resize the container list.
      std::size_t containerListCapacity() const { return m_containers.size(); }

      /// @brief if there is an associated error container get it.
      const auto &errorContainer(unsigned int container_index) const requires(hasErrors<T_DataContainer>) {
         assert( container_index < m_containers.size() );
         assert( m_containers[container_index].errors().empty() || m_containers[container_index].errors().size() == m_range.size());
         return m_containers[container_index].errors();
      }
      /// @brief if there is an associated error container get it (read/write access).
      auto &errorContainer(unsigned int container_index) requires(hasErrors<T_DataContainer>) {
         assert( container_index < m_containers.size() );
         return m_containers[container_index].errors();
      }

   protected:
      ContainerList<T_DataContainer> m_containers;
      std::vector<T_RangeType>       m_range;
   };


   /// @brief Helper class to be used in conjunction with ProxyContainers.
   /// The class provides a range of child elements and for debugging purposes
   /// also caches the element index of the element which is parent to these
   /// child elements.
   /// This class can be used by a proxy container to implement the necessary methods
   /// for iterating over child elements.
   struct IndexWithRange  {
      unsigned int m_beginIndex {}; ///< index of the first child element in the range
      unsigned int m_endIndex {};   ///< index after the last child element of this range
      unsigned int m_rangeIndex {}; ///< the index if the element which is the parent of the children where the index may refer to a different container

      /// @brief the first element in the range
      unsigned int beginIndex() const { return m_beginIndex; }
      /// @brief the index of the element after the last element
      unsigned int endIndex() const { return m_endIndex; }
      /// @brief return true if this range does not contain any elements.
      bool empty() const { return m_endIndex == m_beginIndex; }

      /// @brief the index which identifies this range within its parent.
      /// For example if this range represents the hits of a module, this could be
      /// the identifier hash.
      const unsigned int &rangeIndex() const { return m_rangeIndex; }

      // only used in unit test
      bool operator==(const IndexWithRange &other) const {
         // should only be executed if the indices refer to the same range
         // thus the cached begin index should always be identical, and
         // comparison is limited to the minimum, which is only sufficient
         // if the the two indices refer to the same range, which is the
         // case when this comparison would be done as part of a range based
         // for loop.
         assert(m_endIndex == other.m_endIndex);
         assert(m_beginIndex == other.m_beginIndex);
         return m_rangeIndex == other.m_rangeIndex;
      }
   };

   // assumed cache line size
   static constexpr std::size_t CACHELINE = 64ul;

   /// @brief Base raw data container which provides coordinates of a certain dimension and a data word per RDO (raw data object).
   ///
   /// The class implements the basic container methods to allow its usage together with  proxy container objects
   /// It also provides methods to bit-pack and unpack information into and from a single data word.
   // In case the raw data is filled concurrently, there will be multiple containers which may be
   // adjacent to one-another. to ensure that concurrent modification of the content of adjacent
   // containers will not change the same cache line, an alignment requirement of the the assumed
   // cache line size is chosen.
   template <std::size_t NDim>
   class alignas(CACHELINE) InDetRawDataContainer {
   public:

      /// @brief return true if the index refers to an element in the container
      bool isValid(unsigned int index) const      { return index < m_coordinates.size() && index < m_word.size(); }

      /// @brief return the coordinates i.e. column, row or strip  of a certain RDO (read only).
      const std::array<std::int16_t,NDim> &coordinates(unsigned int index) const { assert(isValid(index)); return m_coordinates[index]; }
      /// @brief return the coordinates i.e. column, row or strip  of a certain RDO.
      std::array<std::int16_t,NDim> &coordinates(unsigned int index)             { assert(isValid(index)); return m_coordinates[index]; }
      /// @brief return the packed data word of of a certain RDO (read only).
      const std::uint32_t &dataWord(unsigned int index) const                 { assert(isValid(index)); return m_word[index]; }
      /// @brief return the packed data word of of a certain RDO.
      std::uint32_t &dataWord(unsigned int index)                             { assert(isValid(index)); return m_word[index]; }

      // Test whether the container is in a sane state (for debugging).
      // The container is in a sane state if the number elements in all the vectors are synchronised.
      bool isSane() const          { return m_coordinates.size() == m_word.size(); }

      /// @brief total number of RDOs which are in this container.
      std::size_t size() const     { return m_coordinates.size(); }
      /// @brief test whether the container is empty i.e. does not contain any RDOs
      bool empty() const           { return m_coordinates.empty(); }
      /// @brief the maximum number RDOs this container can hold without reallocation.
      std::size_t capacity() const { assert(m_coordinates.capacity() == m_word.capacity()); return m_coordinates.capacity(); }

      /// @brief reserve space for a certain number of RDOs.
      void reserve(std::size_t new_capacity) {
         m_coordinates.reserve(new_capacity);
         m_word.reserve(new_capacity);
         assert(isSane() && m_coordinates.capacity() == m_word.capacity());
      }

      /// @brief Add a new RDO to the end of this container.
      /// @param coordinates the coordinates of the RDO e.g. pixel row, column or strop.
      /// @param data_word a bit packed data word which may contain e.g. the ToT.
      void emplace_back(std::array<std::int16_t,NDim> &&coordinates, std::uint32_t data_word) {
         // @TODO should bail out if container is filled concurrently and the
         //       size is equal to the capacity, but the container does not know.
         m_coordinates.emplace_back(std::move(coordinates));
         m_word.emplace_back(data_word);
         assert(isSane());
      }

      /// @brief erase the elements at the end of the container
      /// @param begin_index the index of the first element that is erased.
      /// Will erase all elements from begin_index till the end of the container.
      /// begin_index is expected to be smaller or equal to the container size.
      /// The result may be undefined if begin_index is larger than the container size.
      void erase_back( unsigned int begin_index) {
         assert( begin_index <= m_coordinates.size());
         m_coordinates.erase( m_coordinates.begin()+begin_index, m_coordinates.end());
         m_word.erase( m_word.begin()+begin_index, m_word.end());
         assert(isSane());
      }

      /// @brief extract a value from a bit packed word.
      /// @param mask the mask to blend out bits not part of a certain value after shifting the data word.
      /// @param shift the first bit of the value
      /// @param word the bit packed data word
      /// @return the unpacked value
      /// Will shift the data word and then apply the mask to blend out bits not part of the value.
      static constexpr int unpack(std::uint32_t mask, unsigned int shift, unsigned int word) {
         return static_cast<int>( (word >> shift ) & mask );
      }

      /// @brief Create a bit-packed word from an input value.
      /// @param mask the mask to blend out bits of the input value which are not part of the actual value.
      /// @param shift the bit number of the first bit in the bit packed data word.
      /// @param input the input value.
      /// @return a bit packed  data word which only represents the input value.
      /// This value can be or-ed with other bit-packed values to form a complete data
      /// word. If the input value exceeds the maximum value fitting into the provided
      /// mask, then the result will be undefined.
      static constexpr std::uint32_t pack(std::uint32_t mask, unsigned int shift, int input) {
         // test that packing is loss-less
         assert( static_cast<int>(static_cast<std::uint32_t>(input) & mask) ==input );
         return static_cast<std::uint32_t>( (input & mask) << shift );
      }

      /// @brief Move the coordinates and the data word of a certain number of elements at the end to a new container.
      /// @param dest the container to which the elements will be moved.
      /// @param start_index the index of the first element that will be moved.
      /// Will copy the elements from start_index until then end to the container dest, and erase the copied
      /// elements from this container.
      void move(InDetRawDataContainer &dest, unsigned int start_index) {
         unsigned int copy_n_elements=size()-start_index;
         dest.m_coordinates.resize(copy_n_elements);
         dest.m_word.resize(copy_n_elements);
         std::memcpy(dest.m_coordinates.data(),m_coordinates.data()+start_index,copy_n_elements*sizeof(typename decltype(m_coordinates)::value_type));
         std::memcpy(dest.m_word.data(), m_word.data()+start_index, copy_n_elements*sizeof(typename decltype(m_word)::value_type) );
         this->erase_back(start_index);
      }
   private:
      std::vector<std::array<std::int16_t,NDim> > m_coordinates;
      std::vector<std::uint32_t>                  m_word;
   };


   using namespace Utils;

   /// @brief Base class for an RDO proxy.
   ///
   /// The proxy objects provide an interface which provides on object like interface
   /// for the data stored in the base container.
   /// It provides access to hit coordinates of a single hit and an associated data word, but
   /// lacks the means to interpret the data word.
   template <class T_RawDataContainer>
   class RawDataProxyBase : public Utils::ElementProxyBase<T_RawDataContainer, unsigned int> {
   public:
      using BASE = Utils::ElementProxyBase<T_RawDataContainer, unsigned int>;
      using BASE::BASE;

      const auto &coordinates() const {
         return this->container().coordinates(this->index());
      }
      auto &coordinates() requires (!BASE::isConst)  {
         return this->container().coordinates(this->index());
      }
      const auto &dataWord() const {
         return this->container().dataWord(this->index());
      }
      auto &dataWord() requires (!BASE::isConst)  {
         return this->container().dataWord(this->index());
      }
   };

   /// @brief A proxy providing access to the RDOs of a single module.
   /// @tparam T_RawDataContainer the hit container which provides the data for the proxies.
   /// @tparam T_RawDataProxy a proxy class which provides access to the properties of a single hit
   ///                    and is returned for each child element this proxy provides.
   /// The proxy represents a child element range i.e. RDO range, and will provide a proxy
   /// object for each of its children.
   template <class T_RawDataContainer, class T_RawDataProxy>
   class RawDataContainerProxy :  public ContainerProxy<T_RawDataContainer,
                                                  RawDataContainerProxy<T_RawDataContainer, T_RawDataProxy>,
                                                  T_RawDataProxy,
                                                  IndexWithRange>  {
   public:
      using BASE = ContainerProxy<T_RawDataContainer,
                                  RawDataContainerProxy<T_RawDataContainer, T_RawDataProxy>,
                                  T_RawDataProxy,
                                  IndexWithRange> ;

      using BASE::BASE;

      /// @brief method which returns the index of the first child element this proxy represents
      static unsigned int beginIndex([[maybe_unused]] const T_RawDataContainer *container, const IndexWithRange &range)
      {
         assert( range.empty()  || container);
         return range.beginIndex();
      }
      /// @brief method which returns the index of the element after the last child element this proxy represents
      static unsigned int endIndex([[maybe_unused]] const T_RawDataContainer *container, const IndexWithRange &range) {
         assert( range.empty()  || container);
         return range.endIndex();
      }
      /// @brief method which returns the element which follows the element specified by element_index
      static unsigned int nextElementIndex([[maybe_unused]] const T_RawDataContainer *container, unsigned int element_index)
      {
         assert( element_index < container->size());
         return ++element_index;
      }

      /// @brief method which returns the identifier hash of the module this proxy represents.
      const unsigned int &identifyHash() const { return this->index().rangeIndex(); }

      // convenience methods
      unsigned int beginIndex() const { return this->index().beginIndex(); }
      unsigned int endIndex() const { return this->index().endIndex(); }

   };

   namespace RawData {
   namespace details {
   // to define the proxy that should be used for the given raw data container.
   template <class T_RawDataContainer>
   struct traits  {
      template <AccessPolicy accessPolicy>
      using RawDataProxy = RawDataProxyBase<typename Utils::ContainerAccessHelper<T_RawDataContainer, accessPolicy>::ContainerType >;
   };
   }
   }

   /// @brief An adapter to create a container proxy from a range index which defines the actual element container
   ///        and the element range
   /// @tparam T_RawDataContainerCollection the hit container collection which provides the data for the hits,
   ///                        but also provides the container index and  element ranges for a module index.
   ///                        it has to implement the method range(module_index), and data(container_index)
   /// @tparam T_RawDataProxy a proxy class which provides access to the properties of a single hit
   ///     and is returned for each child element this proxy provides.
   // @TODO add concepts for T_RawDataContainerCollection
   template <class T_RawDataContainerCollection, class T_RawDataProxy>
   class ContainerProxyAdapter : public Utils::ElementProxyBase<T_RawDataContainerCollection,unsigned int >
   {
      using BASE = Utils::ElementProxyBase<T_RawDataContainerCollection,unsigned int >;
      using T_Range = typename T_RawDataContainerCollection::T_RangeTypeBase;
      using T_RawDataContainer = std::remove_cvref_t<decltype(std::declval<T_RawDataContainerCollection>().data(std::uint32_t{}))>;
      // Create a special index which also provided the child element range.
      static IndexWithRange getIndexWithRange(const T_RawDataContainerCollection &container, unsigned int module_index) {
         T_Range range = container.range(module_index); // container has to implement range(module_index)
         return IndexWithRange(range.beginIndex(),
                               range.endIndex(),
                               module_index);
      }
      // Get from this container collection the actual container which contains the hit data (read only).
      static const T_RawDataContainer *getRawDataContainer(const T_RawDataContainerCollection &container, unsigned int module_index) {
         T_Range range = container.range(module_index);
         return &(container.data(range.containerIndex()));
      }
      // Get from this container collection the actual container which contains the hit data (read/write).
      static T_RawDataContainer *getRawDataContainer(T_RawDataContainerCollection &container, unsigned int module_index)
         requires(!BASE::isConst)
      {
         T_Range range = container.range(module_index);
         return &(container.data(range.containerIndex()));
      }
   public:
      using BASE::BASE;

      /// @brief Create the actual container proxy for the elements this proxy refers to.
      /// This is contraction of creating this proxy and converting it into a new one. It can result in a read/write or read only proxy.
      static RawDataContainerProxy<T_RawDataContainer, T_RawDataProxy>
      create(T_RawDataContainerCollection *container, unsigned int module_index)
      {
         assert(container);
         RawDataContainerProxy<T_RawDataContainer, T_RawDataProxy> proxy( getRawDataContainer(*container,module_index),
                                                                           getIndexWithRange(*container,module_index) );
         return proxy;
      }

      /// @brief Create the actual container proxy for the elements this proxy refers to.
      ///
      /// This is contraction of creating this proxy and converting it into a new one. It will result in a read only proxy.
      static RawDataContainerProxy<const T_RawDataContainer, T_RawDataProxy>
      create(const T_RawDataContainerCollection *container, unsigned int module_index)
         requires(BASE::isConst)
      {
         assert(container);
         RawDataContainerProxy<const T_RawDataContainer, T_RawDataProxy> proxy( getRawDataContainer(*container,module_index),
                                                                                 getIndexWithRange(*container,module_index) );
         return proxy;
      }

      /// @brief Supporting method for "converted" proxies resulting from the methods above to recover the original "child" index.
      ///
      /// the original child index is the index which can be used to recover the given converted_element_proxy using
      /// the access operator [] on the parent proxy.
      static unsigned int getOriginalElementIndex(const IndexWithRange &index) {
         return index.rangeIndex();
      }
   };

   /// @brief Helper class to represent the top level proxy which provides  per module proxy objects which provide the hit element proxies.
   template <class T_RawDataContainerCollection, class T_RawDataProxy>
   class ContainerCollectionProxy : public ContainerProxy<T_RawDataContainerCollection,
                                                          ContainerCollectionProxy<T_RawDataContainerCollection, T_RawDataProxy>,
                                                          ContainerProxyAdapter<T_RawDataContainerCollection, T_RawDataProxy>,
                                                          RootNodeIndex > {
      using BASE=ContainerProxy<T_RawDataContainerCollection,
                                ContainerCollectionProxy<T_RawDataContainerCollection, T_RawDataProxy>,
                                ContainerProxyAdapter<T_RawDataContainerCollection, T_RawDataProxy>,
                                RootNodeIndex >;
      using BASE::BASE;
   };

   /// @brief helper class to define all the proxies for a RDO container.
   template <class T_RawDataContainer>
   struct RawDataTypeTraits {
      static constexpr bool isConst = std::is_const_v<T_RawDataContainer>;
      static constexpr Utils::AccessPolicy accessPolicy=AccessPolicyHelper<isConst>::accessPolicy;
      using ContainerNonConst = std::remove_cvref_t<T_RawDataContainer>;

      // the collection should be based on the non const container
      using ContainerCollection = typename Utils::ContainerAccessHelper<PhaseII::IndexedRanges<ContainerNonConst, std::atomic<PhaseII::DataRange> >,
                                                                        accessPolicy>::ContainerType;
      // but the proxies if (const) refer to the const collection and finally the const container.
      using RawDataProxy = typename RawData::details::traits<ContainerNonConst>::template RawDataProxy<accessPolicy>;
      using RawDataContainerProxy  = PhaseII::RawDataContainerProxy<typename Utils::ContainerAccessHelper<T_RawDataContainer,
                                                                                                          accessPolicy>::ContainerType,
                                                                    RawDataProxy >;
      using ContainerProxyAdapter  = PhaseII::ContainerProxyAdapter<typename Utils::ContainerAccessHelper<ContainerCollection,
                                                                                                          accessPolicy>::ContainerType,
                                                                    RawDataProxy>;
      using ContainerCollectionProxy = PhaseII::ContainerCollectionProxy<typename Utils::ContainerAccessHelper<ContainerCollection,
                                                                                                               accessPolicy>::ContainerType,
                                                                         RawDataProxy>;
   };

   /// @brief Create the top level container proxy for an RDO container collection (read only).
   template <class T_RawDataContainerCollection>
   inline auto makeRawDataCollectionProxy(const T_RawDataContainerCollection &collection) {
      using T_RawDataContainer = typename T_RawDataContainerCollection::DataContainerType;
      static_assert( RawDataTypeTraits<const T_RawDataContainer>::ContainerCollectionProxy::isConst);
      using ContainerNonConst = std::remove_cvref_t<T_RawDataContainerCollection>;
      static_assert( std::is_base_of_v<typename RawDataTypeTraits<const T_RawDataContainer>::ContainerCollectionProxy::ContainerNonConst,
                                       ContainerNonConst>);
      return typename RawDataTypeTraits<const T_RawDataContainer>::ContainerCollectionProxy(&collection);
   }

   /// @brief  Create the top level container proxy for an RDO container collection (read/write).
   template <class T_RawDataContainerCollection>
   inline auto makeRawDataCollectionProxy(T_RawDataContainerCollection &collection) {
      using T_RawDataContainer = typename T_RawDataContainerCollection::DataContainerType;
      return typename RawDataTypeTraits<T_RawDataContainer>::ContainerCollectionProxy(&collection);
   }
}
#endif
