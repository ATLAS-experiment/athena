#ifndef ACTSTRK_MODULEINDEXBASE_H
#define ACTSTRK_MODULEINDEXBASE_H
#include "InDetRawData/PhaseIIInDetRawDataContainer.h"
#include "AthLinks/DataLinkBase.h"

#include <vector>

// Structure to store per module element ranges,
// pointers to all referenced source containers,
// and a selection of elements per module.
class ModuleIndexBase {
public:
   using DataRangeValueType = decltype(std::declval<PhaseII::DataRange>().m_payload.m_compactRange);

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
   void setIdentifierHashMax(unsigned int identifier_hash_max) {
      m_range.resize(identifier_hash_max);
   }
   unsigned int identifierHashMax() const {
      return m_range.size();
   }
   const std::vector<unsigned int> &selection() const { return m_selection; };
   std::vector<unsigned int> &selection() { return m_selection; };

   const std::vector<DataRangeValueType> &range() const { return m_range; };

protected:
   std::vector<DataRangeValueType> m_range; // the per module element range refers to either the element selection, or if empty to all elements.
   std::vector<unsigned int> m_selection;   // selection of elements per module
};

#endif
