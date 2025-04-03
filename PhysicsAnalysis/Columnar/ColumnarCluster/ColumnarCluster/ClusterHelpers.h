/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_CLUSTER_CLUSTER_HELPERS_H
#define COLUMNAR_CLUSTER_CLUSTER_HELPERS_H

#include <ColumnarCore/ColumnAccessor.h>
#include <ColumnarCore/VectorColumn.h>

#include <ColumnarCluster/ClusterDef.h>

#include <xAODCaloEvent/CaloClusterDetails.h>

namespace columnar
{
  namespace ClusterHelpers
  {
    /// @file accessor for variables that share calculations with @ref
    /// xAOD::CaloCluster
    ///
    /// This wraps shared accessor code in the file CaloClusterDetails.h
    /// that has been moved there from inside the `xAOD::CaloCluster`
    /// class.  The wrapping adds the needed columnar accessors to the
    /// call and makes the function behave like a "regular" accessor.


    template<ContainerId CI = ContainerId::cluster,typename CM=ColumnarModeDefault>
    class EnergyBEAccessor final
    {
      ColumnAccessor<CI,std::vector<float>,CM> eAcc;
      ColumnAccessor<CI,uint32_t,CM> samplingPatternAcc;

    public:

      typedef CaloSampling::CaloSample CaloSample;

      EnergyBEAccessor (ColumnarTool<CM>& columnarTool)
        : eAcc (columnarTool, "e_sampl"), samplingPatternAcc (columnarTool, "samplingPattern") {}

      float operator () (ObjectId<CI,CM> object, const unsigned sample) const
      {
        // Newer xAODs have the sampling pattern as an auxiliary
        // variable which is what we are using by default.  For older
        // xAODs we fall back to the xAOD-only implementation, and hope
        // that we are not in columnar mode.
        const auto samplingPattern = samplingPatternAcc.isAvailable(object) ? samplingPatternAcc(object) : object.getXAODObject().samplingPattern();

        return xAOD::CaloClusterDetails::energyBE(sample, samplingPattern, eAcc(object));
      }
    };



    template<ContainerId CI = ContainerId::cluster,typename CM=ColumnarModeDefault>
    class EtaBEAccessor final
    {
      ColumnAccessor<CI,std::vector<float>,CM> eAcc;
      ColumnAccessor<CI,std::vector<float>,CM> etaAcc;
      ColumnAccessor<CI,uint32_t,CM> samplingPatternAcc;

    public:

      typedef CaloSampling::CaloSample CaloSample;

      EtaBEAccessor (ColumnarTool<CM>& columnarTool)
        : eAcc (columnarTool, "e_sampl"), etaAcc (columnarTool, "eta_sampl"), samplingPatternAcc (columnarTool, "samplingPattern") {}

      float operator () (ObjectId<CI,CM> object, const unsigned sample) const
      {
        // Newer xAODs have the sampling pattern as an auxiliary
        // variable which is what we are using by default.  For older
        // xAODs we fall back to the xAOD-only implementation, and hope
        // that we are not in columnar mode.
        const auto samplingPattern = samplingPatternAcc.isAvailable(object) ? samplingPatternAcc(object) : object.getXAODObject().samplingPattern();

        return xAOD::CaloClusterDetails::etaBE(sample, samplingPattern, eAcc(object), etaAcc(object));
      }
    };
  }
}

#endif
