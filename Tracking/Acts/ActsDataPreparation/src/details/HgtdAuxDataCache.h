#ifndef ACTSTRK_HGTDAUXDATACACHE_H
#define ACTSTRK_HGTDAUXDATACACHE_H

#include "AthContainers/JaggedVecUtils.h"
#include "xAODCore/VariableStruct.h"
#include <xAODInDetMeasurement/HGTDCluster.h>

struct HgtdAuxDataCache : xAOD::VariableStruct {
   HgtdAuxDataCache(SG::AuxVectorData& cont, unsigned int n_cluster_rdos)
      : xAOD::VariableStruct(cont)
   {
      if (n_cluster_rdos>0) {
         auto *store = cont.getStore();
         assert(store);
         SG::setJaggedVectorData(cont,*store,xAOD::HGTDCluster::rdoListAcc(), n_cluster_rdos, rdoList, rdoListPayload);
         SG::setJaggedVectorData(cont,*store,xAOD::HGTDCluster::totListAcc(), n_cluster_rdos, totList, totListPayload);
      }
   }
   SG::Accessor<SG::JaggedVecElt<Identifier::value_type> >::Elt_span     rdoList;
   SG::Accessor<SG::JaggedVecElt<Identifier::value_type> >::Payload_span rdoListPayload;
   SG::Accessor<SG::JaggedVecElt<int> >::Elt_span     totList;
   SG::Accessor<SG::JaggedVecElt<int> >::Payload_span totListPayload;
};
#endif
