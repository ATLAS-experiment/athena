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
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"

#include "Identifier/Identifier.h"

#include "ActsEvent/MeasurementToTruthParticleAssociation.h"
#include "InDetIdentifier/PixelID.h"
#include "InDetCondTools/ISiLorentzAngleTool.h"

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
    ToolHandle<ISiLorentzAngleTool> m_lorentzAngleTool {this, "LorentzAngleTool", ""};

    SG::ReadHandleKey<xAOD::PixelClusterContainer> m_clustercontainer_key {this,"ClusterContainer", "","Input Pixel Cluster container"};
    SG::ReadHandleKey<ActsTrk::MeasurementToTruthParticleAssociation> m_associationMap_key {this,"AssociationMapOut","", "Association map between measurements and truth particles"};
    SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_pixelDetEleCollKey {this, "PixelDetEleCollKey", "ITkPixelDetectorElementCollection"};
    SG::WriteHandleKey<xAOD::TrackMeasurementValidationContainer> m_write_xaod_key{this,"MeasurementContainer","", "Output Pixel Validation Clusters"};

    // Decorations
    SG::WriteDecorHandleKey<xAOD::TrackMeasurementValidationContainer> m_measurement_truth_indices {this, "MeasurementTruthIndices", "truth_index"};
    SG::WriteDecorHandleKey<xAOD::TrackMeasurementValidationContainer> m_measurement_truth_barcodes {this, "MeasurementTruthBarcode", "truth_barcode"};

    SG::WriteDecorHandleKey<xAOD::TrackMeasurementValidationContainer> m_measurement_detectorElementID {this, "MeasurementDetectorElementID", "detectorElementID"};
    SG::WriteDecorHandleKey<xAOD::TrackMeasurementValidationContainer> m_measurement_waferID {this, "MeasurementWaferID", "waferID"};
    SG::WriteDecorHandleKey<xAOD::TrackMeasurementValidationContainer> m_measurement_bec {this, "MeasurementBEC", "bec"};
    SG::WriteDecorHandleKey<xAOD::TrackMeasurementValidationContainer> m_measurement_layer {this, "MeasurementLayer", "layer"};
    SG::WriteDecorHandleKey<xAOD::TrackMeasurementValidationContainer> m_measurement_sizePhi {this, "MeasurementSizePhi", "sizePhi"};
    SG::WriteDecorHandleKey<xAOD::TrackMeasurementValidationContainer> m_measurement_sizeZ {this, "MeasurementSizeZ", "sizeZ"};
    SG::WriteDecorHandleKey<xAOD::TrackMeasurementValidationContainer> m_measurement_SiWidth {this, "MeasurementSiWidth", "SiWidth"};
    SG::WriteDecorHandleKey<xAOD::TrackMeasurementValidationContainer> m_measurement_eta_module {this, "MeasurementEtaModule", "eta_module"};
    SG::WriteDecorHandleKey<xAOD::TrackMeasurementValidationContainer> m_measurement_phi_module {this, "MeasurementPhiModule", "phi_module"};
    SG::WriteDecorHandleKey<xAOD::TrackMeasurementValidationContainer> m_measurement_omegax {this, "MeasurementOmegaX", "omegax"};
    SG::WriteDecorHandleKey<xAOD::TrackMeasurementValidationContainer> m_measurement_omegay {this, "MeasurementOmegaY", "omegay"};
    SG::WriteDecorHandleKey<xAOD::TrackMeasurementValidationContainer> m_measurement_LorentzShift {this, "MeasurementLorentzShift", "LorentzShift"};
    SG::WriteDecorHandleKey<xAOD::TrackMeasurementValidationContainer> m_measurement_centroid_xphi {this, "MeasurementCentroidXphi", "centroid_xphi"};
    SG::WriteDecorHandleKey<xAOD::TrackMeasurementValidationContainer> m_measurement_centroid_xeta {this, "MeasurementCentroidXeta", "centroid_xeta"};
    SG::WriteDecorHandleKey<xAOD::TrackMeasurementValidationContainer> m_measurement_side {this, "MeasurementSide", "side"};
    
    Gaudi::Property<bool> m_useTruthInfo {this, "UseTruthInfo", true};
    const PixelID *m_PixelHelper {nullptr};
  };
  
}

#endif
