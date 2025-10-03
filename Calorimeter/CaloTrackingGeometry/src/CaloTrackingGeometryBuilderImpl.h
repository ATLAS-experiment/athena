/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CALORIMETER_CALOTRACKINGGEOMETRYBUILDERIMPL_H
#define CALORIMETER_CALOTRACKINGGEOMETRYBUILDERIMPL_H

#include "CaloIdentifier/CaloCell_ID.h"
// Gaudi
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/SystemOfUnits.h"
// Trk
#include "TrkDetDescrInterfaces/ICaloTrackingVolumeBuilder.h"
#include "TrkDetDescrInterfaces/IGeometryBuilderCond.h"
#include "TrkDetDescrInterfaces/ITrackingVolumeArrayCreator.h"
#include "TrkDetDescrInterfaces/ITrackingVolumeCreator.h"
#include "TrkDetDescrUtils/GeometrySignature.h"
#include "TrkDetDescrUtils/LayerIndexSampleMap.h"
#include "TrkGeometry/Material.h"
#include "TrkGeometry/TrackingGeometry.h"
// EnvelopeDefinitionService
#include "SubDetectorEnvelopes/IEnvelopeDefSvc.h"
// CaloDDM
#include "CaloDetDescr/CaloDetDescrManager.h"
// STL
#include <vector>
//
#include "CxxUtils/checker_macros.h"

namespace Trk {
class Layer;
class MagneticFieldProperties;
}  // namespace Trk

namespace Calo {

/** @class CaloTrackingGeometryBuilderImpl

  Retrieves the volume builders from Tile and LAr
  and combines the given volumes to a calorimeter tracking
  geometry.

  This is the underlying implementation logic
  that is shared by the the concrete builders
  */

class CaloTrackingGeometryBuilderImpl : public AthAlgTool {

 public:
  /** Destructor */
  virtual ~CaloTrackingGeometryBuilderImpl();

  /** AlgTool initailize method.*/
  virtual StatusCode initialize() override;

  /** TrackingGeometry Interface method */
  std::unique_ptr<Trk::TrackingGeometry> createTrackingGeometry(Trk::TrackingVolume* innerVol
								, const CaloDetDescrManager* caloDDM
								, const GeoAlignmentStore* geoAlign) const;

  /** The unique signature */
  Trk::GeometrySignature signature() const {
    return Trk::Calo;
  }


 protected:
  /** Constructor */
  CaloTrackingGeometryBuilderImpl(const std::string&, const std::string&,
                                  const IInterface*);

  //!< Helper Tool to create TrackingVolume Arrays
  PublicToolHandle<Trk::ITrackingVolumeArrayCreator> m_trackingVolumeArrayCreator{this, "TrackingVolumeArrayCreator", "Trk::TrackingVolumeArrayCreator/TrackingVolumeArrayCreator"};
  //!< Second helper for volume creation
  PublicToolHandle<Trk::ITrackingVolumeCreator> m_trackingVolumeCreator{this, "TrackingVolumeCreator", "Trk::CylinderVolumeCreator/TrackingVolumeCreator"};
  //!< Volume Builder for the Liquid Argon Calorimeter
  PublicToolHandle<Trk::ICaloTrackingVolumeBuilder> m_lArVolumeBuilder{this, "LArVolumeBuilder", "LAr::LArVolumeBuilder/LArVolumeBuilder"};
  //!< Volume Builder for the Tile Calorimeter
  PublicToolHandle<Trk::ICaloTrackingVolumeBuilder> m_tileVolumeBuilder{this, "TileVolumeBuilder", "Tile::TileVolumeBuilder/TileVolumeBuilder"};

  //!< Material properties
  Trk::Material m_caloMaterial{};
  Trk::Material m_Ar{140.036, 856.32, 39.948, 18., 0.0014};
  Trk::Material m_Al{88.93, 388.62, 26.98, 13., 0.0027};
  Trk::Material m_Scint{424.35, 707.43, 11.16, 5.61, 0.001};  // from G4 definition
  Trk::Material m_crackMaterial{424.35, 707.43, 11.16, 5.61, 0.001}; // Scintillator/Glue (G4 def.)

  DoubleProperty m_caloEnvelope{this, "GapLayerEnvelope", 25 * Gaudi::Units::mm};  //!< Envelope cover for Gap Layers
  // enclosing endcap/cylindervolume
  ServiceHandle<IEnvelopeDefSvc> m_enclosingEnvelopeSvc{this, "EnvelopeDefinitionSvc", "AtlasGeometry_EnvelopeDefSvc"};

  DoubleProperty m_caloDefaultRadius{this, "CalorimeterRadius", 4250.};       //!< the radius if not built from GeoModel
  DoubleProperty m_caloDefaultHalflengthZ{this, "CalorimeterHalflengthZ", 6500.};  //!< the halflength in z if not built from
                                    //!< GeoModel

  BooleanProperty m_indexStaticLayers{this, "IndexStaticLayers", true};  //!< forces robust indexing for layers

  BooleanProperty m_recordLayerIndexCaloSampleMap{this, "RecordLayerIndexCaloSampleMap", true};       //!< for deposition methods
  StringProperty m_layerIndexCaloSampleMapName{this, "LayerIndexCaloSampleMapName", "LayerIndexCaloSampleMap"};  //!< name to record it

  BooleanProperty m_buildMBTS{this, "BuildMBTS", true};  //!< MBTS like detectors
  // //!< MBTS like detectors
  std::vector<double> m_mbtsRadiusGap;    //!< MBTS like detectors
  std::vector<int> m_mbtsPhiSegments;     //!< MBTS like detectors
  std::vector<double> m_mbtsPhiGap;       //!< MBTS like detectors
  std::vector<double> m_mbtsPositionZ;    //!< MBTS like detectors
  std::vector<double> m_mbtsStaggeringZ;  //!< MBTS like detectors

  StringProperty m_entryVolume{this, "EntryVolumeName", "Calo::Container::EntryVolume"};  //!< name of the Calo entrance
  StringProperty m_exitVolume{this, "ExitVolumeName", "Calo::Container"};   //!< name of the Calo container

  /** method to establish a link between the LayerIndex and the CaloCell_ID in
   * an associative container */
  void registerInLayerIndexCaloSampleMap(
      Trk::LayerIndexSampleMap& licsMAp,
      const std::vector<CaloCell_ID::CaloSample>& ccid, const Trk::TrackingVolume& vol,
      int side = 1) const;

  /** method to build enclosed beam pipe volumes */
  std::pair<Trk::TrackingVolume*, Trk::TrackingVolume*> createBeamPipeVolumes(
      const RZPairVector& bpCutouts, float, float, const std::string&,
      float&) const;

};

}  // namespace Calo

#endif  // CALORIMETER_CALOTRACKINGGEOMETRYBUILDERIMPL_H

