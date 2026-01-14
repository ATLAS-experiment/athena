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
#include <Acts/Geometry/CuboidVolumeBounds.hpp>
//#include <Acts/Geometry/ConvexPolygonVolumeBounds.hpp>
#include <Acts/Surfaces/PlaneSurface.hpp>
#include <ActsPlugins/GeoModel/GeoModelMaterialConverter.hpp>
#include <Acts/Visualization/ObjVisualization3D.hpp>


#include <MuonReadoutGeometryR4/Chamber.h>
#include <MuonReadoutGeometryR4/SpectrometerSector.h>
#include <MuonStationIndex/MuonStationIndex.h>

#include "GeoModelValidation/GeoMaterialHelper.h"

namespace {
  //Muon System IDs
constexpr std::size_t s_muonBarrelId = 30;
constexpr std::size_t s_muonEndcapAId = 31;
constexpr std::size_t s_muonEndcapCId = 32;
constexpr std::size_t s_muonEndcapMiddleAId = 33;
constexpr std::size_t s_muonEndcapMiddleCId = 34;
}

namespace ActsTrk {

  StatusCode MuonBlueprintNodeBuilder::initialize(){
        ATH_CHECK(detStore()->retrieve(m_detMgr));
        return StatusCode::SUCCESS;
  }


std::shared_ptr<Acts::Experimental::BlueprintNode> MuonBlueprintNodeBuilder::buildBlueprintNode(const Acts::GeometryContext& gctx, std::shared_ptr<Acts::Experimental::BlueprintNode>&& childNode) {

std::variant<MuonChamberSet, MuonSectorSet> elements;
std::variant<MuonChamberSet, MuonSectorSet> barrelStations, endcapAStations, endcapCStations, endcapMiddleAStations, endcapMiddleCStations;

if (m_useSectors) {
  elements = m_detMgr->getAllSectors();
} else {
  elements = m_detMgr->getAllChambers();
}

std::visit([&](auto& elems) {
  using SetType = std::decay_t<decltype(elems)>;

  // Initialize station containers of the same type
  SetType barrel, endcapA, endcapC, endcapMiddleA, endcapMiddleC;

  for (const auto& element : elems) {
    if (isElementInTheStation(*element,
          {StIdx::BI, StIdx::BM, StIdx::BO, StIdx::BE, StIdx::EE, StIdx::EI},
          EndcapSide::Both)) {
      barrel.push_back(element);
    } else if (isElementInTheStation(*element, {StIdx::EO}, EndcapSide::A)) {
      endcapA.push_back(element);
    } else if (isElementInTheStation(*element, {StIdx::EO}, EndcapSide::C)) {
      endcapC.push_back(element);
    } else if (isElementInTheStation(*element, {StIdx::EM}, EndcapSide::A)) {
      endcapMiddleA.push_back(element);
    } else if (isElementInTheStation(*element, {StIdx::EM}, EndcapSide::C)) {
      endcapMiddleC.push_back(element);
    } else {
      ATH_MSG_WARNING("Element " << element->identString()
                      << " not assigned to any station!");
    }
  }

  // Assign back into the outer variants
  barrelStations       = std::move(barrel);
  endcapAStations      = std::move(endcapA);
  endcapCStations      = std::move(endcapC);
  endcapMiddleAStations= std::move(endcapMiddleA);
  endcapMiddleCStations= std::move(endcapMiddleC);

}, elements);

  // Top level node for the Muon system
auto muonNode = std::make_shared<Acts::Experimental::CylinderContainerBlueprintNode>("MuonNode", Acts::AxisDirection::AxisZ);

Acts::VolumeBoundFactory boundsFactory{};

auto barrelNode = buildMuonNode(gctx, barrelStations, "BI_BM_BO_EE_EI",Acts::GeometryIdentifier().withVolume(s_muonBarrelId), boundsFactory);
auto endcapANode = buildMuonNode(gctx, endcapAStations, "EO_A", Acts::GeometryIdentifier().withVolume(s_muonEndcapAId), boundsFactory);
auto endcapCNode = buildMuonNode(gctx, endcapCStations, "EO_C", Acts::GeometryIdentifier().withVolume(s_muonEndcapCId), boundsFactory);
auto endcapMiddleANode = buildMuonNode(gctx, endcapMiddleAStations, "EM_A", Acts::GeometryIdentifier().withVolume(s_muonEndcapMiddleAId), boundsFactory);
auto endcapMiddleCNode = buildMuonNode(gctx, endcapMiddleCStations, "EM_C", Acts::GeometryIdentifier().withVolume(s_muonEndcapMiddleCId), boundsFactory);

//Add to the muon barrel child node (e.g calo or Itk) - if existed
if(childNode){
  barrelNode->addChild(std::move(childNode));
}

muonNode->addChild(std::move(barrelNode));
muonNode->addChild(std::move(endcapANode));
muonNode->addChild(std::move(endcapCNode));
muonNode->addChild(std::move(endcapMiddleANode));
muonNode->addChild(std::move(endcapMiddleCNode));

return muonNode;

}

template<typename MuonElementsSet>
std::shared_ptr<Acts::Experimental::StaticBlueprintNode>
MuonBlueprintNodeBuilder::buildMuonNode(
    const Acts::GeometryContext& gctx,
    const MuonElementsSet& elements,
    const std::string& name,
    const Acts::GeometryIdentifier& id,
    Acts::VolumeBoundFactory& boundsFactory) const {

    const ActsTrk::GeometryContext* context = gctx.get<const ActsTrk::GeometryContext* >();
    std::vector<std::string> stationNames;
  
    std::vector<std::shared_ptr<Acts::Experimental::StaticBlueprintNode>> nodes;
  
    double innerRadius = 0.0;
    double outerRadius = std::numeric_limits<double>::lowest();
    double maxZ = std::numeric_limits<double>::lowest();
    double minZ = std::numeric_limits<double>::max();

    int chamberId = 1;
    
    std::visit([&](const auto& elems){
  
    for(const auto& element : elems){
      const Amg::Transform3D& transform = element->localToGlobalTransform(*context);
      std::string volName = element->identString();

      auto vol = std::make_unique<Acts::TrackingVolume>(
                                            transform,
                                            element->bounds(),
                                            volName);

      // Get material per chamber, blend it and place it in the center of the volume
      auto material = blendMaterial(*element);
      vol->addSurface(std::move(material));
      // //the chamber geometry id
      Acts::GeometryIdentifier chId = id.withLayer(chamberId++);
      vol->assignGeometryId(chId);

      std::pair<std::vector<staticNodePtr>,std::vector<surfacePtr>> innerStructure = getSensitiveElements(*context, *element, chId, boundsFactory);

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
                                {.visible = true}, {.visible = true});
        helper.write(volName + ".obj");
        helper.clear();
      }

      std::shared_ptr<Acts::Experimental::StaticBlueprintNode> node;

      const bool isSingleMdt =
          (element->readoutEles().size() == 1 &&
          element->readoutEles().front()->detectorType() == DetectorType::Mdt);

      if (isSingleMdt) {
          // Take ownership of the single existing node
          node = std::move(innerStructure.first.front());
          innerStructure.first.clear();
      } else {
          node = std::make_shared<Acts::Experimental::StaticBlueprintNode>(std::move(vol));

          for (auto& childNode : innerStructure.first) {
              node->addChild(std::move(childNode));
          }
          innerStructure.first.clear();
      }

      if (!node) {
          THROW_EXCEPTION("No blueprint node constructed");
      }

      nodes.emplace_back(std::move(node));

      }

    }, elements);

    double halfLengthZ = 0.5 * std::abs(maxZ - minZ);
    ATH_MSG_DEBUG("Inner radius: " << innerRadius);
    ATH_MSG_DEBUG("Outer radius: " << outerRadius);
    ATH_MSG_DEBUG("Max Z: " << maxZ);
    ATH_MSG_DEBUG("Min Z: " << minZ);
    ATH_MSG_DEBUG("Half length Z: " << halfLengthZ);

    Amg::Transform3D trf = Amg::getTranslateZ3D(halfLengthZ + minZ);

    auto bounds = boundsFactory.makeBounds<Acts::CylinderVolumeBounds>(innerRadius, outerRadius, halfLengthZ);
    auto volume = std::make_unique<Acts::TrackingVolume>(trf, bounds, name);
    volume->assignGeometryId(id);
    auto muonNode = std::make_shared<Acts::Experimental::StaticBlueprintNode>(std::move(volume));

    ATH_MSG_DEBUG("There are " << nodes.size() << " nodes");
    std::ranges::for_each(nodes, [&muonNode](auto& node) {
      muonNode->addChild(std::move(node));
    });
    return muonNode;
  }

template<typename T>
std::pair<std::vector<staticNodePtr>, std::vector<surfacePtr>>
MuonBlueprintNodeBuilder::getSensitiveElements(
    const ActsTrk::GeometryContext& gctx,
    const T& element,
    const Acts::GeometryIdentifier& chId,
    Acts::VolumeBoundFactory& boundsFactory) const {

  std::vector<staticNodePtr> readoutVolumes;
  std::vector<surfacePtr> readoutSurfaces;
  Acts::GeometryIdentifier::Value mdtId{1};

  for (const MuonGMR4::MuonReadoutElement* readoutEle : element.readoutEles()) {

    std::vector<surfacePtr> detSurfaces = readoutEle->getSurfaces();
    switch(readoutEle->detectorType()){
      case DetectorType::Mdt: {    
        const auto* mdtReadoutEle = static_cast<const MuonGMR4::MdtReadoutElement*>(readoutEle);
        const MuonGMR4::MdtReadoutElement::parameterBook& parameters{mdtReadoutEle->getParameters()};

          // get the transform to the sector's frame
          const Amg::Vector3D toChamber = element.globalToLocalTransform(gctx)*mdtReadoutEle->center(gctx);
          const Acts::Transform3 mdtTransform = element.localToGlobalTransform(gctx) * Amg::getTranslate3D(toChamber);

          // create the MDT multilayer volume with the dedicated builder
          Acts::Experimental::MultiWireVolumeBuilder::Config mwCfg;
          mwCfg.name = m_detMgr->idHelperSvc()->toStringDetEl(mdtReadoutEle->identify());
          mwCfg.mlSurfaces = detSurfaces;
          mwCfg.transform = mdtTransform;

          //check for rectangular or trapezoidal shape bounds 
          std::shared_ptr<Acts::VolumeBounds> mdtBounds{nullptr};
          
          if(std::abs(parameters.shortHalfX - parameters.longHalfX) < Acts::s_epsilon){

            mdtBounds = boundsFactory.makeBounds<Acts::CuboidVolumeBounds>(parameters.shortHalfX, parameters.halfY, parameters.halfHeight);
           
          } else {
            
              mdtBounds = boundsFactory.makeBounds<Acts::TrapezoidVolumeBounds>(parameters.shortHalfX, 
              parameters.longHalfX, parameters.halfY, parameters.halfHeight);
          }
          
          mwCfg.bounds = mdtBounds;
          using BoundsV = Acts::TrapezoidVolumeBounds::BoundValues;
          mwCfg.binning = {{{Acts::AxisDirection::AxisY, Acts::AxisBoundaryType::Bound,
                            -parameters.halfY,
                            parameters.halfY,
                            static_cast<std::size_t>(std::lround(2 * parameters.halfY / parameters.tubePitch))}, 2u},
                            {{Acts::AxisDirection::AxisZ, Acts::AxisBoundaryType::Bound,
                              -parameters.halfHeight,
                              parameters.halfHeight,
                              static_cast<std::size_t>(std::lround(2 * parameters.halfHeight / parameters.tubePitch))}, 1u}};
          Acts::Experimental::MultiWireVolumeBuilder mdtBuilder{mwCfg};
          std::unique_ptr<Acts::TrackingVolume> mdtVolume = mdtBuilder.buildVolume();

          mdtVolume->assignGeometryId(chId.withExtra(mdtId++));
          //create the blueprint node for the mdt multilayers
          std::shared_ptr<Acts::Experimental::StaticBlueprintNode> mdtNode = std::make_shared<Acts::Experimental::StaticBlueprintNode>(std::move(mdtVolume));
          mdtNode->setNavigationPolicyFactory(mdtBuilder.createNavigationPolicyFactory());
          readoutVolumes.push_back(std::move(mdtNode));

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


template<typename T>
std::shared_ptr<Acts::Surface>
MuonBlueprintNodeBuilder::blendMaterial(
    const T& element) const {

  const float thickness = element.halfZ() * 2;
  PVConstLink parentVolume = element.readoutEles().front()->getMaterialGeom()->getParent();
  GeoModelTools::GeoMaterialHelper geoMaterialHelper;
  std::pair<GeoModelTools::GeoMaterialPtr, double> geoMaterials = geoMaterialHelper.collectMaterial(parentVolume);

  const Acts::Material aMat = ActsPlugins::GeoModel::geoMaterialConverter(*geoMaterials.first);
  //rotate about the z axis
  auto constPtr = element.surface().getSharedPtr();
  //to assign the material shouldnt be const
  auto ptr = std::const_pointer_cast<Acts::Surface>(constPtr);

  Acts::MaterialSlab slab{aMat, thickness};
  std::shared_ptr<Acts::HomogeneousSurfaceMaterial> material = std::make_shared<Acts::HomogeneousSurfaceMaterial>(slab);
  ptr->assignSurfaceMaterial(material);
  return ptr;

}

template<typename T>
bool MuonBlueprintNodeBuilder::isElementInTheStation(const T& element, const std::vector<StIdx>& stationIndex, const EndcapSide& side) const {
  StIdx stationIdx = toStationIndex(element.chamberIndex());
  auto stationSide = element.side(); 
  bool matchesName = std::ranges::any_of(stationIndex.begin(), stationIndex.end(), [&](const auto& n){
        return stationIdx == n;
      });

  bool etaSignCorrect = ((stationSide > 0 && side == EndcapSide::A) || (stationSide < 0 && side == EndcapSide::C) || (side == EndcapSide::Both));
  return matchesName && etaSignCorrect;
}


} //namespace ActsTrk