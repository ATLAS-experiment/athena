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


    template<ContainerIdConcept CI = ContainerId::cluster,typename CM=ColumnarModeDefault>
    class EnergyBEAccessor final
    {
      ColumnAccessor<CI,std::vector<float>,CM> m_eAcc;
      ColumnAccessor<CI,uint32_t,CM> m_samplingPatternAcc;

    public:

      typedef CaloSampling::CaloSample CaloSample;

      EnergyBEAccessor (ColumnarTool<CM>& columnarTool)
        : m_eAcc (columnarTool, "e_sampl"), m_samplingPatternAcc (columnarTool, "samplingPattern") {}

      float operator () (ObjectId<CI,CM> object, const unsigned sample) const
      {
        // Newer xAODs have the sampling pattern as an auxiliary
        // variable which is what we are using by default.  For older
        // xAODs we fall back to the xAOD-only implementation, and hope
        // that we are not in columnar mode.
        const auto samplingPattern = m_samplingPatternAcc.isAvailable(object) ? m_samplingPatternAcc(object) : object.getXAODObject().samplingPattern();

        return xAOD::CaloClusterDetails::energyBE(sample, samplingPattern, m_eAcc(object));
      }
    };



    template<ContainerIdConcept CI = ContainerId::cluster,typename CM=ColumnarModeDefault>
    class EtaBEAccessor final
    {
      ColumnAccessor<CI,std::vector<float>,CM> m_eAcc;
      ColumnAccessor<CI,std::vector<float>,CM> m_etaAcc;
      ColumnAccessor<CI,uint32_t,CM> m_samplingPatternAcc;

    public:

      typedef CaloSampling::CaloSample CaloSample;

      EtaBEAccessor (ColumnarTool<CM>& columnarTool)
        : m_eAcc (columnarTool, "e_sampl"), m_etaAcc (columnarTool, "eta_sampl"), m_samplingPatternAcc (columnarTool, "samplingPattern") {}

      float operator () (ObjectId<CI,CM> object, const unsigned sample) const
      {
        // Newer xAODs have the sampling pattern as an auxiliary
        // variable which is what we are using by default.  For older
        // xAODs we fall back to the xAOD-only implementation, and hope
        // that we are not in columnar mode.
        const auto samplingPattern = m_samplingPatternAcc.isAvailable(object) ? m_samplingPatternAcc(object) : object.getXAODObject().samplingPattern();

        return xAOD::CaloClusterDetails::etaBE(sample, samplingPattern, m_eAcc(object), m_etaAcc(object));
      }
    };
  }
}

#endif
