#ifndef ACTSTRK_HGTDAUXDATACACHE_H
#define ACTSTRK_HGTDAUXDATACACHE_H

#include "xAODCore/VariableStruct.h"
#include <xAODInDetMeasurement/HGTDCluster.h>
#include "xAODInDetMeasurement/JaggedVecEltCache.h"

namespace ActsTrk {
struct HgtdAuxDataCache : xAOD::VariableStruct {
   HgtdAuxDataCache(SG::AuxVectorData& cont, unsigned int n_cluster_rdos)
      : xAOD::VariableStruct(cont),
        rdoList(cont, xAOD::HGTDCluster::rdoListAcc(), n_cluster_rdos),
        totList(cont, xAOD::HGTDCluster::totListAcc(), n_cluster_rdos)
   {}
   xAOD::xAODInDetMeasurement::Utilities::JaggedVecEltCache<Identifier::value_type> rdoList;
   xAOD::xAODInDetMeasurement::Utilities::JaggedVecEltCache<int>                    totList;
};
}
#endif
