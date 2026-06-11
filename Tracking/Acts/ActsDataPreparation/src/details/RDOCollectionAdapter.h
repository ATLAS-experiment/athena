/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
  */
#ifndef ACTSTRK_RDOCOLLECTIONADAPTER_H
#define ACTSTRK_RDOCOLLECTIONADAPTER_H

namespace ActsTrk {
   // helper class to adapt different RDO collections to have the same interface
   // forward declaration needs dedicated implementation
   template <typename T_RDOContainer>
   class RDOCollectionAdapter {
      const T_RDOContainer::base_value_type *m_RDOs;
   public:
      RDOCollectionAdapter(const typename T_RDOContainer::base_value_type *RDOs)
         : m_RDOs(RDOs)
      {}
      RDOCollectionAdapter(const typename T_RDOContainer::base_value_type &RDOs)
         : m_RDOs(&RDOs)
      {}
      // test whether this object can be dereferenced to return the representation of a single module.
      bool isValid() const {
         return m_RDOs!=nullptr;
      }
      operator bool() const { return isValid();}
      // dereferencing will return a representation of one module.
      const typename T_RDOContainer::base_value_type &operator*() const {
         assert(m_RDOs);
         return *m_RDOs;
      }
      // return a pointer to the representation of one module.
      const typename T_RDOContainer::base_value_type *operator->() const {
         assert(m_RDOs);
         return m_RDOs;
      }
      // check whether the RDO collection represented by this object is empty.
      // @TODO or rather implement operator-> ? That would give this adapter a more consistent interface
      //       since it rather represents a pointer to an object than an object.
      bool empty() const {
         assert(m_RDOs);
         return m_RDOs->empty();
      }
      // result represents the module of the given id_hash when dereferenced.
      // @note must only be dereferenced if isValid is true.
      static std::optional<RDOCollectionAdapter> make(const T_RDOContainer &rdo_container,const IdentifierHash &id_hash) {
         const typename T_RDOContainer::base_value_type *RDOs = rdo_container.indexFindPtr(id_hash);
         if (RDOs) { return RDOCollectionAdapter(*RDOs); }
         else { return std::optional<RDOCollectionAdapter>{}; }
      }
      // result represents an iterable range where each element represents one module
      static const T_RDOContainer &range(const T_RDOContainer &rdo_container) {
         return rdo_container;
      }
   };

}
#endif
