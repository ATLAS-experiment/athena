/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// This absolutely needs to go first to ensure Eigen plugin is loaded
#include "GeoPrimitives/GeoPrimitives.h"
//
#include <GeoModelKernel/GeoTube.h>
#include <GeoModelKernel/GeoVPhysVol.h>

#include <Acts/Definitions/Units.hpp>
#include <Acts/Geometry/ContainerBlueprintNode.hpp>
#include <Acts/Geometry/CylinderVolumeBounds.hpp>
#include <Acts/Geometry/GeometryIdentifierBlueprintNode.hpp>
#include <Acts/Geometry/MaterialDesignatorBlueprintNode.hpp>
#include <Acts/Geometry/StaticBlueprintNode.hpp>
#include <Acts/Geometry/VolumeAttachmentStrategy.hpp>
#include <Acts/Geometry/VolumeResizeStrategy.hpp>
#include <Acts/Utilities/AxisDefinitions.hpp>

#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "ActsGeometryInterfaces/GeometryDefs.h"
#include "BeamPipeBlueprintNodeBuilder.h"
#include "BeamPipeGeoModel/BeamPipeDetectorManager.h"
#include "GeoPrimitives/GeoPrimitivesHelpers.h"

using namespace Acts::UnitLiterals;

namespace {
using namespace ActsTrk::detail::GeoVolIds;

using enum Acts::CylinderVolumeBounds::Face;
using enum Acts::AxisDirection;
using enum Acts::AxisBoundaryType;
using AttachmentStrategy = Acts::VolumeAttachmentStrategy;
using ResizeStrategy = Acts::VolumeResizeStrategy;

}  // namespace

namespace ActsTrk {

StatusCode BeamPipeBlueprintNodeBuilder::initialize() {
  if (m_loadfromDatabase) {
    ATH_CHECK(detStore()->retrieve(m_beamPipeMgr, "BeamPipe"));
  }
  return StatusCode::SUCCESS;
}

std::shared_ptr<Acts::Experimental::BlueprintNode>
BeamPipeBlueprintNodeBuilder::buildBlueprintNode(
    const Acts::GeometryContext& /*gctx*/,
    std::shared_ptr<Acts::Experimental::BlueprintNode>&& child) {

  // Beam pipe is the innermost element and has no child to wrap
  if (child) {
    ATH_MSG_ERROR("BeamPipeBlueprintNodeBuilder expects no child node");
    throw std::runtime_error("Child node is not null");
  }

  // If the manager is available, use it to construct the beam pipe geometry,
  // and retrieve the beampipe radius from the geometry database.
  // Otherwise, use default parameters to construct a simple tube.
  double beamPipeRadius = m_defaultInnerRadius;  // mm
  Amg::Transform3D beamPipeTransform = Amg::Transform3D::Identity();

  if (m_loadfromDatabase) {

    // Navigate the GeoModel tree to reach the actual beam pipe volume.
    // When there is a single tree top, the real tube sits two levels down
    // (envelope → envelope → tube).
    PVConstLink beamPipeTopVolume = m_beamPipeMgr->getTreeTop(0);
    if (m_beamPipeMgr->getNumTreeTops() == 1) {
      beamPipeTopVolume =
          m_beamPipeMgr->getTreeTop(0)->getChildVol(0)->getChildVol(0);
    }

    // Extract the translation so the volume is placed at the same
    // position as the GeoModel volume.
    beamPipeTransform =
        Amg::getTranslate3D(beamPipeTopVolume->getX().translation());

    // If SectionC03 is found, then overwrite the default radius
    const GeoLogVol* beamPipeLogVolume = beamPipeTopVolume->getLogVol();
    if (beamPipeLogVolume == nullptr) {
      ATH_MSG_ERROR("Beam pipe volume has no log volume");
      throw std::runtime_error("Beam pipe volume has no log volume");
    }

    const GeoTube* beamPipeTube =
        dynamic_cast<const GeoTube*>(beamPipeLogVolume->getShape());
    if (beamPipeTube == nullptr) {
      ATH_MSG_ERROR("BeamPipeLogVolume was not of type GeoTube");
      throw std::runtime_error{"BeamPipeLogVolume was not of type GeoTube"};
    }

    // SectionC03 is the central beam-pipe section at η≈0 whose mid-radius is
    // used as the representative beam pipe radius for the Acts cylinder.
    for (unsigned int i = 0; i < beamPipeTopVolume->getNChildVols(); i++) {
      if (beamPipeTopVolume->getNameOfChildVol(i) != "SectionC03") {
        continue;
      }
      PVConstLink childTopVolume = beamPipeTopVolume->getChildVol(i);
      const GeoLogVol* childLogVolume = childTopVolume->getLogVol();
      const GeoTube* childTube =
          dynamic_cast<const GeoTube*>(childLogVolume->getShape());
      if (childTube) {
        beamPipeRadius = 0.5 * (childTube->getRMax() + childTube->getRMin());
        break;
      }
    }

    ATH_MSG_VERBOSE(
        "BeamPipe constructed from database with radius = " << beamPipeRadius);
    ATH_MSG_VERBOSE(
        "BeamPipe shift : " << beamPipeTransform.translation().transpose());
  } else {
    ATH_MSG_VERBOSE(
        "BeamPipe constructed from default parameters with radius = "
        << beamPipeRadius);
    ATH_MSG_VERBOSE(
        "BeamPipe shift : " << beamPipeTransform.translation().transpose());
  }

  // Build the blueprint subtree:
  //   GeometryIdentifierNode  (assigns volume ID s_beamPipeVolumeId)
  //     └── MaterialDesignator "BeamPipe_Material"  (material on outer
  //     cylinder)
  //           └── StaticVolume "BeamPipe"           (the actual cylinder)
  auto beamPipeNode =
      std::make_shared<Acts::Experimental::GeometryIdentifierBlueprintNode>();
  beamPipeNode->setAllVolumeIdsTo(s_beamPipeVolumeId).incrementLayerIds(1);

  auto& beamPipeContainer =
      beamPipeNode->addCylinderContainer("BeamPipeContainer", AxisR);
  beamPipeContainer.setAttachmentStrategy(AttachmentStrategy::Gap);
  beamPipeContainer.setResizeStrategy(ResizeStrategy::Gap);

  beamPipeContainer.addMaterial("BeamPipe_Material", [&](auto& mat) {
    // Place material on the outer cylindrical surface facing outward
    mat.configureFace(OuterCylinder, {AxisRPhi, Closed, 20},
                      {AxisZ, Bound, 20});
    // A solid cylinder from r=0 to beamPipeRadius, extending ±3 m in z
    mat.addStaticVolume(beamPipeTransform,
                        std::make_shared<Acts::CylinderVolumeBounds>(
                            0, beamPipeRadius * 1_mm, 3. * 1_m),
                        "BeamPipeVolume");
  });

  return beamPipeNode;
}

}  // namespace ActsTrk
