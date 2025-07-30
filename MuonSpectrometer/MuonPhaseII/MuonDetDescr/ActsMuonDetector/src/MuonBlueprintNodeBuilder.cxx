/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

*/

#include "MuonBlueprintNodeBuilder.h"

#include "Acts/Geometry/BlueprintNode.hpp"
#include "Acts/Geometry/StaticBlueprintNode.hpp"
#include "Acts/Geometry/CylinderVolumeBounds.hpp"
#include <Acts/Geometry/ContainerBlueprintNode.hpp>
#include "Acts/Geometry/MultiWireVolumeBuilder.hpp"
#include "Acts/Surfaces/TrapezoidBounds.hpp"
#include "Acts/Material/HomogeneousSurfaceMaterial.hpp"
#include <Acts/Geometry/MaterialDesignatorBlueprintNode.hpp>
#include <Acts/Geometry/GeometryIdentifierBlueprintNode.hpp>
#include <Acts/Geometry/VolumeAttachmentStrategy.hpp>
#include "Acts/Geometry/VolumeResizeStrategy.hpp"
#include <Acts/Geometry/TrackingVolume.hpp>
#include <Acts/Geometry/TrapezoidVolumeBounds.hpp>
#include <Acts/Surfaces/PlaneSurface.hpp>
#include <Acts/Plugins/GeoModel/GeoModelMaterialConverter.hpp>
#include <Acts/Visualization/ObjVisualization3D.hpp>


#include <MuonReadoutGeometryR4/Chamber.h>
#include <MuonStationIndex/MuonStationIndex.h>

#include "GeoModelValidation/GeoMaterialHelper.h"

namespace {
  //Muon System IDs
constexpr std::size_t s_muonBarrelId = 30;
constexpr std::size_t s_muonEndcapAId = 31;
constexpr std::size_t s_muonEndcapCId = 32;
}

namespace ActsTrk {

  StatusCode MuonBlueprintNodeBuilder::initialize(){
        ATH_CHECK(detStore()->retrieve(m_detMgr));
        return StatusCode::SUCCESS;
  }


std::shared_ptr<Acts::Experimental::BlueprintNode> MuonBlueprintNodeBuilder::buildBlueprintNode(const Acts::GeometryContext& gctx, std::shared_ptr<Acts::Experimental::BlueprintNode>&& childNode) {

  // Top level node for the Muon system
auto muonNode = std::make_shared<Acts::Experimental::CylinderContainerBlueprintNode>("MuonNode", Acts::AxisDirection::AxisZ);

Acts::VolumeBoundFactory boundsFactory{};

auto barrelNode = buildMuonNode(gctx, {StIdx::BI, StIdx::BM, StIdx::BO, StIdx::EE, StIdx::EI}, EndcapSide::Both, Acts::GeometryIdentifier().withVolume(s_muonBarrelId), boundsFactory);
auto endcapANode = buildMuonNode(gctx, {StIdx::EM, StIdx::EO}, EndcapSide::A, Acts::GeometryIdentifier().withVolume(s_muonEndcapAId), boundsFactory);
auto endcapCNode = buildMuonNode(gctx, {StIdx::EM, StIdx::EO}, EndcapSide::C, Acts::GeometryIdentifier().withVolume(s_muonEndcapCId), boundsFactory);

// Add to the muon barrel child node (e.g calo or Itk) - if existed
if(childNode){
  barrelNode->addChild(std::move(childNode));
}

muonNode->addChild(std::move(barrelNode));
muonNode->addChild(std::move(endcapANode));
muonNode->addChild(std::move(endcapCNode));

return muonNode;

}

std::shared_ptr<Acts::Experimental::StaticBlueprintNode>
MuonBlueprintNodeBuilder::buildMuonNode(
    const Acts::GeometryContext& gctx,
    const std::vector<StIdx>& stations,
    const EndcapSide& side,
    const Acts::GeometryIdentifier& id,
    Acts::VolumeBoundFactory& boundsFactory) const {

    const MuonChamberSet chambers = m_detMgr->getAllChambers();
    const ActsGeometryContext* context = gctx.get<const ActsGeometryContext* >();
    std::vector<std::string> stationNames;
    stationNames.reserve(stations.size());
    // Convert station indices to names
    std::transform(stations.begin(), stations.end(), std::back_inserter(stationNames),
                   [](const StIdx& st) { return Muon::MuonStationIndex::stName(st); });

    std::vector<std::shared_ptr<Acts::Experimental::StaticBlueprintNode>> nodes;
  
    double innerRadius = 0.0;
    double outerRadius = std::numeric_limits<double>::lowest();
    double maxZ = std::numeric_limits<double>::lowest();
    double minZ = std::numeric_limits<double>::max();

   
    int chamberId = 1;
    ATH_MSG_DEBUG("Chambers= "<<chambers.size());
    for(const MuonGMR4::Chamber* chamber: chambers){
      if(!isChamberInTheStation(*chamber, stations, side)) {
        continue;
      }
      const Amg::Transform3D& transform = chamber->localToGlobalTrans(*context);
      auto vol = std::make_unique<Acts::TrackingVolume>(
                                            transform,
                                            chamber->bounds(),
                                            chamber->identString());

      // Get material per chamber, blend it and place it in the center of the volume 
      auto material = blendChamberMaterial(*chamber);
      vol->addSurface(std::move(material));
      //the chamber geometry id
      Acts::GeometryIdentifier chId = id.withLayer(chamberId++);
      vol->assignGeometryId(chId);

      std::pair<std::vector<volumePtr>,std::vector<surfacePtr>> innerStructure = getSensitiveElements(*context, *chamber, chId, boundsFactory);

      //get the readout elements as volumes and surfaces and add them to the node
      for(auto& readoutVol : innerStructure.first){
        vol->addVolume(std::move(readoutVol));
      }

      for(auto& surface: innerStructure.second){
        vol->addSurface(surface);
      }

      //calculate the bounds of the cylinder container
      for(const auto& surface: vol->boundarySurfaces()){
        const auto& surfaceRepr = surface->surfaceRepresentation();
        const Acts::Polyhedron& polyhedron = surfaceRepr.polyhedronRepresentation(gctx);
        const Amg::Vector3D& center = surfaceRepr.center(gctx);

        maxZ = std::max(maxZ, center.z());
        minZ = std::min(minZ, center.z());

        // Outer radius needs to be treated differently due to curvature of cylindrical surface
        for(const Amg::Vector3D& vertex: polyhedron.vertices){
          outerRadius = std::max(outerRadius, vertex.perp());
        }
      }
      if(m_dumpVolumes){
        //for visualizing each chamber volume individually
        Acts::ObjVisualization3D helper;
        vol->visualize(helper, gctx, {.visible = true},
                                {.visible = false}, {.visible = true});
        helper.write(chamber->identString() + ".obj");
        helper.clear();
      }
      
      nodes.emplace_back(std::make_shared<Acts::Experimental::StaticBlueprintNode>(std::move(vol)));
    }

    double halfLengthZ = 0.5 * std::abs(maxZ - minZ);
    ATH_MSG_DEBUG("Inner radius: " << innerRadius);
    ATH_MSG_DEBUG("Outer radius: " << outerRadius);
    ATH_MSG_DEBUG("Max Z: " << maxZ);
    ATH_MSG_DEBUG("Min Z: " << minZ);
    ATH_MSG_DEBUG("Half length Z: " << halfLengthZ);

    Amg::Transform3D trf = Amg::getTranslateZ3D(halfLengthZ + minZ);

    auto bounds = boundsFactory.makeBounds<Acts::CylinderVolumeBounds>(innerRadius, outerRadius, halfLengthZ);
    auto sideStr = (side == EndcapSide::A) ? "A" : (side == EndcapSide::C) ? "C" : "Both";
    auto volume = std::make_unique<Acts::TrackingVolume>(trf, bounds, std::accumulate(stationNames.begin(), stationNames.end(), std::string("MuonChamber")) + "Volume" + sideStr);
    volume->assignGeometryId(id);
    auto muonNode = std::make_shared<Acts::Experimental::StaticBlueprintNode>(std::move(volume));

    ATH_MSG_DEBUG("There are " << nodes.size() << " nodes");
    std::ranges::for_each(nodes, [&muonNode](auto& node) {
      muonNode->addChild(std::move(node));
    });
    return muonNode;
  }


std::pair<std::vector<volumePtr>, std::vector<surfacePtr>>
MuonBlueprintNodeBuilder::getSensitiveElements(
    const ActsGeometryContext& gctx,
    const MuonGMR4::Chamber& chamber,
    const Acts::GeometryIdentifier& chId,
    Acts::VolumeBoundFactory& boundsFactory) const {

  std::vector<volumePtr> readoutVolumes;
  std::vector<surfacePtr> readoutSurfaces;
  Acts::GeometryIdentifier::Value mdtId{1};

  for (const MuonGMR4::MuonReadoutElement* readoutEle : chamber.readoutEles()) {

    std::vector<surfacePtr> detSurfaces = readoutEle->getSurfaces();
    switch(readoutEle->detectorType()){
      case DetectorType::Mdt: {    
        const auto* mdtReadoutEle = static_cast<const MuonGMR4::MdtReadoutElement*>(readoutEle);
        const MuonGMR4::MdtReadoutElement::parameterBook& parameters{mdtReadoutEle->getParameters()};

          // get the transform to the chamber's frame
          const Amg::Vector3D toChamber = chamber.globalToLocalTrans(gctx)*mdtReadoutEle->center(gctx);
          const Acts::Transform3 mdtTransform = chamber.localToGlobalTrans(gctx) * Amg::getTranslate3D(toChamber);     

          // create the MDT multilayer volume with the dedicated builder
          Acts::Experimental::MultiWireVolumeBuilder::Config mwCfg;
          mwCfg.name = m_detMgr->idHelperSvc()->toStringDetEl(mdtReadoutEle->identify());
          mwCfg.mlSurfaces = detSurfaces;
          mwCfg.transform = mdtTransform;

          auto mdtBounds = boundsFactory.makeBounds<Acts::TrapezoidVolumeBounds>(parameters.shortHalfX, 
          parameters.longHalfX, parameters.halfY, parameters.halfHeight);

          mwCfg.bounds = mdtBounds;
          using BoundsV = Acts::TrapezoidVolumeBounds::BoundValues;
          mwCfg.binning = {{{Acts::AxisDirection::AxisY, Acts::AxisBoundaryType::Bound,
                            -mdtBounds->get(BoundsV::eHalfLengthY),
                            mdtBounds->get(BoundsV::eHalfLengthY),
                            static_cast<std::size_t>(std::lround(2 * mdtBounds->get(BoundsV::eHalfLengthY) / parameters.tubePitch))}, 2u},
                            {{Acts::AxisDirection::AxisZ, Acts::AxisBoundaryType::Bound,
                              -mdtBounds->get(BoundsV::eHalfLengthZ),
                              mdtBounds->get(BoundsV::eHalfLengthZ),
                              static_cast<std::size_t>(std::lround(2 * mdtBounds->get(BoundsV::eHalfLengthZ) / parameters.tubePitch))}, 1u}};
          Acts::Experimental::MultiWireVolumeBuilder mdtBuilder{mwCfg};
          std::unique_ptr<Acts::TrackingVolume> mdtVolume = mdtBuilder.buildVolume(gctx.context());

          mdtVolume->assignGeometryId(chId.withExtra(mdtId++));
          readoutVolumes.push_back(std::move(mdtVolume));
          break;

        } case DetectorType::Rpc: 
          case DetectorType::Tgc:
          case DetectorType::sTgc:
          case DetectorType::Mm: {              
             
          readoutSurfaces.insert(readoutSurfaces.end(), std::make_move_iterator(detSurfaces.begin()),
                                 std::make_move_iterator(detSurfaces.end()));

          break;

        } default: 
              THROW_EXCEPTION("Unknown detector type for readout element: " << ActsTrk::to_string(readoutEle->detectorType()));
              break;
     
    }
  }

  return std::make_pair(std::move(readoutVolumes), std::move(readoutSurfaces));
}



std::shared_ptr<Acts::Surface>
MuonBlueprintNodeBuilder::blendChamberMaterial(
    const MuonGMR4::Chamber& chamber) const {

  const float thickness = chamber.halfZ() * 2;
  PVConstLink parentVolume = chamber.readoutEles().front()->getMaterialGeom()->getParent();
  GeoModelTools::GeoMaterialHelper geoMaterialHelper;
  std::pair<GeoModelTools::GeoMaterialPtr, double> geoMaterials = geoMaterialHelper.collectMaterial(parentVolume);

  const Acts::Material aMat = Acts::GeoModel::geoMaterialConverter(*geoMaterials.first);
  //rotate about the z axis
  auto constPtr = chamber.surface().getSharedPtr();
  //to assign the material shouldnt be const
  auto ptr = std::const_pointer_cast<Acts::Surface>(constPtr);
  Acts::MaterialSlab slab{aMat, thickness};
  std::shared_ptr<Acts::HomogeneousSurfaceMaterial> material = std::make_shared<Acts::HomogeneousSurfaceMaterial>(slab);
  ptr->assignSurfaceMaterial(material);
  return ptr;

}

bool MuonBlueprintNodeBuilder::isChamberInTheStation(const MuonGMR4::Chamber& chamber, const std::vector<StIdx>& stationIndex, const EndcapSide& side) const {
  StIdx stationIdx = toStationIndex(chamber.chamberIndex());
  bool matchesName = std::ranges::any_of(stationIndex.begin(), stationIndex.end(), [&](const auto& n){
        return stationIdx == n;
      });
      const int stationEta = chamber.stationEta();
      bool etaSignCorrect = ((stationEta > 0 && side == EndcapSide::A) || (stationEta < 0 && side == EndcapSide::C) || (side == EndcapSide::Both));
      return matchesName && etaSignCorrect;
}


} //namespace ActsTrk