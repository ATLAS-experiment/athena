#ifndef XAOD_INDETMEASUREMENT_UTILITIES_JAGGEDVECELTCACHE_H
#define XAOD_INDETMEASUREMENT_UTILITIES_JAGGEDVECELTCACHE_H

#include "AthContainers/JaggedVecAccessor.h"

namespace xAOD::xAODInDetMeasurement::Utilities {
template <typename T>
class JaggedVecEltCache {
public:
   JaggedVecEltCache(SG::AuxVectorData& cont, const SG::Accessor<SG::JaggedVecElt<T> > &accessor, unsigned int n) {
      if (n>0) {
         assert(cont.getStore());
         cont.getStore()->getData(accessor.linkedAuxid(),n, n);
         m_elt = accessor.getEltSpan(cont);
         m_payload = accessor.getPayloadSpan(cont);
      }
   }
   unsigned int getBeginIndex(unsigned int obj_i) const {
      assert(obj_i==0 || obj_i-1 < m_elt.size());
      return obj_i>0 ? m_elt[obj_i-1].end() : 0u;
   }
   unsigned int getEndIndex(unsigned int obj_i) const {
      assert(obj_i < m_elt.size());
      return m_elt[obj_i].end();
   }
   void setValue(unsigned int elm_i, T &&value) {
      assert(elm_i<m_payload.size());
      m_payload[elm_i]=std::move(value);
   }
   void setValue(unsigned int elm_i, const T &value) {
      assert(elm_i<m_payload.size());
      m_payload[elm_i]=value;
   }
   void updateEndIndex(unsigned int obj_i, unsigned int elm_i) {
      assert(elm_i<=m_payload.size());
      assert(obj_i<m_elt.size());
      m_elt[obj_i]=elm_i;
   }
   std::size_t nObjects() const { return m_elt.size(); };
   std::size_t nElements() const { return m_payload.size(); };
private:
   SG::Accessor<SG::JaggedVecElt<T> >::Elt_span     m_elt;
   SG::Accessor<SG::JaggedVecElt<T> >::Payload_span m_payload;
};
}
#endif
