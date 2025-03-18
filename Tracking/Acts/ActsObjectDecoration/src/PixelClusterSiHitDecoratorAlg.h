/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSOBJECTDECORATION_PIXELCLUSTER_SIHIT_DECORATORALG_H
#define ACTSOBJECTDECORATION_PIXELCLUSTER_SIHIT_DECORATORALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "xAODTracking/TrackMeasurementValidationContainer.h"
#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "InDetSimData/InDetSimDataCollection.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "InDetSimEvent/SiHitCollection.h"
#include "InDetIdentifier/PixelID.h"

namespace ActsTrk {
  
  class PixelClusterSiHitDecoratorAlg
    : public AthReentrantAlgorithm {
  public:
    PixelClusterSiHitDecoratorAlg(const std::string &name,ISvcLocator *pSvcLocator);
    virtual ~PixelClusterSiHitDecoratorAlg() = default;
    
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;

  private:
    std::vector<SiHit>
    findAllHitsCompatibleWithCluster( const xAOD::PixelCluster& cluster,
				      const InDetDD::SiDetectorElement&	element,
				      const std::vector<const SiHit*>& sihits,
				      const std::vector< std::vector< int > >& sdoTracks) const;
    
  private:
    SG::ReadHandleKey< xAOD::TrackMeasurementValidationContainer > m_inputMeasurementsKey {this, "Measurements", ""};
    SG::ReadHandleKey< xAOD::PixelClusterContainer > m_inputClustersKey {this, "Clusters", ""};
    SG::ReadHandleKey< InDetSimDataCollection > m_SDOcontainer_key {this, "SDOs", ""};
    SG::ReadHandleKey< SiHitCollection > m_siHitsKey {this, "SiHits", ""};
    SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_pixelDetEleCollKey {this, "PixelDetEleCollKey", "ITkPixelDetectorElementCollection"};
    
    // SDO decorations
    SG::WriteDecorHandleKey< xAOD::TrackMeasurementValidationContainer > m_sdo_words {this, "SdoWords", "sdo_words"};
    SG::WriteDecorHandleKey< xAOD::TrackMeasurementValidationContainer > m_sdo_depositsBarcode {this, "SdoDepositsBarcode", "sdo_depositsBarcode"};
    SG::WriteDecorHandleKey< xAOD::TrackMeasurementValidationContainer > m_sdo_depositsEnergy {this, "SdoDepositsEnergy", "sdo_depositsEnergy"};

    // SiHit decorations
    SG::WriteDecorHandleKey< xAOD::TrackMeasurementValidationContainer > m_sihit_energyDeposit_decor_key {this, "SiHitEnergyDeposit", "sihit_energyDeposit"};
    SG::WriteDecorHandleKey< xAOD::TrackMeasurementValidationContainer > m_sihit_meanTime_decor_key {this, "SiHitMeanTime", "sihit_meanTime"};
    SG::WriteDecorHandleKey< xAOD::TrackMeasurementValidationContainer > m_sihit_barcode_decor_key {this, "SiHitBarcode", "sihit_barcode"};
    SG::WriteDecorHandleKey< xAOD::TrackMeasurementValidationContainer > m_sihit_pdgid_decor_key {this, "SiHitPdgId", "sihit_pdgid"};

    SG::WriteDecorHandleKey< xAOD::TrackMeasurementValidationContainer > m_sihit_startPosX_decor_key {this, "SiHitStartPosX", "sihit_startPosX"};
    SG::WriteDecorHandleKey< xAOD::TrackMeasurementValidationContainer > m_sihit_startPosY_decor_key {this, "SiHitStartPosY", "sihit_startPosY"};
    SG::WriteDecorHandleKey< xAOD::TrackMeasurementValidationContainer > m_sihit_startPosZ_decor_key {this, "SiHitStartPosZ", "sihit_startPosZ"};
    
    SG::WriteDecorHandleKey< xAOD::TrackMeasurementValidationContainer > m_sihit_endPosX_decor_key {this, "SiHitStopPosX", "sihit_endPosX"};
    SG::WriteDecorHandleKey< xAOD::TrackMeasurementValidationContainer > m_sihit_endPosY_decor_key {this, "SiHitStopPosY", "sihit_endPosY"};
    SG::WriteDecorHandleKey< xAOD::TrackMeasurementValidationContainer > m_sihit_endPosZ_decor_key {this, "SiHitStopPosZ", "sihit_endPosZ"};

    Gaudi::Property<bool> m_useSiHitsGeometryMatching {this, "UseSiHitsGeometryMatching", true};

    const PixelID *m_PixelHelper {nullptr};
  };
  
}

#endif
