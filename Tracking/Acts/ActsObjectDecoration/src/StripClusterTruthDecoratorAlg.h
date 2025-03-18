/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// Header file for class StipClusterTruthDecoratorAlg
//
// The algorithm extends xAOD::StripClusterContainer
// with additional decorations associated to truth information
// And stores the results in a TrackMeasurementValidationContainer
// for compatibility with monitoring tools
///////////////////////////////////////////////////////////////////

#ifndef STRIPCLUSTERTRUTHDECORATORALG_H
#define STRIPCLUSTERTRUTHDECORATORALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"
#include "StoreGate/ReadHandleKey.h"

#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/WriteDecorHandle.h"

#include "xAODInDetMeasurement/StripClusterContainer.h"
#include "xAODTracking/TrackMeasurementValidationContainer.h"
#include "xAODTracking/TrackMeasurementValidationAuxContainer.h"

#include "Identifier/Identifier.h"

#include "ActsEvent/MeasurementToTruthParticleAssociation.h"

namespace ActsTrk {
  
  class StripClusterTruthDecoratorAlg : public AthReentrantAlgorithm {
  public:
    StripClusterTruthDecoratorAlg(const std::string &name,ISvcLocator *pSvcLocator);
    virtual ~StripClusterTruthDecoratorAlg() = default;
    
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;

  private:
    SG::ReadHandleKey<xAOD::StripClusterContainer> m_clustercontainer_key{this,"ClusterContainer", "", "Input Strip Cluster container"};
    SG::ReadHandleKey<MeasurementToTruthParticleAssociation> m_associationMap_key{this,"AssociationMapOut", "", "Association map between measurements and truth particles"};

    SG::WriteHandleKey<xAOD::TrackMeasurementValidationContainer> m_write_xaod_key{this,"MeasurementContainer","", "Output Strip Validation Clusters"};

    SG::WriteDecorHandleKey<xAOD::TrackMeasurementValidationContainer> m_measurement_truth_indices {this, "MeasurementTruthIndices", "truth_index"};
    SG::WriteDecorHandleKey<xAOD::TrackMeasurementValidationContainer> m_measurement_truth_barcodes {this, "MeasurementTruthBarcode", "truth_barcode"};
    
    Gaudi::Property<bool> m_useTruthInfo {this, "UseTruthInfo", true};
  };
}


#endif
