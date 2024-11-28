/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// Header file for class StipClusterTruthDecorator
//
// The algorithm extends xAOD::StripClusterContainer
// with additional decorations associated to truth information
// And stores the results in a TrackMeasurementValidationContainer
// for compatibility with monitoring tools
///////////////////////////////////////////////////////////////////

#ifndef STRIPCLUSTERTRUTHDECORATOR_H
#define STRIPCLUSTERTRUTHDECORATOR_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"
#include "StoreGate/ReadHandleKey.h"

#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/WriteDecorHandle.h"

#include "xAODInDetMeasurement/StripClusterContainer.h"
#include "xAODTracking/TrackMeasurementValidationContainer.h"

#include "Identifier/Identifier.h"

#include "ActsEvent/MeasurementToTruthParticleAssociation.h"

namespace ActsTrk {
  
  class StripClusterTruthDecorator : public AthReentrantAlgorithm {
  public:
    StripClusterTruthDecorator(const std::string &name,ISvcLocator *pSvcLocator);
    virtual ~StripClusterTruthDecorator() = default;
    
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;
    virtual StatusCode finalize() override;
    
    SG::ReadHandleKey<xAOD::StripClusterContainer> m_clustercontainer_key{this,"SiClusterContainer",
      "StripClusters","Input Strip Cluster container"};
    
    SG::ReadHandleKey<MeasurementToTruthParticleAssociation> m_associationMap_key{this,"AssociationMapOut","",
    "Association map between measurements and truth particles"};
    SG::WriteHandleKey<xAOD::TrackMeasurementValidationContainer> m_write_xaod_key{this,"OutputClusterContainer","ITkStripClusters",
    "Output Strip Validation Clusters"};
    
  private:

    bool m_useTruthInfo{true};
  };
}


#endif
