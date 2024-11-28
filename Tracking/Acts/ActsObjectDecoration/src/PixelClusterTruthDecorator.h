/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// Header file for class PixelClusterTruthDecorator
//
// The algorithm extends xAOD::PixelClusterContainer
// with additional decorations associated to truth information
// And stores the results in a TrackMeasurementValidationContainer
// for compatibility with monitoring tools
///////////////////////////////////////////////////////////////////

#ifndef PIXELCLUSTERTRUTHDECORATOR_H
#define PIXELCLUSTERTRUTHDECORATOR_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"
#include "StoreGate/ReadHandleKey.h"

#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/WriteDecorHandle.h"

#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODTracking/TrackMeasurementValidationContainer.h"

#include "Identifier/Identifier.h"

#include "ActsEvent/MeasurementToTruthParticleAssociation.h"


namespace ActsTrk {
  
  class PixelClusterTruthDecorator : public AthReentrantAlgorithm {
  public:
    PixelClusterTruthDecorator(const std::string &name,ISvcLocator *pSvcLocator);
    virtual ~PixelClusterTruthDecorator() = default;
    
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;
    virtual StatusCode finalize() override;
    
    SG::ReadHandleKey<xAOD::PixelClusterContainer> m_clustercontainer_key{this,"SiClusterContainer",
      "PixelClusters","Input Pixel Cluster container"};
    SG::ReadHandleKey<MeasurementToTruthParticleAssociation> m_associationMap_key{this,"AssociationMapOut","",
      "Association map between measurements and truth particles"};
    
    SG::WriteHandleKey<xAOD::TrackMeasurementValidationContainer> m_write_xaod_key{this,"OutputClusterContainer","ITkPixelClusters",
      "Output Pixel Validation Clusters"};
    
    
  private:
    bool m_useTruthInfo{true};
    
  };

  
  
}


#endif
