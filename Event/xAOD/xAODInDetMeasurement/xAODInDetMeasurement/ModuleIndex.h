#ifndef ACTSTRK_MODULEINDEX_H
#define ACTSTRK_MODULEINDEX_H
#include "AthLinks/DataLink.h"
#include "ModuleIndexBase.h"

// Class to store per module element ranges,
// pointers to all referenced source containers,
// and a selection of elements per module.
template <typename T_Container>
class ModuleIndex : public ModuleIndexBase {
public:
   using ModuleIndexBase::ModuleIndexBase;
   using DataRangeValueType = ModuleIndexBase::DataRangeValueType;
   
   unsigned int containerIndex(const T_Container &src_container) {
      typename std::vector< DataLink<T_Container> >::const_iterator iter =
         std::find_if(m_srcContainer.begin(),m_srcContainer.end(), [container_ptr=&src_container](const DataLink<T_Container> &link) {
            return container_ptr == link.cptr();
         });
      unsigned int index = iter - m_srcContainer.begin();
      if (iter == m_srcContainer.end()) {
         m_srcContainer.emplace_back(DataLink<T_Container>(&src_container));
      }
      return index;
   }
   const T_Container &container(const PhaseII::DataRange &range) const {
      assert( range.containerIndex() < m_srcContainer.size());
      return *(reinterpret_cast<const DataLink<T_Container> &>(m_srcContainer[range.containerIndex()]).cptr());
   }

   // will call a function for each container, passing a const reference of the container
   // @TODO introduce concept
   template <typename T_Function>
   void visitContainers(T_Function &&function) const {
      for (const DataLink<T_Container> &link: m_srcContainer) {
         function( *(link.cptr()) );
      }
   }
   void copyContainerList(const ModuleIndex<T_Container> &other) {
      m_srcContainer = other.m_srcContainer;
   }
   unsigned int containerListSize() const {
      return m_srcContainer.size();
   }

protected:
   std::vector< DataLink<T_Container> > m_srcContainer; // list of source countainers the range refers to.

};

#endif
