/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRY_ACTSTRACKINGGEOMETRYSVC_H
#define ACTSGEOMETRY_ACTSTRACKINGGEOMETRYSVC_H

// ATHENA
#include "AthenaBaseComps/AthService.h"

// PACKAGE
#include "ActsGeometryInterfaces/ITrackingGeometrySvc.h"
#include "ActsGeometry/DetectorElementToActsGeometryIdMap.h"
#include "ActsGeometryInterfaces/IActsTrackingVolumeBuilder.h"
#include "ActsGeometryInterfaces/IBlueprintNodeBuilder.h"
#include "ActsGeometryInterfaces/IRefineTrackingGeoTool.h"
#include "ActsGeometry/ActsLayerBuilder.h"
#include "ActsGeometry/ActsElementVector.h"

// ACTS
#include "Acts/Geometry/CylinderVolumeBuilder.hpp"

#include "AthDeviceInterfaces/IMemoryResourceTool.h"
#include "GaudiKernel/ToolHandle.h"

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
class BlueprintNode;

}


namespace ActsTrk{
class TrackingGeometrySvc : public extends<AthService, ActsTrk::ITrackingGeometrySvc> {
public:

  ~TrackingGeometrySvc() override;
  StatusCode initialize() override;
  StatusCode finalize() override;

  TrackingGeometrySvc( const std::string& name, ISvcLocator* pSvcLocator );
    /** @copydoc ActsTrk::ITrackingGeometrySvc::trackingGeometry */
  std::shared_ptr<const Acts::TrackingGeometry> trackingGeometry() const override;
  /** @copydoc ActsTrk::ITrackingGeometrySvc::populateAlignmentStore */
  unsigned int populateAlignmentStore(ActsTrk::DetectorAlignStore& store) const override;

  /** @copydoc ActsTrk::ITrackingGeometrySvc::getNominalContext */
  const ActsTrk::GeometryContext& getNominalContext() const override;
  /** @copydoc ActsTrk::ITrackingGeometrySvc::getEnvelope */
  const Acts::TrackingVolume* getEnvelope(const ActsTrk::SystemEnvelope envType) const override;
  /** @copydoc ActsTrk::ITrackingGeometrySvc::surfaceIdMap */
  virtual const ActsTrk::DetectorElementToActsGeometryIdMap* surfaceIdMap() const override;

  /** @brief Returns the Detray geometry converted from the Acts::TrackingGeometry,
             or nullptr if none was built. The service stays the owner: the
             geometry is released in finalize(), while the memory resource it
             was allocated from is still around.
             Only populated when the BuildDetrayGeometry property is enabled. */
  const traccc::host_detector* detrayGeometry() const override {
    return m_detrayGeometry.get();
  }

private:
  /** @brief Creates and popules the DetectorElement -> Acts::Surface geo identifier map from the geometry service */
  std::unique_ptr<ActsTrk::DetectorElementToActsGeometryIdMap> createDetectorElementToGeoIdMap() const;

  /** @brief Converts the built Acts::TrackingGeometry into a Detray geometry and stores it in m_detrayGeometry */
  StatusCode buildDetrayGeometry();

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
  
  ActsTrk::GeometryContext m_nominalContext{};
  
  Gaudi::Property<bool> m_useMaterialMap{this, "UseMaterialMap", false, ""};
  Gaudi::Property<bool> m_objDebugOutput{this, "ObjDebugOutput", false, ""};
  Gaudi::Property<bool> m_keepGoingOnMaterialMergeFailure{this, "KeepGoingOnMaterialMergeFailure", false, ""};
  Gaudi::Property<std::string> m_materialMapInputFileBase{this, "MaterialMapInputFile", "", ""};
  Gaudi::Property<std::string> m_materialMapCalibFolder{this, "MaterialMapCalibFolder", ".", ""};
  Gaudi::Property<bool> m_buildBeamPipe{this, "BuildBeamPipe", false, ""};

  /// @brief Print the assembled tracking geometry after building
  Gaudi::Property<bool> m_printGeo{this, "printGeometry", false};

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

  ToolHandle<IActsTrackingVolumeBuilder> m_caloVolumeBuilder{this, "CaloVolumeBuilder", ""};
  
  ToolHandleArray<ActsTrk::IBlueprintNodeBuilder> m_blueprintNodeBuilders{this, "BlueprintNodeBuilders", {}};

  ToolHandleArray<ActsTrk::IRefineTrackingGeoTool> m_refineVisitors{this, "RefinementTools", {}};
  /// Define the subdetectors for which the tracking geometry does not expect a valid alignment store
  Gaudi::Property<std::vector<unsigned int>> m_subDetNoAlignProp{this, "NotAlignDetectors", {}};
  std::set<ActsTrk::DetectorType> m_subDetNoAlign{};

  Gaudi::Property<bool> m_useBlueprint{this, "UseBlueprint", false, "Use the new Blueprint API for geometry construction"};

  Gaudi::Property<bool> m_buildDetrayGeometry{this, "BuildDetrayGeometry", false,
      "Convert the constructed Acts::TrackingGeometry into a Detray geometry."};

  Gaudi::Property<bool> m_checkDetrayGeometry{this, "CheckDetrayGeometry", false,
      "Run the Detray consistency check on the converted geometry. "
      "Only used when BuildDetrayGeometry is enabled."};

  Gaudi::Property<bool> m_buildDetraySurfaceGrids{this, "BuildDetraySurfaceGrids", true,
      "Build the Detray surface grids from the volume navigation policies. "
      "Only used when BuildDetrayGeometry is enabled."};

  Gaudi::Property<bool> m_buildDetrayMaterial{this, "BuildDetrayMaterial", true,
      "Build the Detray material from the volume navigation policies. "
      "Only used when BuildDetrayGeometry is enabled."};    

  /// Tool providing the memory resource that the Detray geometry is allocated
  /// from. The detector keeps referring to that resource for its deallocations,
  /// so the tool has to outlive m_detrayGeometry. Only used, and only required
  /// to be set, when BuildDetrayGeometry is enabled.
  ToolHandle<AthDevice::IMemoryResourceTool> m_hostMR{this, "HostMR", "",
      "Host memory resource tool used for the Detray geometry allocations. The "
      "conversion runs entirely on the host, so the resource has to be host "
      "accessible: a plain host or a managed/shared one, not a device one."};
  
  Gaudi::Property<std::string> m_blueprintGraphviz{this, "BlueprintGraphviz", 
                                                   "", "Write the blueprint graph to a file. No file will be written if empty"};
  Gaudi::Property<bool> m_doEndcapLayerMerging{this, "DoEndcapLayerMerging", true, "Merge overlapping endcap layers in z"};

  /// controls how many bins are created for the sensitive surface grid.
  /// 1 results in the same number of bins as there are surfaces per layer in each dimension.
  /// using a higher number will reduce the number of surfaces per bin, thus speeding up navigation, but increasing memory consumption.
  Gaudi::Property<double> m_numberOfBinsFactor{this, "NumberOfBinsFactor", 5.0};

  /// Most bins the surface array lookup serves on each side of the crossing,
  /// as {phi, z} for the barrel and {r, phi} for the endcap. The lookup sizes
  /// the window from the crossing angle and the layer thickness, so these
  /// values only cap it. The defaults cover the steepest crossing for
  /// |z0| < 200 mm at NumberOfBinsFactor = 5.
  Gaudi::Property<std::vector<unsigned int>> m_itkPixelInnerBarrelNeighborWindow{
      this, "ITkPixelInnerBarrelNeighborWindow", {1, 8},
      "Neighbor window bound {phi, z} of the ITk inner pixel barrel layers"};
  Gaudi::Property<std::vector<unsigned int>> m_itkPixelInnerEndcapNeighborWindow{
      this, "ITkPixelInnerEndcapNeighborWindow", {2, 1},
      "Neighbor window bound {r, phi} of the ITk inner pixel endcap layers"};
  Gaudi::Property<std::vector<unsigned int>> m_itkPixelOuterBarrelNeighborWindow{
      this, "ITkPixelOuterBarrelNeighborWindow", {1, 3},
      "Neighbor window bound {phi, z} of the ITk outer pixel barrel layers"};
  Gaudi::Property<std::vector<unsigned int>> m_itkPixelOuterEndcapNeighborWindow{
      this, "ITkPixelOuterEndcapNeighborWindow", {4, 1},
      "Neighbor window bound {r, phi} of the ITk outer pixel endcap and inclined layers"};
  Gaudi::Property<std::vector<unsigned int>> m_itkStripBarrelNeighborWindow{
      this, "ITkStripBarrelNeighborWindow", {1, 4},
      "Neighbor window bound {phi, z} of the ITk strip barrel layers"};
  Gaudi::Property<std::vector<unsigned int>> m_itkStripEndcapNeighborWindow{
      this, "ITkStripEndcapNeighborWindow", {2, 1},
      "Neighbor window bound {r, phi} of the ITk strip endcap layers"};

  /// Extra cells per direction and axis that each surface fills around its footprint
  Gaudi::Property<unsigned int> m_surfaceArrayOverfill{this, "SurfaceArrayOverfill", 0,
      "Extra cells per direction and axis that each surface fills in the surface arrays"};
  
  /// Special treatment for hgtd layers as well.
  Gaudi::Property<double> m_numberOfHgtdBinsFactor{this, "NumberOfHgtdBinsFactor", 1.0};

  std::unique_ptr<const ActsTrk::DetectorElementToActsGeometryIdMap> m_detIdMap{};

  std::unique_ptr<traccc::host_detector> m_detrayGeometry;
};

}

#endif
