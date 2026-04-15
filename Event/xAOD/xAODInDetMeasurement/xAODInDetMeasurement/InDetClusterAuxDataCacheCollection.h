#ifndef INDETCLUSTERAUXDATACACHECOLLECTION_H
#define INDETCLUSTERAUXDATACACHECOLLECTION_H

namespace traits {
   template <typename T>
   concept has_moduleIndex = requires (T &&a) { a.moduleIndex(); };
}

template<typename T_AuxDataCache, typename T_Container>
struct InDetClusterAuxDataCacheCollection  {
   std::vector< T_AuxDataCache > m_caches;
   const ModuleIndex<std::remove_cvref_t<T_Container> > *m_moduleIndex;

   static std::vector< T_AuxDataCache > make(T_Container &container)  {
      std::vector< T_AuxDataCache > ret{ T_AuxDataCache(container) };
      return ret;
   }
   static std::vector< T_AuxDataCache > make(const std::vector<DataLink<std::remove_cvref_t<T_Container> > > &container_list)  {
      std::vector< T_AuxDataCache > ret;
      ret.reserve(container_list.size());
      for (const auto &container_link : container_list) {
         ret.emplace_back(*(container_link.cptr()));
      }
      return ret;
   }

   InDetClusterAuxDataCacheCollection(T_Container &container) requires( traits::has_moduleIndex<std::remove_cvref_t<T_Container> > )
   : m_caches( container.moduleIndex().m_srcContainer.empty() ?  make(container) : make(container.moduleIndex().m_srcContainer)),
     m_moduleIndex(&container.moduleIndex())
   {}

   // @TODO needed ?
   InDetClusterAuxDataCacheCollection(T_Container &container) requires( !traits::has_moduleIndex<std::remove_cvref_t<T_Container> > )
   : m_caches(make(container)),
     m_moduleIndex(nullptr)
   {}
   auto range(unsigned int identifier_hash) const {
      assert(m_moduleIndex);
      return m_moduleIndex->range(identifier_hash);
   }
   const std::remove_cvref_t<T_AuxDataCache> &container(const PhaseII::DataRange &range) const {
      assert(range.containerIndex() < m_caches.size() );
      return m_caches[range.containerIndex()];
   }
   const std::vector<unsigned int> &selection() const {
      assert( m_moduleIndex);
      return m_moduleIndex->m_selection;
   }
   // return the number of ranges (needed by the container proxy)
   std::size_t size() const {
      return m_moduleIndex->m_range.size();
   }
   bool emptyy() const {
      return m_moduleIndex->m_range.empty();
   }

   /// get the interface object representing a certain element of a certain container.
   /// @param container_index the index of the container in this container collection.
   /// #param element_index the index of the element in the selection for this container.
   typename T_Container::const_value_type getInterfaceObject(unsigned int container_index, unsigned int element_index) const {
      assert(container_index < m_caches.size() );
      const auto &the_container = m_caches[container_index];
      assert(m_moduleIndex);
      if (!m_moduleIndex->m_selection.empty()) {
         assert( element_index < m_moduleIndex->m_selection.size());
         element_index=m_moduleIndex->m_selection[element_index];
      }
      return the_container.getInterfaceObject(element_index);
   }

   const ModuleIndex<std::remove_cvref_t<T_Container> > &moduleIndex() const {
      return *m_moduleIndex;
   }
};

   struct IndexWithSelection {
      const std::vector<unsigned int> *m_selection;
      unsigned int m_index;

      IndexWithSelection &operator++() {
         ++m_index;
         return *this;
      }
      bool isValid( std::size_t container_size) const {
         assert( m_selection);
         return !m_selection->empty() ? m_index < m_selection->size() : m_index < container_size;
      }
      unsigned int inContainerIndex() const {
         assert(    m_selection->empty()
                 || (!m_selection->empty() && m_index < m_selection->size()));
         return !m_selection->empty() ? (*m_selection)[m_index] : m_index;
      }
      unsigned int rangeIndex() const {
         return m_index;
      }
      bool operator==(const IndexWithSelection &other) const {
         // the selection only caches values for the next step
         // this index should be fully defined by the range index. Thus, only the
         // latter needs to be compared.
         assert( m_selection == other.m_selection);
         return rangeIndex() == other.rangeIndex();
      }
   };

   inline std::int64_t operator-(const IndexWithSelection &a, const IndexWithSelection &b) {
      return a.rangeIndex()-b.rangeIndex();
   }

   template <typename T_AuxDataCache, typename T_ClusterProxy>
   class ClusterProxyAdapter : public Utils::ElementProxyBase<T_AuxDataCache,IndexWithSelection >
   {
   public:
      static T_ClusterProxy
      create(T_AuxDataCache *container, const IndexWithSelection &index)
      {
         assert(container);
         T_ClusterProxy proxy( container, index.inContainerIndex());
         return proxy;
      }
      static unsigned int getOriginalElementIndex(const IndexWithSelection &index) {
         return index.rangeIndex();
      }
      // @TODO what about the T_ClusterProxy, does that  not also need a method getOriginalElementIndex
      //       to recover the index to get this proxy from the parent ?
   };

   struct IndexWithSubsetRange : PhaseII::IndexWithRange {
      const std::vector<unsigned int> *m_selection;

      IndexWithSubsetRange(unsigned int begin_index, unsigned int end_index, unsigned int identifier_hash, const std::vector<unsigned int> &selection)
         : PhaseII::IndexWithRange{begin_index, end_index, identifier_hash},
           m_selection(&selection)
      {}
      using PhaseII::IndexWithRange::IndexWithRange;
      unsigned int convertIndex(unsigned int index) {
         assert(m_selection);
         assert(index < m_selection->size() );
         return !m_selection->empty() ? (*m_selection)[index] : index;
      }
      const std::vector<unsigned int> &selection() const {
         assert(m_selection);
         return *m_selection;
      }
      bool isValid([[maybe_unused]] std::size_t container_size) const {
         assert( m_selection);
         return true;
      }
      IndexWithSubsetRange &operator++() {
         ++m_rangeIndex;
         return *this;
      }
      bool operator==(const IndexWithSubsetRange &other) const {
         // the range and the selection only caches some values for the next step
         // this index should be fully defined by the range index. Thus, only the
         // latter needs to be compared.
         assert( m_selection == other.m_selection);
         assert( beginIndex() == other.beginIndex());
         assert( endIndex() == other.endIndex());
         return rangeIndex() == other.rangeIndex();
      }
   };

   inline std::int64_t operator-(const IndexWithSubsetRange &a, const IndexWithSubsetRange &b) {
      return a.rangeIndex()-b.rangeIndex();
   }


   template <typename T_AuxDataCache, typename T_ClusterProxy>
   class ClusterContainerProxy :  public Utils::ContainerProxy<T_AuxDataCache,
                                                               ClusterContainerProxy<T_AuxDataCache, T_ClusterProxy>,
                                                               ClusterProxyAdapter<T_AuxDataCache,T_ClusterProxy>,
                                                               IndexWithSubsetRange>  {
   public:
      using BASE = Utils::ContainerProxy<T_AuxDataCache,
                                         ClusterContainerProxy<T_AuxDataCache, T_ClusterProxy>,
                                         ClusterProxyAdapter<T_AuxDataCache,T_ClusterProxy>,
                                         IndexWithSubsetRange>;
      using BASE::BASE;

      /// @brief method which returns the index of the first child element this proxy represents
      static IndexWithSelection beginIndex([[maybe_unused]] const T_AuxDataCache *container, const IndexWithSubsetRange &range)
      {
         assert( range.empty()  || container);
         return IndexWithSelection{&(range.selection()), range.beginIndex() };
      }
      /// @brief method which returns the index of the element after the last child element this proxy represents
      static IndexWithSelection endIndex([[maybe_unused]] const T_AuxDataCache *container, const IndexWithSubsetRange &range) {
         assert( range.empty()  || container);
         return IndexWithSelection{&(range.selection()), range.endIndex() };
      }
      /// @brief method which returns the element which follows the element specified by element_index
      static IndexWithSelection nextElementIndex([[maybe_unused]] const T_AuxDataCache *container, IndexWithSelection &&element_index)
      {
         assert( element_index.isValid(container->size())  );
         return ++element_index;
      }

      /// @brief method which returns the identifier hash of the module this proxy represents.
      const unsigned int &identifyHash() const { return this->index().rangeIndex(); }

      // @TODO does this not need a method getOriginalElementIndex to recover the index to
      //       get this proxy from the parent ?
   };

   template <typename T_AuxDataCacheCollection, typename T_ClusterProxy>
   class ContainerProxyAdapter : public Utils::ElementProxyBase<T_AuxDataCacheCollection,unsigned int >
   {
      using BASE = Utils::ElementProxyBase<T_AuxDataCacheCollection,unsigned int >;
      using T_Range = PhaseII::DataRange;
      using T_vector = std::remove_cvref_t<decltype(std::declval<T_AuxDataCacheCollection>().m_caches)>;
      using T_AuxDataCacheNonConst = typename std::remove_cvref_t<decltype(std::declval<T_AuxDataCacheCollection>().m_caches)>::value_type;
      using T_AuxDataCache = std::conditional<std::is_const_v<T_AuxDataCacheCollection>, const T_AuxDataCacheNonConst, T_AuxDataCacheNonConst>::type;
      // Create a special index which also provided the child element range.
      static IndexWithSubsetRange getIndexWithRange(T_AuxDataCacheCollection &collection, unsigned int identifier_hash) {
         assert(collection.m_moduleIndex);
         T_Range range = collection.range(identifier_hash); // container has to implement range(module_index)
         return IndexWithSubsetRange(range.beginIndex(),
                                     range.endIndex(),
                                     identifier_hash,
                                     collection.selection());
      }
      // Get from this container collection the actual container which contains the hit data (read only).
      static T_AuxDataCache *getContainer(T_AuxDataCacheCollection &collection, unsigned int identifier_hash) {
         T_Range range = collection.range(identifier_hash);
         return &collection.container(range);
      }
   public:
      using BASE::BASE;

      /// @brief Create the actual container proxy for the elements this proxy refers to.
      /// This is contraction of creating this proxy and converting it into a new one. It can result in a read/write or read only proxy.
      static ClusterContainerProxy<T_AuxDataCache, T_ClusterProxy>
      create(T_AuxDataCacheCollection *collection, unsigned int identifier_hash)
      {
         assert(collection);
         ClusterContainerProxy<T_AuxDataCache, T_ClusterProxy> proxy( getContainer(*collection,identifier_hash),
                                                                      getIndexWithRange(*collection,identifier_hash) );
         return proxy;
      }

      static unsigned int getOriginalElementIndex(const IndexWithSubsetRange &index) {
         return index.rangeIndex();
      }
   };

   /// @brief Helper class to represent the top level proxy which provides  per module proxy objects which provide the hit element proxies.
   template <typename T_AuxDataCacheCollection,  typename T_ClusterProxy>
   class ContainerCollectionProxy : public Utils::ContainerProxy<T_AuxDataCacheCollection,
                                                                 ContainerCollectionProxy<T_AuxDataCacheCollection, T_ClusterProxy>,
                                                                 ContainerProxyAdapter<T_AuxDataCacheCollection, T_ClusterProxy>,
                                                                 Utils::RootNodeIndex > {
      using BASE=Utils::ContainerProxy<T_AuxDataCacheCollection,
                                                                 ContainerCollectionProxy<T_AuxDataCacheCollection, T_ClusterProxy>,
                                                                 ContainerProxyAdapter<T_AuxDataCacheCollection, T_ClusterProxy>,
                                                                 Utils::RootNodeIndex >;
      using BASE::BASE;
   };


#endif
