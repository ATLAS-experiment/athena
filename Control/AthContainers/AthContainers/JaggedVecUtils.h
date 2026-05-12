/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
  */
#ifndef SG_JAGGEDVECUTILS_H
#define SG_JAGGEDVECUTILS_H

#include "AthContainers/AuxVectorData.h"
namespace SG {

/// Convenience method to resize the payload and get the spans of jagged vectors (end index, payload).
template <typename T_StoreType, typename T_JaggedVecAccessor, typename T_SpanElt, typename T_SpanPayload>
void setJaggedVectorData(SG::AuxVectorData& cont,
                         T_StoreType &store,
                         const T_JaggedVecAccessor &accessor,
                         unsigned int n_cluster_rdos,
                         T_SpanElt &elt,
                         T_SpanPayload &payload) {
   if (n_cluster_rdos>0u) {
      store.getData(accessor.linkedAuxid(),n_cluster_rdos, n_cluster_rdos);
      elt = accessor.getEltSpan(cont);
      payload = accessor.getPayloadSpan(cont);
   }
}

}
#endif
