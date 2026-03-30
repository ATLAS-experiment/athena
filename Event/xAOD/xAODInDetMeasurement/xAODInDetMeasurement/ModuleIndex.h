#ifndef ACTSTRK_MODULEINDEX_H
#define ACTSTRK_MODULEINDEX_H
#include "InDetRawData/PhaseIIInDetRawDataContainer.h"

#include <vector>

// Structure to store per module element ranges,
// pointers to all referenced source containers,
// and a selection of elements per module.
template <typename T_Container>
struct ModuleIndex {
   unsigned int containerIndex(const T_Container &src_container) {
      typename std::vector< const T_Container *>::const_iterator iter =
         std::find(m_srcContainer.begin(),m_srcContainer.end(),&src_container);
      unsigned int index = iter - m_srcContainer.begin();
      if (iter == m_srcContainer.end()) {
         m_srcContainer.push_back(&src_container);
      }
      return index;
   }
   unsigned int elementIndex(unsigned int selection_index) const {
      assert( selection_index< m_selection.size());
      return m_selection.empty() ? selection_index : m_selection[selection_index];
   }
   const PhaseII::DataRange &range(unsigned int identifier_hash) const {
      assert( identifier_hash < m_range.size());
      return m_range[identifier_hash];
   }
   void registerRange(unsigned int identifier_hash, unsigned int begin_index, unsigned int end_index, unsigned int container_index) {
      m_range[identifier_hash]=PhaseII::DataRange(begin_index, end_index - begin_index, container_index);
   }
   const T_Container &container(const PhaseII::DataRange &range) const {
      assert( range.containerIndex() < m_srcContainer.size());
      return *m_srcContainer[range.containerIndex()];
   }

   std::vector< const T_Container *> m_srcContainer; // list of source countainers the range refers to.
   std::vector<PhaseII::DataRange> m_range; // the per module element range refers to either the element selection, or if empty to all elements.
   std::vector<unsigned int> m_selection;   // selection of elements per module
};

#endif
