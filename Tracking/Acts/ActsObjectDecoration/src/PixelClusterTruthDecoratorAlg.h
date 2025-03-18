/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// Header file for class PixelClusterTruthDecoratorAlg
//
// The algorithm extends xAOD::PixelClusterContainer
// with additional decorations associated to truth information
// And stores the results in a TrackMeasurementValidationContainer
// for compatibility with monitoring tools
///////////////////////////////////////////////////////////////////

#ifndef PIXELCLUSTERTRUTHDECORATORALG_H
#define PIXELCLUSTERTRUTHDECORATORALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"
#include "StoreGate/ReadHandleKey.h"

#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/WriteDecorHandle.h"

#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODTracking/TrackMeasurementValidationContainer.h"
#include "xAODTracking/TrackMeasurementValidationAuxContainer.h"

#include "Identifier/Identifier.h"

#include "ActsEvent/MeasurementToTruthParticleAssociation.h"


namespace ActsTrk {
  
  class PixelClusterTruthDecoratorAlg
    : public AthReentrantAlgorithm {
  public:
    PixelClusterTruthDecoratorAlg(const std::string &name,
				  ISvcLocator *pSvcLocator);
    virtual ~PixelClusterTruthDecoratorAlg() = default;
    
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;
    
  private:
    SG::ReadHandleKey<xAOD::PixelClusterContainer> m_clustercontainer_key {this,"ClusterContainer", "","Input Pixel Cluster container"};
    SG::ReadHandleKey<ActsTrk::MeasurementToTruthParticleAssociation> m_associationMap_key {this,"AssociationMapOut","", "Association map between measurements and truth particles"};
    
    SG::WriteHandleKey<xAOD::TrackMeasurementValidationContainer> m_write_xaod_key{this,"MeasurementContainer","", "Output Pixel Validation Clusters"};

    // Decorations
    SG::WriteDecorHandleKey<xAOD::TrackMeasurementValidationContainer> m_measurement_truth_indices {this, "MeasurementTruthIndices", "truth_index"};
    SG::WriteDecorHandleKey<xAOD::TrackMeasurementValidationContainer> m_measurement_truth_barcodes {this, "MeasurementTruthBarcode", "truth_barcode"};

    Gaudi::Property<bool> m_useTruthInfo {this, "UseTruthInfo", true};
  };
  
}

#endif
