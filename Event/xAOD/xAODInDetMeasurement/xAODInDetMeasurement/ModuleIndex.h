#ifndef ACTSTRK_MODULEINDEX_H
#define ACTSTRK_MODULEINDEX_H
#include "InDetRawData/PhaseIIInDetRawDataContainer.h"
#include "AthLinks/DataLink.h"

#include <vector>

// Structure to store per module element ranges,
// pointers to all referenced source containers,
// and a selection of elements per module.
template <typename T_Container>
struct ModuleIndex {
   using DataRangeValueType = decltype(std::declval<PhaseII::DataRange>().m_payload.m_compactRange);
   
   unsigned int containerIndex(const T_Container &src_container) {
      typename std::vector< DataLink<T_Container> >::const_iterator iter =
         std::find_if(m_srcContainer.begin(),m_srcContainer.end(), [container_ptr=&src_container](const DataLink<T_Container> &link) {
            return container_ptr == link.cptr();
         });
      unsigned int index = iter - m_srcContainer.begin();
      if (iter == m_srcContainer.end()) {
         m_srcContainer.emplace_back(&src_container);
      }
      return index;
   }
   unsigned int elementIndex(unsigned int selection_index) const {
      assert( m_selection.empty() || selection_index< m_selection.size());
      return m_selection.empty() ? selection_index : m_selection[selection_index];
   }
   PhaseII::DataRange range(unsigned int identifier_hash) const {
      assert( identifier_hash < m_range.size());
      return m_range[identifier_hash];
   }
   static void setDataRangeValue(DataRangeValueType &range, unsigned int begin_index, unsigned int end_index, unsigned int container_index) {
      static_assert( sizeof(PhaseII::DataRange) == sizeof(DataRangeValueType));
      // the following creates better code than makeDataRange(..).makeCompact()
      reinterpret_cast<PhaseII::DataRange &>(range)=PhaseII::DataRange::makeDataRange(begin_index,end_index,container_index);
   }
   void registerRange(unsigned int identifier_hash, unsigned int begin_index, unsigned int end_index, unsigned int container_index) {
      assert( identifier_hash < m_range.size());
      setDataRangeValue(m_range[identifier_hash], begin_index, end_index, container_index);
   }
   const T_Container &container(const PhaseII::DataRange &range) const {
      assert( range.containerIndex() < m_srcContainer.size());
      return *(m_srcContainer[range.containerIndex()].cptr());
   }

   std::vector< DataLink<T_Container> > m_srcContainer; // list of source countainers the range refers to.
   std::vector<DataRangeValueType> m_range; // the per module element range refers to either the element selection, or if empty to all elements.
   std::vector<unsigned int> m_selection;   // selection of elements per module
};

#endif
