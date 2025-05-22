/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRY_ACTSTRACKINGGEOMETRYSVC_H
#define ACTSGEOMETRY_ACTSTRACKINGGEOMETRYSVC_H

// ATHENA
#include "AthenaBaseComps/AthService.h"

// PACKAGE
#include "ActsGeometryInterfaces/IActsTrackingGeometrySvc.h"
#include "ActsGeometryInterfaces/IActsTrackingVolumeBuilder.h"
#include "ActsGeometry/ActsLayerBuilder.h"
#include "ActsGeometry/ActsElementVector.h"

// ACTS
#include "Acts/Geometry/CylinderVolumeBuilder.hpp"

// STL
#include <map>

#include <tbb/concurrent_unordered_map.h>

namespace InDetDD {
  class InDetDetectorManager;
  class SiDetectorManager;
  class TRT_DetectorManager;
}

class TRT_ID;
class ActsAlignmentStore;
class BeamPipeDetectorManager;
class HGTD_ID;
class HGTD_DetectorManager;

class ActsDetectorElement;

namespace Acts {

class TrackingGeometry;
class CylinderVolumeHelper;
class ILayerBuilder;

class GeometryIdentifier;
class BinnedSurfaceMaterial;

}


class ActsTrackingGeometrySvc : public extends<AthService, IActsTrackingGeometrySvc> {
public:

  StatusCode initialize() override;

  ActsTrackingGeometrySvc( const std::string& name, ISvcLocator* pSvcLocator );

  std::shared_ptr<const Acts::TrackingGeometry>
  trackingGeometry() override;

  unsigned int populateAlignmentStore(ActsTrk::DetectorAlignStore& store) const override;

  const ActsGeometryContext& getNominalContext() const override;

private:
  ActsLayerBuilder::Config
  makeLayerBuilderConfig(const InDetDD::InDetDetectorManager* manager);

  std::shared_ptr<const Acts::ILayerBuilder>
  makeStrawLayerBuilder(const InDetDD::InDetDetectorManager* manager);

  std::shared_ptr<const Acts::ILayerBuilder>
  makeHGTDLayerBuilder(const HGTD_DetectorManager *manager);
  
  std::shared_ptr<Acts::TrackingVolume>
  makeSCTTRTAssembly(const Acts::GeometryContext& gctx, const Acts::ILayerBuilder& sct_lb,
      const Acts::ILayerBuilder& trt_lb, const Acts::CylinderVolumeHelper& cvh,
      const std::shared_ptr<const Acts::TrackingVolume>& pixel);

  Acts::CylinderVolumeBuilder::Config makeBeamPipeConfig(
      std::shared_ptr<const Acts::CylinderVolumeHelper> cvh) const;

  bool runConsistencyChecks() const;

  ServiceHandle<StoreGateSvc> m_detStore;
  const InDetDD::SiDetectorManager* p_pixelManager{nullptr};
  const InDetDD::SiDetectorManager* p_SCTManager{nullptr};
  const InDetDD::TRT_DetectorManager* p_TRTManager{nullptr};
  const InDetDD::SiDetectorManager* p_ITkPixelManager{nullptr};
  const InDetDD::SiDetectorManager* p_ITkStripManager{nullptr};
  const BeamPipeDetectorManager* p_beamPipeMgr{nullptr};
  const HGTD_DetectorManager* p_HGTDManager{nullptr};

  std::shared_ptr<ActsElementVector> m_elementStore{nullptr};
  std::shared_ptr<const Acts::TrackingGeometry> m_trackingGeometry{nullptr};

  const TRT_ID *m_TRT_idHelper{nullptr};
  const HGTD_ID *m_HGTD_idHelper{nullptr};
  
  ActsGeometryContext m_nominalContext{};
  
  Gaudi::Property<bool> m_useMaterialMap{this, "UseMaterialMap", false, ""};
  Gaudi::Property<bool> m_objDebugOutput{this, "ObjDebugOutput", false, ""};
  Gaudi::Property<std::string> m_materialMapInputFileBase{this, "MaterialMapInputFile", "", ""};
  Gaudi::Property<std::string> m_materialMapCalibFolder{this, "MaterialMapCalibFolder", ".", ""};
  Gaudi::Property<bool> m_buildBeamPipe{this, "BuildBeamPipe", false, ""};

  Gaudi::Property<std::vector<size_t>> m_barrelMaterialBins{this, "BarrelMaterialBins", {10, 10}};
  Gaudi::Property<std::vector<size_t>> m_endcapMaterialBins{this, "EndcapMaterialBins", {5, 20}};
  Gaudi::Property<std::vector<std::string>> m_buildSubdetectors{this, "BuildSubDetectors", {"Pixel", "SCT", "TRT", "Calo", "HGTD", "Muon"}};

  /// the specifications for building additional
  /// passive cylinders in the barrel region:
  /// for each cylinder you want to specify
  /// radius, half length in z and thickness
  Gaudi::Property<std::vector<float>> m_passiveITkInnerPixelBarrelLayerRadii{this, "PassiveITkInnerPixelBarrelLayerRadii", {}};
  Gaudi::Property<std::vector<float>> m_passiveITkInnerPixelBarrelLayerHalflengthZ{this, "PassiveITkInnerPixelBarrelLayerHalflengthZ", {}};
  Gaudi::Property<std::vector<float>> m_passiveITkInnerPixelBarrelLayerThickness{this, "PassiveITkInnerPixelBarrelLayerThickness", {}};

  Gaudi::Property<std::vector<float>> m_passiveITkOuterPixelBarrelLayerRadii{this, "PassiveITkOuterPixelBarrelLayerRadii", {}};
  Gaudi::Property<std::vector<float>> m_passiveITkOuterPixelBarrelLayerHalflengthZ{this, "PassiveITkOuterPixelBarrelLayerHalflengthZ", {}};
  Gaudi::Property<std::vector<float>> m_passiveITkOuterPixelBarrelLayerThickness{this, "PassiveITkOuterPixelBarrelLayerThickness", {}};

  Gaudi::Property<std::vector<float>> m_passiveITkStripBarrelLayerRadii{this, "PassiveITkStripBarrelLayerRadii", {}};
  Gaudi::Property<std::vector<float>> m_passiveITkStripBarrelLayerHalflengthZ{this, "PassiveITkStripBarrelLayerHalflengthZ", {}};
  Gaudi::Property<std::vector<float>> m_passiveITkStripBarrelLayerThickness{this, "PassiveITkStripBarrelLayerThickness", {}};

  BooleanProperty m_runConsistencyChecks{this, "RunConsistencyChecks", 
    false, "Run extra consistency checks w.r.t to Trk::. This is SLOW!"};

  StringProperty m_consistencyCheckOutput{this, "ConsistencyCheckOutput", 
    "", "Output file for geometry debugging, will not write if empty",};

  Gaudi::Property<size_t> m_consistencyCheckPoints{this, "ConsistencyCheckPoints",
    1000, "number of random points for consistency check"};

  ToolHandle<IActsTrackingVolumeBuilder> m_caloVolumeBuilder{this, 
      "CaloVolumeBuilder", "", "CaloVolumeBuilder"};

  ToolHandle<IActsTrackingVolumeBuilder> m_msVolumeBuilder{this, 
      "MSVolumeBuilder", "", "MSVolumeBuilder"};
    /// Define the subdetectors for which the tracking geometry does not expect a valid alignment store
  Gaudi::Property<std::vector<unsigned int>> m_subDetNoAlignProp{this, "NotAlignDetectors", {}};
  std::set<ActsTrk::DetectorType> m_subDetNoAlign{};


};



#endif
