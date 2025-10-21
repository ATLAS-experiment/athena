/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// This absolutely needs to go first to ensure Eigen plugin is loaded
#include "GeoPrimitives/GeoPrimitives.h"
//

#include "ActsGeometry/ActsTrackingGeometrySvc.h"

#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"
// ATHENA
#include "GaudiKernel/EventContext.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "PathResolver/PathResolver.h"
#include "InDetIdentifier/TRT_ID.h"
#include "InDetReadoutGeometry/SiDetectorManager.h"
#include "HGTD_ReadoutGeometry/HGTD_DetectorManager.h"
#include "StoreGate/StoreGateSvc.h"
#include "TRT_ReadoutGeometry/TRT_DetectorManager.h"
#include "BeamPipeGeoModel/BeamPipeDetectorManager.h"
#include "GeoModelKernel/GeoTube.h"

// ACTS
#include "Acts/ActsVersion.hpp"
#include "Acts/Geometry/Blueprint.hpp"
#include "Acts/Geometry/ContainerBlueprintNode.hpp"
#include "Acts/Geometry/CylinderVolumeBounds.hpp"
#include "Acts/Geometry/CylinderVolumeBuilder.hpp"
#include "Acts/Geometry/CylinderVolumeHelper.hpp"
#include "Acts/Geometry/ITrackingVolumeBuilder.hpp"
#include "Acts/Geometry/LayerArrayCreator.hpp"
#include "Acts/Geometry/LayerCreator.hpp"
#include "Acts/Geometry/SurfaceArrayCreator.hpp"
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "Acts/Geometry/TrackingGeometryBuilder.hpp"
#include "Acts/Geometry/TrackingVolume.hpp"
#include "Acts/Geometry/TrackingVolumeArrayCreator.hpp"
#include "Acts/Utilities/Logger.hpp"
#include "Acts/Definitions/Units.hpp"
#include "Acts/Geometry/PassiveLayerBuilder.hpp"
#include <ActsPlugins/Json/JsonMaterialDecorator.hpp>
#include <ActsPlugins/Json/MaterialMapJsonConverter.hpp>
#include <Acts/Surfaces/PlanarBounds.hpp>
#include <Acts/Surfaces/AnnulusBounds.hpp>
#include <Acts/Surfaces/DiscSurface.hpp>
#include <Acts/Surfaces/LineSurface.hpp>
#include <Acts/Surfaces/RectangleBounds.hpp>
#include <Acts/Visualization/ObjVisualization3D.hpp>

// PACKAGE
#include "ActsGeometryInterfaces/IDetectorElement.h"
#include "ActsGeometryInterfaces/GeometryContext.h"
#include "ActsGeometry/ActsLayerBuilder.h"
#include "ActsGeometry/ActsStrawLayerBuilder.h"
#include "ActsGeometry/ActsHGTDLayerBuilder.h"
#include "ActsInterop/IdentityHelper.h"
#include "ActsInterop/Logger.h"
#include "ActsInterop/LoggerUtils.h"

#include <Acts/Utilities/AxisDefinitions.hpp>
#include <limits>
#include <random>
#include <stdexcept>

using namespace Acts::UnitLiterals;
using namespace ActsTrk;
ActsTrackingGeometrySvc::ActsTrackingGeometrySvc(const std::string &name,
                                                 ISvcLocator *svc)
    : base_class(name, svc),
      m_detStore("StoreGateSvc/DetectorStore", name),
      m_elementStore (std::make_shared<ActsElementVector>())
{
}

StatusCode ActsTrackingGeometrySvc::initialize() {
  ATH_MSG_INFO(name() << " is initializing");
  for (unsigned int skipAlign : m_subDetNoAlignProp) {
    try {
        m_subDetNoAlign.insert(static_cast<DetectorType>(skipAlign));
    } catch (...) {
        ATH_MSG_FATAL("Failed to interpret " << m_subDetNoAlignProp << " as ActsDetectorElements");
        return StatusCode::FAILURE;
    }
  }
  ATH_CHECK(m_caloVolumeBuilder.retrieve(EnableTool{!m_caloVolumeBuilder.empty()}));
  ATH_CHECK(m_msVolumeBuilder.retrieve(EnableTool{!m_msVolumeBuilder.empty()}));

 
  // FIXME: ActsCaloTrackingVolumeBuilder holds ReadHandle to
  // CaloDetDescrManager. Hopefully this service is never called before that
  // object is available.
  m_autoRetrieveTools = false;
  m_checkToolDeps = false;

  ATH_MSG_INFO("ACTS version is: v"
               << Acts::VersionMajor << "." << Acts::VersionMinor << "."
               << Acts::VersionPatch << " [" << Acts::CommitHash.value_or("unknown hash") << "]");

  // load which subdetectors to build from property
  std::set<std::string> buildSubdet(m_buildSubdetectors.begin(),
                                    m_buildSubdetectors.end());
  ATH_MSG_INFO("Configured to build " << buildSubdet.size()
                                      << " subdetectors:");
  for (const auto &s : buildSubdet) {
    ATH_MSG_INFO(" - " << s);
  }

  ATH_MSG_DEBUG("Loading detector manager(s)");
  if (buildSubdet.find("Pixel") != buildSubdet.end()) {
    ATH_CHECK(m_detStore->retrieve(p_pixelManager, "Pixel"));
  }
  if (buildSubdet.find("SCT") != buildSubdet.end()) {
    ATH_CHECK(m_detStore->retrieve(p_SCTManager, "SCT"));
  }
  if (buildSubdet.find("TRT") != buildSubdet.end()) {
    ATH_CHECK(m_detStore->retrieve(p_TRTManager, "TRT"));
    ATH_CHECK(m_detStore->retrieve(m_TRT_idHelper, "TRT_ID"));
  }
  if (buildSubdet.find("ITkPixel") != buildSubdet.end()) {
    ATH_CHECK(m_detStore->retrieve(p_ITkPixelManager, "ITkPixel"));
  }
  if (buildSubdet.find("ITkStrip") != buildSubdet.end()) {
    ATH_CHECK(m_detStore->retrieve(p_ITkStripManager, "ITkStrip"));
  }
  if (buildSubdet.find("HGTD") != buildSubdet.end()) {
    ATH_CHECK(m_detStore->retrieve(p_HGTDManager, "HGTD"));
    ATH_CHECK(m_detStore->retrieve(m_HGTD_idHelper, "HGTD_ID"));
  }

  if(m_buildBeamPipe) {
    ATH_CHECK(m_detStore->retrieve(p_beamPipeMgr, "BeamPipe"));
  }

  // Consistency check on the size vectors for passive layers
  if (m_passiveITkInnerPixelBarrelLayerRadii.size() != m_passiveITkInnerPixelBarrelLayerHalflengthZ.size() ||
    m_passiveITkInnerPixelBarrelLayerHalflengthZ.size() != m_passiveITkInnerPixelBarrelLayerThickness.size()) {
        ATH_MSG_FATAL("Consistency check for ITk inner pixel barrel passive layer construction failed. Please check your inputs! ");
        return StatusCode::FAILURE;
  }

  if (m_passiveITkOuterPixelBarrelLayerRadii.size() != m_passiveITkOuterPixelBarrelLayerHalflengthZ.size() ||
    m_passiveITkOuterPixelBarrelLayerHalflengthZ.size() != m_passiveITkOuterPixelBarrelLayerThickness.size()) {
        ATH_MSG_FATAL("Consistency check for ITk outer pixel barrel passive layer construction failed. Please check your inputs! ");
        return StatusCode::FAILURE;
  }

  if (m_passiveITkStripBarrelLayerRadii.size() != m_passiveITkStripBarrelLayerHalflengthZ.size() ||
    m_passiveITkStripBarrelLayerHalflengthZ.size() != m_passiveITkStripBarrelLayerThickness.size()) {
        ATH_MSG_FATAL("Consistency check for ITk strip barrel passive layer construction failed. Please check your inputs! ");
        return StatusCode::FAILURE;
  }

  if (m_useBlueprint) {


    ATH_MSG_INFO("Using Blueprint API for geometry construction");
    std::set<std::string> buildSubdet(m_buildSubdetectors.begin(),
                                    m_buildSubdetectors.end());

    ATH_CHECK(m_blueprintNodeBuilders.retrieve());

    using enum Acts::AxisDirection;
  
    std::vector<ActsTrk::IBlueprintNodeBuilder*> ptrBuilders;
    std::transform(m_blueprintNodeBuilders.begin(), m_blueprintNodeBuilders.end(),
               std::back_inserter(ptrBuilders),
               [](ToolHandle<ActsTrk::IBlueprintNodeBuilder>& b) { return b.get(); });

    auto logger = makeActsAthenaLogger(this, std::string("Blueprint"), std::string("ActsTGSvc"));
    
    Acts::Experimental::Blueprint::Config cfg;
    cfg.envelope[AxisZ] = {20_mm, 20_mm};
    cfg.envelope[AxisR] = {0_mm, 20_mm};

    auto blueprint = std::make_unique<Acts::Experimental::Blueprint>(cfg);

    auto& root = blueprint->addCylinderContainer("Detector", AxisZ);
    //The starting top node 
    std::shared_ptr<Acts::Experimental::BlueprintNode> currentTop{nullptr};

    for (auto& builder : ptrBuilders) {
      currentTop = builder->buildBlueprintNode(getNominalContext().context(), std::move(currentTop));
      
    }

    root.addChild(std::move(currentTop));
    
    m_trackingGeometry = blueprint->construct(
      {}, getNominalContext().context(), *logger->clone(std::nullopt, Acts::Logging::DEBUG));

    if (m_objDebugOutput) {
    Acts::ObjVisualization3D vis;
    m_trackingGeometry->visualize(vis, getNominalContext().context(), {.visible = false},
                                {.visible = false}, {.visible = true});
    vis.write("blueprint_sensitive.obj");
    vis.clear();

    m_trackingGeometry->visualize(vis, getNominalContext().context(), {.visible = true},
                                {.visible = false}, {.visible = false});
    vis.write("blueprint_volume.obj");
    vis.clear();

    m_trackingGeometry->visualize(vis, getNominalContext().context(), {.visible = false},
                                {.visible = true}, {.visible = false});
    vis.write("blueprint_portals.obj");


  }

    return StatusCode::SUCCESS;
  }

  ATH_MSG_DEBUG("Setting up ACTS geometry helpers");

  Acts::LayerArrayCreator::Config lacCfg;
  auto layerArrayCreator = std::make_shared<const Acts::LayerArrayCreator>(
      lacCfg, makeActsAthenaLogger(this, std::string("LayArrCrtr"), std::string("ActsTGSvc")));

  Acts::TrackingVolumeArrayCreator::Config tvcCfg;
  auto trackingVolumeArrayCreator =
      std::make_shared<const Acts::TrackingVolumeArrayCreator>(
          tvcCfg, makeActsAthenaLogger(this, std::string("TrkVolArrCrtr"), std::string("ActsTGSvc")));

  Acts::CylinderVolumeHelper::Config cvhConfig;
  cvhConfig.layerArrayCreator = layerArrayCreator;
  cvhConfig.trackingVolumeArrayCreator = trackingVolumeArrayCreator;

  auto cylinderVolumeHelper =
      std::make_shared<const Acts::CylinderVolumeHelper>(
          cvhConfig, makeActsAthenaLogger(this, std::string("CylVolHlpr"), std::string("ActsTGSvc")));

  Acts::TrackingGeometryBuilder::Config tgbConfig;
  tgbConfig.trackingVolumeHelper = cylinderVolumeHelper;

  if (m_useMaterialMap) {
    std::shared_ptr<const Acts::IMaterialDecorator> matDeco = nullptr;

    std::string matFileFullPath = PathResolverFindCalibFile(m_materialMapCalibFolder.value()+"/"+m_materialMapInputFileBase.value());
    if (matFileFullPath.empty()) {
      ATH_MSG_ERROR( "Material Map Input File " << m_materialMapCalibFolder.value() << "/" << m_materialMapInputFileBase.value() << " not found.");
      return StatusCode::FAILURE;
    }
    ATH_MSG_INFO("Configured to use material input: " << matFileFullPath);

    if (matFileFullPath.find(".json") != std::string::npos) {
      // Set up the converter first
      Acts::MaterialMapJsonConverter::Config jsonGeoConvConfig;
      // Set up the json-based decorator
      matDeco = std::make_shared<const Acts::JsonMaterialDecorator>(
	   jsonGeoConvConfig, matFileFullPath, ActsTrk::actsLevelVector(msg().level()));
    }
    tgbConfig.materialDecorator = matDeco;
  }

  std::array<double, 2> sctECEnvelopeZ{20_mm, 20_mm};

  try {
    // BeamPipe
    if(m_buildBeamPipe) {
      tgbConfig.trackingVolumeBuilders.push_back([&](const auto &gctx,
                                                     const auto &inner,
                                                     const auto &) {

        Acts::CylinderVolumeBuilder::Config bpvConfig =
          makeBeamPipeConfig(cylinderVolumeHelper);

        Acts::CylinderVolumeBuilder beamPipeVolumeBuilder {
          bpvConfig, makeActsAthenaLogger(this, std::string("BPVolBldr"), std::string("ActsTGSvc"))};

        return beamPipeVolumeBuilder.trackingVolume(gctx, inner);
      });
    }



    // PIXEL
    if (buildSubdet.count("Pixel") > 0) {
      tgbConfig.trackingVolumeBuilders.push_back([&](const auto &gctx,
                                                     const auto &inner,
                                                     const auto &) {
        auto cfg = makeLayerBuilderConfig(p_pixelManager);
        cfg.mode = ActsLayerBuilder::Mode::Pixel;
        auto lb = std::make_shared<ActsLayerBuilder>(
            cfg, makeActsAthenaLogger(this, std::string("PixelGMSLayBldr"), std::string("ActsTGSvc")));
        Acts::CylinderVolumeBuilder::Config cvbConfig;
        cvbConfig.layerEnvelopeR = {3_mm, 3_mm};
        cvbConfig.layerEnvelopeZ = 1_mm;
        cvbConfig.trackingVolumeHelper = cylinderVolumeHelper;
        cvbConfig.volumeName = "Pixel";
        cvbConfig.layerBuilder = lb;
        cvbConfig.buildToRadiusZero = !m_buildBeamPipe;

        Acts::CylinderVolumeBuilder cvb(
            cvbConfig, makeActsAthenaLogger(this, std::string("CylVolBldr"), std::string("ActsTGSvc")));

        return cvb.trackingVolume(gctx, inner);
      });
    }

    // ITK PIXEL
    if (buildSubdet.count("ITkPixel") > 0) {
      tgbConfig.trackingVolumeBuilders.push_back(
          [&](const auto &gctx, const auto &inner, const auto &) {
            auto cfg = makeLayerBuilderConfig(p_ITkPixelManager);
            cfg.mode = ActsLayerBuilder::Mode::ITkPixelInner;
            cfg.objDebugOutput = m_objDebugOutput;
            cfg.doEndcapLayerMerging = true;
            cfg.passiveBarrelLayerRadii = m_passiveITkInnerPixelBarrelLayerRadii;
            cfg.passiveBarrelLayerHalflengthZ = m_passiveITkInnerPixelBarrelLayerHalflengthZ;
            cfg.passiveBarrelLayerThickness = m_passiveITkInnerPixelBarrelLayerThickness;
            auto lb = std::make_shared<ActsLayerBuilder>(
                cfg, makeActsAthenaLogger(this, std::string("ITkPxInLb"), std::string("ActsTGSvc")));

            Acts::CylinderVolumeBuilder::Config cvbConfig;
            cvbConfig.layerEnvelopeR = {5_mm, 5_mm};
            cvbConfig.layerEnvelopeZ = 1_mm;
            cvbConfig.trackingVolumeHelper = cylinderVolumeHelper;
            cvbConfig.volumeName = "ITkPixelInner";
            cvbConfig.layerBuilder = lb;
            cvbConfig.buildToRadiusZero = !m_buildBeamPipe;

            Acts::CylinderVolumeBuilder cvb(
                cvbConfig,
                makeActsAthenaLogger(this, std::string("CylVolBldr"), std::string("ActsTGSvc")));

            return cvb.trackingVolume(gctx, inner);
          });

      tgbConfig.trackingVolumeBuilders.push_back(
          [&](const auto &gctx, const auto &inner, const auto &) {
            auto cfg = makeLayerBuilderConfig(p_ITkPixelManager);
            cfg.mode = ActsLayerBuilder::Mode::ITkPixelOuter;
            cfg.objDebugOutput = m_objDebugOutput;
            cfg.doEndcapLayerMerging = false;
            cfg.passiveBarrelLayerRadii = m_passiveITkOuterPixelBarrelLayerRadii;
            cfg.passiveBarrelLayerHalflengthZ = m_passiveITkOuterPixelBarrelLayerHalflengthZ;
            cfg.passiveBarrelLayerThickness = m_passiveITkOuterPixelBarrelLayerThickness;
            auto lb = std::make_shared<ActsLayerBuilder>(
                cfg, makeActsAthenaLogger(this, std::string("ITkPxOtLb"), std::string("ActsTGSvc")));

            Acts::CylinderVolumeBuilder::Config cvbConfig;
            cvbConfig.layerEnvelopeR = {5_mm, 5_mm};
            cvbConfig.layerEnvelopeZ = 1_mm;
            cvbConfig.trackingVolumeHelper = cylinderVolumeHelper;
            cvbConfig.volumeName = "ITkPixelOuter";
            cvbConfig.layerBuilder = lb;
            cvbConfig.buildToRadiusZero = false;
            cvbConfig.checkRingLayout = true;
            cvbConfig.ringTolerance = 10_mm;

            Acts::CylinderVolumeBuilder cvb(
                cvbConfig,
                makeActsAthenaLogger(this, std::string("CylVolBldr"), std::string("ActsTGSvc")));

            return cvb.trackingVolume(gctx, inner);
          });
    }

    // ITK STRIP
    if (buildSubdet.count("ITkStrip") > 0) {
      tgbConfig.trackingVolumeBuilders.push_back(
          [&](const auto &gctx, const auto &inner, const auto &) {
            auto cfg = makeLayerBuilderConfig(p_ITkStripManager);
            cfg.mode = ActsLayerBuilder::Mode::ITkStrip;
            cfg.objDebugOutput = m_objDebugOutput;
            cfg.passiveBarrelLayerRadii = m_passiveITkStripBarrelLayerRadii;
            cfg.passiveBarrelLayerHalflengthZ = m_passiveITkStripBarrelLayerHalflengthZ;
            cfg.passiveBarrelLayerThickness = m_passiveITkStripBarrelLayerThickness;
            auto lb = std::make_shared<ActsLayerBuilder>(
                cfg, makeActsAthenaLogger(this, std::string("ITkStripLB"), std::string("ActsTGSvc")));

            Acts::CylinderVolumeBuilder::Config cvbConfig;
            cvbConfig.layerEnvelopeR = {5_mm, 5_mm};
            cvbConfig.layerEnvelopeZ = 1_mm;
            cvbConfig.trackingVolumeHelper = cylinderVolumeHelper;
            cvbConfig.volumeName = "ITkStrip";
            cvbConfig.layerBuilder = lb;
            cvbConfig.buildToRadiusZero = 
              buildSubdet.count("ITkPixel") == 0 && !m_buildBeamPipe;

            Acts::CylinderVolumeBuilder cvb(
                cvbConfig,
                makeActsAthenaLogger(this, std::string("CylVolBldr"), std::string("ActsTGSvc")));

            return cvb.trackingVolume(gctx, inner);
          });
    }

    bool buildSCT = buildSubdet.count("SCT") > 0;
    bool buildTRT = buildSubdet.count("TRT") > 0;

    if (buildSCT && buildTRT) {
      // building both we need to take care
      tgbConfig.trackingVolumeBuilders.push_back(
          [&](const auto &gctx, const auto &inner, const auto &) {
            auto cfg = makeLayerBuilderConfig(p_SCTManager);
            cfg.mode = ActsLayerBuilder::Mode::SCT;
            cfg.endcapEnvelopeZ = sctECEnvelopeZ;
            auto sct_lb = std::make_shared<ActsLayerBuilder>(
                cfg, makeActsAthenaLogger(this, std::string("SCTGMSLayBldr"), std::string("ActsTGSvc")));

            auto trt_lb = makeStrawLayerBuilder(p_TRTManager);

            return makeSCTTRTAssembly(gctx, *sct_lb, *trt_lb,
                                      *cylinderVolumeHelper, inner);
          });

    } else if (buildSCT) {
      tgbConfig.trackingVolumeBuilders.push_back(
          [&](const auto &gctx, const auto &inner, const auto &) {
            auto lbCfg = makeLayerBuilderConfig(p_SCTManager);
            lbCfg.mode = ActsLayerBuilder::Mode::SCT;
            lbCfg.endcapEnvelopeZ = sctECEnvelopeZ;
            auto lb = std::make_shared<ActsLayerBuilder>(
                lbCfg,
                makeActsAthenaLogger(this, std::string("SCTGMSLayBldr"), std::string("ActsTGSvc")));

            Acts::CylinderVolumeBuilder::Config cvbConfig;
            cvbConfig.layerEnvelopeR = {5_mm, 5_mm};
            cvbConfig.layerEnvelopeZ = 2_mm;
            cvbConfig.trackingVolumeHelper = cylinderVolumeHelper;
            cvbConfig.volumeName = "SCT";
            cvbConfig.layerBuilder = lb;
            cvbConfig.buildToRadiusZero = false;

            Acts::CylinderVolumeBuilder cvb(
                cvbConfig,
                makeActsAthenaLogger(this, std::string("SCTCylVolBldr"), std::string("ActsTGSvc")));

            return cvb.trackingVolume(gctx, inner);
          });
    } else if (buildTRT) {
      tgbConfig.trackingVolumeBuilders.push_back(
          [&](const auto &gctx, const auto &inner, const auto &) {
            auto lb = makeStrawLayerBuilder(p_TRTManager);
            Acts::CylinderVolumeBuilder::Config cvbConfig;
            cvbConfig.layerEnvelopeR = {5_mm, 5_mm};
            cvbConfig.layerEnvelopeZ = 2_mm;
            cvbConfig.trackingVolumeHelper = cylinderVolumeHelper;
            cvbConfig.volumeName = "TRT";
            cvbConfig.layerBuilder = lb;
            cvbConfig.buildToRadiusZero = false;

            Acts::CylinderVolumeBuilder cvb(
                cvbConfig,
                makeActsAthenaLogger(this, std::string("TRTCylVolBldr"), std::string("ActsTGSvc")));

            return cvb.trackingVolume(gctx, inner);
          });
    }

    //HGTD
    if(buildSubdet.count("HGTD") > 0) {
      tgbConfig.trackingVolumeBuilders.push_back(
          [&](const auto &gctx, const auto &inner, const auto &) {
            auto lb = makeHGTDLayerBuilder(p_HGTDManager); //using ActsHGTDLayerBuilder
            Acts::CylinderVolumeBuilder::Config cvbConfig;
            cvbConfig.layerEnvelopeR = {5_mm, 5_mm};
            cvbConfig.layerEnvelopeZ = 1_mm;
            cvbConfig.trackingVolumeHelper = cylinderVolumeHelper;
            cvbConfig.volumeName = "HGTD";
            cvbConfig.layerBuilder = lb;
            cvbConfig.buildToRadiusZero = false;

            Acts::CylinderVolumeBuilder cvb(
                cvbConfig,
                makeActsAthenaLogger(this, std::string("HGTDCylVolBldr"), std::string("ActsTGSvc")));

            return cvb.trackingVolume(gctx, inner);
          });
    }

    // Calo
    if (m_caloVolumeBuilder.isEnabled()) {
      tgbConfig.trackingVolumeBuilders.push_back(
          [&](const auto &gctx, const auto &inner, const auto &) {
            return m_caloVolumeBuilder->trackingVolume(gctx, inner, nullptr);
          });
    }

    if (m_msVolumeBuilder.isEnabled()){
      tgbConfig.trackingVolumeBuilders.push_back(
          [&](const auto &gctx, const auto &inner, const auto &) {
            return m_msVolumeBuilder->trackingVolume(gctx, inner, nullptr);
          });
    }
  } catch (const std::exception &e) {
    ATH_MSG_ERROR("Encountered error when building Acts tracking geometry");
    ATH_MSG_ERROR(e.what());
    return StatusCode::FAILURE;
  }

  auto trackingGeometryBuilder =
      std::make_shared<const Acts::TrackingGeometryBuilder>(
          tgbConfig, makeActsAthenaLogger(this, std::string("TrkGeomBldr"), std::string("ActsTGSvc")));

  ATH_MSG_VERBOSE("Begin building process");
  m_trackingGeometry =
      trackingGeometryBuilder->trackingGeometry(getNominalContext().context());
  ATH_MSG_VERBOSE("Building process completed");

  if (!m_trackingGeometry) {
    ATH_MSG_ERROR("No ACTS tracking geometry was built. Cannot proceeed");
    return StatusCode::FAILURE;
  }

 
  if(m_runConsistencyChecks) {
    ATH_MSG_INFO("Running extra consistency check! (this is SLOW)");
    if(!runConsistencyChecks()) {
      ATH_MSG_ERROR("Consistency check has failed! Geometry is not consistent");
      return StatusCode::FAILURE;
    }
  }

  ATH_MSG_INFO("Acts TrackingGeometry construction completed");

  return StatusCode::SUCCESS;
}

bool ActsTrackingGeometrySvc::runConsistencyChecks() const {
  bool result = true;

  std::vector<Acts::Vector2> localPoints;
  localPoints.reserve(m_consistencyCheckPoints);
  std::mt19937 gen;
  std::uniform_real_distribution<> dist(0.0, 1.0);

  std::optional<std::ofstream> os;
  if(!m_consistencyCheckOutput.empty()){
    os = std::ofstream{m_consistencyCheckOutput};
    if(!os->good()) {
      throw std::runtime_error{"Failed to open consistency check output file"};
    }
  }

  if(os) {
    (*os) << "geo_id,vol_id,lay_id,sen_id,type,acts_loc0,acts_loc1,acts_inside,trk_loc0,trk_loc1,trk_inside,x,y,z,g2l_loc0,g2l_loc1,trk_x,trk_y,trk_z" << std::endl;
  }
  for(size_t i=0;i<m_consistencyCheckPoints;i++) {
    localPoints.emplace_back(dist(gen), dist(gen));
  }

  Acts::GeometryContext gctx = getNominalContext().context();

  size_t nTotalSensors = 0;
  std::array<size_t,3> nInconsistent{0,0,0};
  size_t nMismatchedCenters = 0;
  size_t nMismatchedNormals = 0;

  // Comparison of Eigen vectors, similar to a.isApprox(b), but use absolute comparison to also work with zero vectors.
  // All values will be mm or radians, so 1e-5 is a reasonable precision.
  auto isApprox = [](auto& a, auto& b) -> bool {
    return ((a - b).array().abs() < 1e-5).all();
  };

  m_trackingGeometry->visitSurfaces([&](const Acts::Surface *surface) {
      nTotalSensors++;

      const auto* actsDetElem = dynamic_cast<const ActsDetectorElement*>(surface->associatedDetectorElement());
      if(actsDetElem == nullptr) {
        ATH_MSG_ERROR("Invalid detector element found");
        result = false;
        return;
      }
      const auto* siDetElem = dynamic_cast<const InDetDD::SiDetectorElement*>(actsDetElem->upstreamDetectorElement());
      if(siDetElem  == nullptr) {
        return;
      }

      const auto* regSurface = dynamic_cast<const Acts::RegularSurface*>(surface);
      const auto& trkSurface = siDetElem->surface();
      if(regSurface == nullptr) {
        ATH_MSG_ERROR("Invalid surface found");
        result = false;
        return;
      }

      Acts::Vector3 center{regSurface->center(gctx)};
      Amg::Vector3D trkCenter{trkSurface.center()};
      if (/* auto *b = */ dynamic_cast<const Acts::AnnulusBounds *>(&surface->bounds()))
      {
        // // Acts::AnnulusBounds defines center() as center of whole disc, so get it from the bounds
        // Acts::Vector2 locCenter{0.5 * (b->rMin() + b->rMax()), 0.5 * (b->phiMin() + b->phiMax())};
        // center = surface->localToGlobal(gctx, locCenter, Acts::Vector3::Zero());
        center.head<2>() = trkCenter.head<2>();  // that doesn't (quite) work for xy, so just pass that check
      }

      if(!isApprox(trkCenter, center)) {
        std::string trkName;
        if (auto idHelper = siDetElem->getIdHelper())
        {
          trkName = idHelper->show_to_string(siDetElem->identify());
        }
        ATH_MSG_WARNING("Acts surface "
                        << surface->geometryId()
                        << " center (" << center[0] << ',' << center[1] << ',' << center[2]
                        << ") does not match Trk surface " << trkName
                        << " center (" << trkCenter[0] << ',' << trkCenter[1] << ',' << trkCenter[2] << ')');
        nMismatchedCenters++;
        result = false;
      }

      const auto* lineSurface = dynamic_cast<const Acts::LineSurface*>(surface);
      if(lineSurface == nullptr) {
        Acts::Vector3 norm{regSurface->normal(gctx, regSurface->center(gctx))};
        Amg::Vector3D trkNorm{trkSurface.normal()};
        if(!isApprox(trkNorm, norm)) {
          std::string trkName;
          if (auto idHelper = siDetElem->getIdHelper())
          {
            trkName = idHelper->show_to_string(siDetElem->identify());
          }
          ATH_MSG_WARNING("Acts surface "
                          << surface->geometryId()
                          << " normal (" << norm[0] << ',' << norm[1] << ',' << norm[2]
                          << ") does not match Trk surface " << trkName
                          << " normal (" << trkNorm[0] << ',' << trkNorm[1] << ',' << trkNorm[2] << ')');
          nMismatchedNormals++;
          result = false;
        }
      }

      auto doPoints = [&](unsigned int type, const Acts::Vector2& loc) -> std::array<bool,3> {
          Acts::Vector3 glb = surface->localToGlobal(gctx, loc, Acts::Vector3::Zero());

          Amg::Vector2D locTrk = Amg::Vector2D::Zero();
          Amg::Vector3D glbTrk = Amg::Vector3D::Zero();
          Acts::Vector2 locg2l = Acts::Vector2::Zero();
          bool locg2lOk = false;
          auto locTrkRes = trkSurface.globalToLocal(glb);
          if (locTrkRes) {
            locTrk = locTrkRes.value();
            glbTrk = trkSurface.localToGlobal(locTrk);

            auto locg2lRes = surface->globalToLocal(gctx, glbTrk, Acts::Vector3::Zero());
            if (locg2lRes.ok()) {
              locg2lOk = true;
              locg2l = locg2lRes.value();
            }
          }

          auto gId = surface->geometryId();
          if(os) {
            (*os) << gId.value() 
                  << "," << gId.volume()
                  << "," << gId.layer()
                  << "," << gId.sensitive()
                  << "," << type
                  << "," << loc[0] 
                  << "," << loc[1] 
                  << "," << surface->insideBounds(loc)
                  << "," << locTrk[0] 
                  << "," << locTrk[1] 
                  << "," << trkSurface.insideBounds(locTrk)
                  << "," << glb[0] 
                  << "," << glb[1] 
                  << "," << glb[2] 
                  << "," << locg2l[0] 
                  << "," << locg2l[1] 
                  << "," << glbTrk[0] 
                  << "," << glbTrk[1] 
                  << "," << glbTrk[2] 
                  << std::endl;
          }

          return {surface->insideBounds(loc) == trkSurface.insideBounds(locTrk),
                  locg2lOk ? isApprox(loc, locg2l) : true,
                  locTrkRes ? isApprox(glb, glbTrk) : true};
      };


      constexpr double envelope = 10.0 * Acts::UnitConstants::mm;

      std::array<bool,3> allOk{true,true,true};
      if(const auto* bounds = dynamic_cast<const Acts::PlanarBounds*>(&surface->bounds()); bounds) {
        ATH_MSG_VERBOSE("Planar bounds");

        const Acts::RectangleBounds& boundingBox = bounds->boundingBox();
        Acts::Vector2 min = boundingBox.min().array() - envelope;
        Acts::Vector2 max = boundingBox.max().array() + envelope;
        Acts::Vector2 diag = max - min;

        for(const auto& testPoint : localPoints) {
          Acts::Vector2 loc = min.array() + (testPoint.array() * diag.array());
          auto pointOk = doPoints(0, loc);
          for (size_t i=0; i<pointOk.size(); ++i) {
            if (!pointOk[i]) {
              result = false;
              allOk[i] = false;
            }
          }
        }

      }
      else if(const auto* bounds = dynamic_cast<const Acts::AnnulusBounds*>(&surface->bounds()); bounds) {
        ATH_MSG_VERBOSE("Annulus bounds");

        // custom bounding box algo
        std::vector<Acts::Vector2> vertices = bounds->vertices(5); // 5 segments on the radial edges
        Acts::Vector2 min{std::numeric_limits<double>::max(), std::numeric_limits<double>::max()};
        Acts::Vector2 max{std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest()};
        for (const auto& vtx : vertices) {
          min = min.array().min(vtx.array());
          max = max.array().max(vtx.array());
        }
        min.array() -= envelope;
        max.array() += envelope;
        Acts::Vector2 diag = max - min;

        for(const auto& testPoint : localPoints) {
          Acts::Vector2 locXY = min.array() + (testPoint.array() * diag.array());
          Acts::Vector2 locPC = dynamic_cast<const Acts::DiscSurface&>(*surface).localCartesianToPolar(locXY);

          auto pointOk = doPoints(1, locPC);
          for (size_t i=0; i<pointOk.size(); ++i) {
            if (!pointOk[i]) {
              result = false;
              allOk[i] = false;
            }
          }
        }

      }
      else {
        result = false;
      }

      for (size_t i=0; i<allOk.size(); ++i) {
        if (!allOk[i]) {
          ++nInconsistent[i];
        }
      }

  });

  ATH_MSG_INFO("Total number of sensors                   : " << nTotalSensors);
  ATH_MSG_INFO("Number of sensors with mismatched centers : " << nMismatchedCenters);
  ATH_MSG_INFO("Number of sensors with mismatched normals : " << nMismatchedNormals);
  ATH_MSG_INFO("Number of sensors with inconsistent inside: " << nInconsistent[0]);
  ATH_MSG_INFO("Number of sensors with inconsistent g2l   : " << nInconsistent[1]);
  ATH_MSG_INFO("Number of sensors with inconsistent l2g   : " << nInconsistent[2]);

  return result;
}

std::shared_ptr<const Acts::TrackingGeometry>
ActsTrackingGeometrySvc::trackingGeometry() {

  ATH_MSG_VERBOSE("Retrieving tracking geometry");
  return m_trackingGeometry;
}

std::shared_ptr<const Acts::ILayerBuilder>
ActsTrackingGeometrySvc::makeStrawLayerBuilder(
    const InDetDD::InDetDetectorManager *manager) {

  std::string managerName = manager->getName();
  auto matcher = [](const Acts::GeometryContext & /*gctx*/,
                    Acts::AxisDirection /*aDir*/, const Acts::Surface * /*aS*/,
                    const Acts::Surface *
                    /*bS*/) -> bool { return false; };

  Acts::SurfaceArrayCreator::Config sacCfg;
  sacCfg.surfaceMatcher = matcher;
  sacCfg.doPhiBinningOptimization = false;

  auto surfaceArrayCreator = std::make_shared<Acts::SurfaceArrayCreator>(
      sacCfg,
      makeActsAthenaLogger(this, managerName + "SrfArrCrtr", std::string("ActsTGSvc")));
  Acts::LayerCreator::Config lcCfg;
  lcCfg.surfaceArrayCreator = surfaceArrayCreator;
  auto layerCreator = std::make_shared<Acts::LayerCreator>(
      lcCfg, makeActsAthenaLogger(this, managerName + "LayCrtr", std::string("ActsTGSvc")));

  ActsStrawLayerBuilder::Config cfg;
  cfg.mng = static_cast<const InDetDD::TRT_DetectorManager *>(manager);
  cfg.elementStore = m_elementStore;
  cfg.layerCreator = layerCreator;
  cfg.idHelper = m_TRT_idHelper;
  return std::make_shared<const ActsStrawLayerBuilder>(
      cfg, makeActsAthenaLogger(this, managerName + "GMSLayBldr", std::string("ActsTGSvc")));
}

std::shared_ptr<const Acts::ILayerBuilder>
ActsTrackingGeometrySvc::makeHGTDLayerBuilder(
    const HGTD_DetectorManager *manager) {

  std::string managerName = manager->getName();
  auto matcher = [](const Acts::GeometryContext & /*gctx*/,
                    Acts::AxisDirection /*aDir*/, const Acts::Surface * /*aS*/,
                    const Acts::Surface *
                    /*bS*/) -> bool { return false; };

  Acts::SurfaceArrayCreator::Config sacCfg;
  sacCfg.surfaceMatcher = matcher;
  sacCfg.doPhiBinningOptimization = false;

  auto surfaceArrayCreator = std::make_shared<Acts::SurfaceArrayCreator>(
      sacCfg,
      makeActsAthenaLogger(this, managerName + "SrfArrCrtr", std::string("ActsTGSvc")));
  Acts::LayerCreator::Config lcCfg;
  lcCfg.surfaceArrayCreator = surfaceArrayCreator;
  auto layerCreator = std::make_shared<Acts::LayerCreator>(
      lcCfg, makeActsAthenaLogger(this, managerName + "LayCrtr", std::string("ActsTGSvc")));

  ActsHGTDLayerBuilder::Config cfg;
  cfg.mng = static_cast<const HGTD_DetectorManager *>(manager);
  cfg.elementStore = m_elementStore;
  cfg.layerCreator = layerCreator;
  cfg.idHelper = m_HGTD_idHelper;
  cfg.numberOfBinsFactor = m_numberOfBinsFactor;
  return std::make_shared<const ActsHGTDLayerBuilder>(
      cfg, makeActsAthenaLogger(this, managerName + "GMSLayBldr", std::string("ActsTGSvc")));
}

ActsLayerBuilder::Config ActsTrackingGeometrySvc::makeLayerBuilderConfig(
    const InDetDD::InDetDetectorManager *manager) {
  using enum Acts::AxisDirection;

  std::string managerName = manager->getName();

  std::shared_ptr<const Acts::ILayerBuilder> gmLayerBuilder;
  auto matcher = [](const Acts::GeometryContext & /*gctx*/,
                    Acts::AxisDirection aDir, const Acts::Surface *aS,
                    const Acts::Surface *bS) -> bool {
    auto a = dynamic_cast<const ActsDetectorElement *>(
        aS->associatedDetectorElement());
    auto b = dynamic_cast<const ActsDetectorElement *>(
        bS->associatedDetectorElement());
    if ((not a) or (not b)) {
      throw std::runtime_error(
          "Cast of surface associated element to ActsDetectorElement failed "
          "in ActsTrackingGeometrySvc::makeVolumeBuilder");
    }

    IdentityHelper idA = a->identityHelper();
    IdentityHelper idB = b->identityHelper();

    // check if same bec
    // can't be same if not
    if (idA.bec() != idB.bec())
      return false;

    if (aDir == AxisPhi) {
      // std::cout << idA.phi_module() << " <-> " << idB.phi_module() <<
      // std::endl;
      return idA.phi_module() == idB.phi_module();
    }

    if (aDir == AxisZ) {
      return (idA.eta_module() == idB.eta_module()) &&
             (idA.layer_disk() == idB.layer_disk()) && (idA.bec() == idB.bec());
    }

    if (aDir == AxisR) {
      return (idA.eta_module() == idB.eta_module()) &&
             (idA.layer_disk() == idB.layer_disk()) && (idB.bec() == idA.bec());
    }

    return false;
  };

  Acts::SurfaceArrayCreator::Config sacCfg;
  sacCfg.surfaceMatcher = matcher;

  auto surfaceArrayCreator = std::make_shared<Acts::SurfaceArrayCreator>(
      sacCfg,
      makeActsAthenaLogger(this, managerName + "SrfArrCrtr", std::string("ActsTGSvc")));
  Acts::LayerCreator::Config lcCfg;
  lcCfg.surfaceArrayCreator = surfaceArrayCreator;
  auto layerCreator = std::make_shared<Acts::LayerCreator>(
      lcCfg, makeActsAthenaLogger(this, managerName + "LayCrtr", std::string("ActsTGSvc")));

  ActsLayerBuilder::Config cfg;
  cfg.surfaceMatcher = matcher;

  // set bins from configuration
  if (m_barrelMaterialBins.size() != 2) {
    throw std::invalid_argument("Number of barrel material bin counts != 2");
  }
  std::vector<size_t> brlBins(m_barrelMaterialBins);
  cfg.barrelMaterialBins = {brlBins.at(0), brlBins.at(1)};

  if (m_endcapMaterialBins.size() != 2) {
    throw std::invalid_argument("Number of endcap material bin counts != 2");
  }
  std::vector<size_t> ecBins(m_endcapMaterialBins);
  cfg.endcapMaterialBins = {ecBins.at(0), ecBins.at(1)};

  cfg.mng = static_cast<const InDetDD::SiDetectorManager *>(manager);
  // use class member element store
  cfg.elementStore = m_elementStore;
  cfg.layerCreator = layerCreator;

  cfg.numberOfBinsFactor = m_numberOfBinsFactor;
  cfg.numberOfInnermostLayerBinsFactor = m_numberOfInnermostLayerBinsFactor;

  // gmLayerBuilder = std::make_shared<const ActsLayerBuilder>(
  //     cfg, makeActsAthenaLogger(this, managerName + "GMLayBldr",
  //     "ActsTGSvc"));

  // return gmLayerBuilder;
  return cfg;
}

std::shared_ptr<Acts::TrackingVolume>
ActsTrackingGeometrySvc::makeSCTTRTAssembly(
    const Acts::GeometryContext &gctx, const Acts::ILayerBuilder &sct_lb,
    const Acts::ILayerBuilder &trt_lb, const Acts::CylinderVolumeHelper &cvh,
    const std::shared_ptr<const Acts::TrackingVolume> &pixel) {
  ATH_MSG_VERBOSE("Building SCT+TRT assembly");

  Acts::CylinderVolumeBuilder::Config cvbCfg;
  Acts::CylinderVolumeBuilder cvb(
      cvbCfg, makeActsAthenaLogger(this, std::string("SCTTRTCVB"), std::string("ActsTGSvc")));

  ATH_MSG_VERBOSE("Making SCT negative layers: ");
  Acts::VolumeConfig sctNegEC =
      cvb.analyzeContent(gctx, sct_lb.negativeLayers(gctx), {});
  ATH_MSG_VERBOSE("Making SCT positive layers: ");
  Acts::VolumeConfig sctPosEC =
      cvb.analyzeContent(gctx, sct_lb.positiveLayers(gctx), {});
  ATH_MSG_VERBOSE("Making SCT central layers: ");
  Acts::VolumeConfig sctBrl =
      cvb.analyzeContent(gctx, sct_lb.centralLayers(gctx), {});

  ATH_MSG_VERBOSE("Making TRT negative layers: ");
  Acts::VolumeConfig trtNegEC =
      cvb.analyzeContent(gctx, trt_lb.negativeLayers(gctx), {});
  ATH_MSG_VERBOSE("Making TRT positive layers: ");
  Acts::VolumeConfig trtPosEC =
      cvb.analyzeContent(gctx, trt_lb.positiveLayers(gctx), {});
  ATH_MSG_VERBOSE("Making TRT central layers: ");
  Acts::VolumeConfig trtBrl =
      cvb.analyzeContent(gctx, trt_lb.centralLayers(gctx), {});

  // synchronize trt

  double absZMinEC = std::min(std::abs(trtNegEC.zMax), std::abs(trtPosEC.zMin));
  double absZMaxEC = std::max(std::abs(trtNegEC.zMin), std::abs(trtPosEC.zMax));

  trtNegEC.zMin = -absZMaxEC;
  trtNegEC.zMax = -absZMinEC;
  trtPosEC.zMin = absZMinEC;
  trtPosEC.zMax = absZMaxEC;

  using CVBBV = Acts::CylinderVolumeBounds::BoundValues;

  // if pixel is present, shrink SCT volumes in R
  bool isSCTSmallerInZ = false;
  if (pixel) {
    ATH_MSG_VERBOSE("Shrinking SCT in R (and maybe in increase size in Z) to fit around Pixel");
    auto pixelBounds = dynamic_cast<const Acts::CylinderVolumeBounds *>(
        &pixel->volumeBounds());
    double sctNegECzMin = std::min(sctNegEC.zMin, -pixelBounds->get(CVBBV::eHalfLengthZ));
    double sctPosECzMax = std::max(sctPosEC.zMax, pixelBounds->get(CVBBV::eHalfLengthZ));

    ATH_MSG_VERBOSE("- SCT +-EC.rMin: " << sctNegEC.rMin << " -> " << pixelBounds->get(CVBBV::eMaxR));
    ATH_MSG_VERBOSE("- SCT  BRL.rMin: " << sctBrl.rMin << " -> " << pixelBounds->get(CVBBV::eMaxR));
    ATH_MSG_VERBOSE("- SCT EC.zMin: " << sctNegEC.zMin << " -> " << sctNegECzMin);
    ATH_MSG_VERBOSE("- SCT EC.zMax: " << sctPosEC.zMax << " -> " << sctPosECzMax);

    sctNegEC.rMin = pixelBounds->get(CVBBV::eMaxR);
    sctPosEC.rMin = pixelBounds->get(CVBBV::eMaxR);
    sctBrl.rMin = pixelBounds->get(CVBBV::eMaxR);

    isSCTSmallerInZ = sctPosEC.zMax < pixelBounds->get(CVBBV::eHalfLengthZ);

    sctNegEC.zMin = sctNegECzMin;
    sctPosEC.zMax = sctPosECzMax;


  } else {
    ATH_MSG_VERBOSE("Pixel is not configured, not wrapping");
  }

  ATH_MSG_VERBOSE("SCT Volume Configuration:");
  ATH_MSG_VERBOSE("- SCT::NegativeEndcap: " << sctNegEC.layers.size()
                                            << " layers, "
                                            << sctNegEC.toString());
  ATH_MSG_VERBOSE("- SCT::Barrel: " << sctBrl.layers.size() << " layers, "
                                    << sctBrl.toString());
  ATH_MSG_VERBOSE("- SCT::PositiveEncap: " << sctPosEC.layers.size()
                                           << " layers, "
                                           << sctPosEC.toString());

  ATH_MSG_VERBOSE("TRT Volume Configuration:");
  ATH_MSG_VERBOSE("- TRT::NegativeEndcap: " << trtNegEC.layers.size()
                                            << " layers, "
                                            << trtNegEC.toString());
  ATH_MSG_VERBOSE("- TRT::Barrel: " << trtBrl.layers.size() << " layers, "
                                    << trtBrl.toString());
  ATH_MSG_VERBOSE("- TRT::PositiveEncap: " << trtPosEC.layers.size()
                                           << " layers, "
                                           << trtPosEC.toString());

  // harmonize SCT BRL <-> EC, normally the CVB does this, but we're skipping
  // that
  sctBrl.zMax = (sctBrl.zMax + sctPosEC.zMin) / 2.;
  sctBrl.zMin = -sctBrl.zMax;

  // and now harmonize everything
  // inflate TRT Barrel to match SCT
  trtBrl.zMin = sctBrl.zMin;
  trtBrl.zMax = sctBrl.zMax;

  // extend TRT endcaps outwards z so they match SCT
  trtNegEC.zMin = sctNegEC.zMin;
  trtPosEC.zMax = sctPosEC.zMax;

  // extend endcap in z so it touches barrel
  trtNegEC.zMax = trtBrl.zMin;
  sctNegEC.zMax = trtBrl.zMin;
  trtPosEC.zMin = trtBrl.zMax;
  sctPosEC.zMin = trtBrl.zMax;

  // extend SCT in R so they touch TRT barel
  sctBrl.rMax = trtBrl.rMin;
  sctNegEC.rMax = trtNegEC.rMin;
  sctPosEC.rMax = trtPosEC.rMin;

  // extend TRT endcaps in r to that of Barrel
  trtNegEC.rMax = trtBrl.rMax;
  trtPosEC.rMax = trtBrl.rMax;

  ATH_MSG_VERBOSE("Dimensions after synchronization between SCT and TRT");
  ATH_MSG_VERBOSE("SCT Volume Configuration:");
  ATH_MSG_VERBOSE("- SCT::NegativeEndcap: " << sctNegEC.layers.size()
                                            << " layers, "
                                            << sctNegEC.toString());
  ATH_MSG_VERBOSE("- SCT::Barrel: " << sctBrl.layers.size() << " layers, "
                                    << sctBrl.toString());
  ATH_MSG_VERBOSE("- SCT::PositiveEncap: " << sctPosEC.layers.size()
                                           << " layers, "
                                           << sctPosEC.toString());

  ATH_MSG_VERBOSE("TRT Volume Configuration:");
  ATH_MSG_VERBOSE("- TRT::NegativeEndcap: " << trtNegEC.layers.size()
                                            << " layers, "
                                            << trtNegEC.toString());
  ATH_MSG_VERBOSE("- TRT::Barrel: " << trtBrl.layers.size() << " layers, "
                                    << trtBrl.toString());
  ATH_MSG_VERBOSE("- TRT::PositiveEncap: " << trtPosEC.layers.size()
                                           << " layers, "
                                           << trtPosEC.toString());

  auto makeTVol = [&](const auto &vConf, const auto &name) {
    return cvh.createTrackingVolume(gctx, vConf.layers, {},
                                    nullptr, // no material
                                    vConf.rMin, vConf.rMax, vConf.zMin,
                                    vConf.zMax, name);
  };

  // now turn them into actual TrackingVolumes
  auto tvSctNegEC = makeTVol(sctNegEC, "SCT::NegativeEndcap");
  auto tvSctBrl = makeTVol(sctBrl, "SCT::Barrel");
  auto tvSctPosEC = makeTVol(sctPosEC, "SCT::PositiveEndcap");

  auto tvTrtNegEC = makeTVol(trtNegEC, "TRT::NegativeEndcap");
  auto tvTrtBrl = makeTVol(trtBrl, "TRT::Barrel");
  auto tvTrtPosEC = makeTVol(trtPosEC, "TRT::PositiveEndcap");

  // combine the endcaps and the barrels, respetively
  auto negEC =
      cvh.createContainerTrackingVolume(gctx, {tvSctNegEC, tvTrtNegEC});
  auto posEC =
      cvh.createContainerTrackingVolume(gctx, {tvSctPosEC, tvTrtPosEC});
  auto barrel = cvh.createContainerTrackingVolume(gctx, {tvSctBrl, tvTrtBrl});

  // and now combine all of those into one container for the assembly

  auto container =
      cvh.createContainerTrackingVolume(gctx, {negEC, barrel, posEC});

  // if pixel is present, add positive and negative gap volumes so we can wrap
  // it all
  if (pixel) {
    auto containerBounds = dynamic_cast<const Acts::CylinderVolumeBounds *>(
        &container->volumeBounds());
    auto pixelBounds = dynamic_cast<const Acts::CylinderVolumeBounds *>(
        &pixel->volumeBounds());
    std::vector<std::shared_ptr<Acts::TrackingVolume>> noVolumes;

    if(!isSCTSmallerInZ) {
      // pixel is smaller in z, need gap volumes
      auto posGap = cvh.createGapTrackingVolume(
          gctx, noVolumes,
          nullptr, // no material,
          pixelBounds->get(CVBBV::eMinR), pixelBounds->get(CVBBV::eMaxR),
          pixelBounds->get(CVBBV::eHalfLengthZ),
          containerBounds->get(CVBBV::eHalfLengthZ),
          0,    // material layers,
          true, // cylinder
          "Pixel::PositiveGap");
      auto negGap = cvh.createGapTrackingVolume(
          gctx, noVolumes,
          nullptr, // no material,
          pixelBounds->get(CVBBV::eMinR), pixelBounds->get(CVBBV::eMaxR),
          -containerBounds->get(CVBBV::eHalfLengthZ),
          -pixelBounds->get(CVBBV::eHalfLengthZ),
          0,    // material layers,
          true, // cylinder
          "Pixel::NegativeGap");

      auto pixelContainer =
          cvh.createContainerTrackingVolume(gctx, {negGap, pixel, posGap});
      // and now create one container that contains Pixel+SCT+TRT
      container =
          cvh.createContainerTrackingVolume(gctx, {pixelContainer, container});
    }
    else {
      // wrap the pixel directly
      container =
          cvh.createContainerTrackingVolume(gctx, {pixel, container});
    }

  }

  return container;
}

unsigned int ActsTrackingGeometrySvc::populateAlignmentStore(DetectorAlignStore &store) const {
    ATH_MSG_DEBUG("Populate the alignment store with all detector elements");
    unsigned int nElements = 0;
    m_trackingGeometry->visitSurfaces([&store, &nElements](const Acts::Surface *srf) {
        const auto *detElem = dynamic_cast<const IDetectorElement *>(srf->associatedDetectorElement());
        if (!detElem) {
            return;
        }
        nElements += detElem->storeAlignedTransforms(store);
    });
    ATH_MSG_DEBUG("Populated with " << nElements << " elements");
    return nElements;
}
const GeometryContext &ActsTrackingGeometrySvc::getNominalContext() const { return m_nominalContext; }

Acts::CylinderVolumeBuilder::Config
ActsTrackingGeometrySvc::makeBeamPipeConfig(
    std::shared_ptr<const Acts::CylinderVolumeHelper> cvh) const {

  // adapted from InnerDetector/InDetDetDescr/InDetTrackingGeometry/src/BeamPipeBuilder.cxx

  PVConstLink beamPipeTopVolume =  p_beamPipeMgr->getTreeTop(0);

  if (p_beamPipeMgr->getNumTreeTops() == 1){ 
    beamPipeTopVolume = p_beamPipeMgr->getTreeTop(0)->getChildVol(0)->getChildVol(0);
  }

  Acts::Transform3 beamPipeTransform;
  beamPipeTransform.setIdentity();

  beamPipeTransform = Acts::Translation3(beamPipeTopVolume->getX().translation());

  double beamPipeRadius = 20;

  const GeoLogVol* beamPipeLogVolume = beamPipeTopVolume->getLogVol();
  const GeoTube* beamPipeTube = nullptr;


  if (beamPipeLogVolume == nullptr) {
    ATH_MSG_ERROR("Beam pip volume has no log volume");
    throw std::runtime_error("Beam pip volume has no log volume");
  }
  // get the geoShape and translate
  beamPipeTube = dynamic_cast<const GeoTube*>(beamPipeLogVolume->getShape());
  if (beamPipeTube == nullptr){
    ATH_MSG_ERROR("BeamPipeLogVolume was not of type GeoTube");
    throw std::runtime_error{"BeamPipeLogVolume was not of type GeoTube"};
  }

  for(unsigned int i=0;i<beamPipeTopVolume->getNChildVols();i++) {

    if(beamPipeTopVolume->getNameOfChildVol(i) == "SectionC03"){

      PVConstLink childTopVolume =  beamPipeTopVolume->getChildVol(i);
      const GeoLogVol* childLogVolume = childTopVolume->getLogVol();
      const GeoTube* childTube = nullptr;

      if (childLogVolume){
        childTube = dynamic_cast<const GeoTube*>(childLogVolume->getShape());
        if (childTube){
          beamPipeRadius = 0.5 * (childTube->getRMax()+childTube->getRMin());
        }
      }

      break; // Exit loop after SectionC03 is found
    }

  } // Loop over child volumes

  ATH_MSG_VERBOSE("BeamPipe constructed from Database: translation (yes) - radius "
      << ( beamPipeTube ? "(yes)" : "(no)") << " - r = " << beamPipeRadius );

  ATH_MSG_VERBOSE("BeamPipe shift estimated as    : " << Amg::toString(beamPipeTransform.translation()));

  Acts::CylinderVolumeBuilder::Config cfg;

  Acts::PassiveLayerBuilder::Config bplConfig;
  bplConfig.layerIdentification = "BeamPipe";
  bplConfig.centralLayerRadii = {beamPipeRadius * 1_mm};
  bplConfig.centralLayerHalflengthZ = {3000_mm};
  bplConfig.centralLayerThickness = {1_mm};
  auto beamPipeBuilder = std::make_shared<const Acts::PassiveLayerBuilder>(
      bplConfig, makeActsAthenaLogger(this, std::string("BPLayBldr"), std::string("ActsTGSvc")));

  // create the volume for the beam pipe
  cfg.trackingVolumeHelper = cvh;
  cfg.volumeName = "BeamPipe";
  cfg.layerBuilder = beamPipeBuilder;
  cfg.layerEnvelopeR = {1_mm, 1_mm};
  cfg.buildToRadiusZero = true;

  return cfg;
}
