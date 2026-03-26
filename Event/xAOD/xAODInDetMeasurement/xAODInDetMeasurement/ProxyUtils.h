#ifndef PROXYUTILS_H
#define PROXYUTILS_H
namespace Utils {

   // An index usable by a proxy which caches the child element range
   struct IndexWithRange  {
      unsigned int m_beginIndex {}; ///< index of the first child element in the range
      unsigned int m_endIndex {};   ///< index after the last child element of this range
#ifndef NDEBUG
      unsigned int m_rangeIndex {}; ///< the index if the element which is the parent of the childs where the index may refer to a different container
#endif
      IndexWithRange(unsigned int beginIndex,
                     unsigned int endIndex,
                     [[maybe_unused]] unsigned int parentIndex)
         : m_beginIndex(beginIndex),
           m_endIndex(endIndex)
#ifndef NDEBUG
         ,m_rangeIndex(parentIndex)
#endif
      {  }

      unsigned int beginIndex() const { return m_beginIndex; }
      unsigned int endIndex() const { return m_endIndex; }
      bool empty() const { return m_endIndex == m_beginIndex; }

      // for debugging
#ifndef NDEBUG
      const unsigned int &rangeIndex() const { return m_rangeIndex; }
#endif
   };

   // Simple container proxy where the index caches the child element range
   template<typename T_Container, typename T_ElementProxy>
   struct ElementRangeProxy :public Utils::ContainerProxy<T_Container,
                                                          ElementRangeProxy<T_Container, T_ElementProxy>,
                                                          T_ElementProxy,
                                                          IndexWithRange>  {
   public:
      using BASE = Utils::ContainerProxy<T_Container,
                                         ElementRangeProxy<T_Container, T_ElementProxy>,
                                         T_ElementProxy,
                                         IndexWithRange>;

      using BASE::BASE;

      static unsigned int beginIndex([[maybe_unused]] const T_Container *container, const IndexWithRange &range)
      {
         assert( range.empty()  || container);
         return range.beginIndex();
      }
      static unsigned int endIndex([[maybe_unused]] const T_Container *container, const IndexWithRange &range) {
         assert( range.empty()  || container);
         return range.endIndex();
      }
      static unsigned int nextElementIndex([[maybe_unused]] const T_Container *container, unsigned int element_index)
      {
         return ++element_index;
      }
   };
}
#endif
