/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonBlueprintNodeBuilder.h"

#include <Acts/Geometry/BlueprintNode.hpp>
#include <Acts/Geometry/StaticBlueprintNode.hpp>
#include <Acts/Geometry/CylinderVolumeBounds.hpp>
#include <Acts/Geometry/ContainerBlueprintNode.hpp>
#include <Acts/Geometry/MultiWireVolumeBuilder.hpp>
#include <Acts/Surfaces/TrapezoidBounds.hpp>
#include <Acts/Geometry/MaterialDesignatorBlueprintNode.hpp>
#include <Acts/Geometry/GeometryIdentifierBlueprintNode.hpp>
#include <Acts/Geometry/VolumeAttachmentStrategy.hpp>
#include <Acts/Geometry/VolumeResizeStrategy.hpp>
#include <Acts/Geometry/TrackingVolume.hpp>
#include <Acts/Geometry/TrapezoidVolumeBounds.hpp>
#include <Acts/Geometry/CuboidVolumeBounds.hpp>
#include <Acts/Geometry/DiamondVolumeBounds.hpp>
#include <Acts/Surfaces/PlaneSurface.hpp>
#include <Acts/Surfaces/CylinderSurface.hpp>
#include <Acts/Surfaces/DiscSurface.hpp>
#include <Acts/Surfaces/RadialBounds.hpp>
#include <ActsPlugins/GeoModel/GeoModelMaterialConverter.hpp>
#include <Acts/Visualization/ObjVisualization3D.hpp>
#include <Acts/Visualization/GeometryView3D.hpp>
#include <Acts/Surfaces/LineBounds.hpp>
#include <Acts/Material/HomogeneousSurfaceMaterial.hpp>
#include <Acts/Material/ProtoSurfaceMaterial.hpp>
#include <Acts/Surfaces/RectangleBounds.hpp>

#include <MuonReadoutGeometryR4/Chamber.h>
#include <MuonReadoutGeometryR4/SpectrometerSector.h>
#include <MuonReadoutGeometryR4/MdtReadoutElement.h>
#include <MuonStationIndex/MuonStationIndex.h>

#include "GeoModelValidation/GeoMaterialHelper.h"

using namespace Acts::UnitLiterals;
using namespace Muon::MuonStationIndex;
namespace {

  //Helper function to configure a material node with the correct Faces of the chambers' tracking volumes
  void configureMaterialFaces(
    Acts::Experimental::MaterialDesignatorBlueprintNode& node,
    const Acts::VolumeBounds& bounds,
    std::shared_ptr<const Acts::ISurfaceMaterial> material){

    switch (bounds.type()) {
      case Acts::VolumeBounds::BoundsType::eCuboid: {
        node.configureFace(
            Acts::CuboidVolumeBounds::Face::NegativeZFace, material);
        node.configureFace(
            Acts::CuboidVolumeBounds::Face::PositiveZFace, material);
        break;
      }
      case Acts::VolumeBounds::BoundsType::eTrapezoid: {
        node.configureFace(
            Acts::TrapezoidVolumeBounds::Face::NegativeZFaceXY, material);
        node.configureFace(
            Acts::TrapezoidVolumeBounds::Face::PositiveZFaceXY, material);
        break;
      }
      case Acts::VolumeBounds::BoundsType::eDiamond: {
        node.configureFace(
            Acts::DiamondVolumeBounds::Face::NegativeZFaceXY, material);
        node.configureFace(
            Acts::DiamondVolumeBounds::Face::PositiveZFaceXY, material);
        break;
      }
      default:
        THROW_EXCEPTION(
            "Unsupported volume bounds for material configuration");
    }

  }
  // null check for variant of shared ptrs
  template<typename... T>
  bool isNullVariant(std::variant<T...> variant) {
    return std::visit([](auto&& ptr) { 
      return !ptr; 
    }, variant);
  }

}

namespace ActsTrk {

  StatusCode MuonBlueprintNodeBuilder::initialize(){
        ATH_CHECK(detStore()->retrieve(m_detMgr));
        return StatusCode::SUCCESS;
  }


std::shared_ptr<Acts::Experimental::BlueprintNode> MuonBlueprintNodeBuilder::buildBlueprintNode(const Acts::GeometryContext& gctx, std::shared_ptr<Acts::Experimental::BlueprintNode>&& childNode) {

EnvelopeSet_t elements;
EnvelopeSet_t barrelStations, endcapOuterAStations, endcapOuterCStations, 
              endcapMiddleAStations, endcapMiddleCStations;

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
  endcapOuterAStations = std::move(endcapA);
  endcapOuterCStations = std::move(endcapC);
  endcapMiddleAStations = std::move(endcapMiddleA);
  endcapMiddleCStations = std::move(endcapMiddleC);
}, elements);

  // Top level node for the Muon system
auto muonNode = std::make_shared<Acts::Experimental::CylinderContainerBlueprintNode>("MuonNode", Acts::AxisDirection::AxisZ);

Acts::VolumeBoundFactory boundsFactory{};
using namespace ActsTrk::detail::GeoVolIds;
auto barrelNode = buildMuonNode(gctx, barrelStations, "BI_BM_BO_EE_EI", Acts::GeometryIdentifier().withVolume(s_muonBarrelId), boundsFactory, {ChIdx::BIS, ChIdx::BML, ChIdx::BOL, 
                                                                                                                                               ChIdx::EIS, ChIdx::EIL});
auto endcapANode = buildMuonNode(gctx, endcapOuterAStations, "EO_A", Acts::GeometryIdentifier().withVolume(s_muonEndcapAId), boundsFactory);
auto endcapCNode = buildMuonNode(gctx, endcapOuterCStations, "EO_C", Acts::GeometryIdentifier().withVolume(s_muonEndcapCId), boundsFactory);
auto endcapMiddleANode = buildMuonNode(gctx, endcapMiddleAStations, "EM_A", Acts::GeometryIdentifier().withVolume(s_muonEndcapMiddleAId), boundsFactory, {ChIdx::EML, ChIdx::EMS});
auto endcapMiddleCNode = buildMuonNode(gctx, endcapMiddleCStations, "EM_C", Acts::GeometryIdentifier().withVolume(s_muonEndcapMiddleCId), boundsFactory, {ChIdx::EML, ChIdx::EMS});

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

std::shared_ptr<Acts::Experimental::StaticBlueprintNode>
MuonBlueprintNodeBuilder::buildMuonNode(const Acts::GeometryContext& gctx, 
                                        const EnvelopeSet_t& elements, 
                                        const std::string& name, 
                                        const Acts::GeometryIdentifier& id,
                                        Acts::VolumeBoundFactory& boundsFactory, 
                                        const std::vector<ChIdx>& passiveStationIds) const {

    const ActsTrk::GeometryContext* context = gctx.get<const ActsTrk::GeometryContext* >();
    std::vector<std::string> stationNames;

    //build the material nodes that will have as children the static nodes bult from the tracking volumes of the chambers   
    std::vector<std::variant<staticNodePtr, materialNodePtr>> nodes;                                       
  
    double innerRadius{0.0};
    double outerRadius{std::numeric_limits<double>::lowest()};
    double maxZ{std::numeric_limits<double>::lowest()};
    double minZ{std::numeric_limits<double>::max()};
    int chamberId = 1;  
    std::vector<std::shared_ptr<Acts::Surface>> passiveSurfaces;

    std::visit([&](const auto& elems){
    
    using SetType = std::decay_t<decltype(elems)>;
    std::unordered_map<unsigned int, SetType> elementsPerStation;
  
    for(const auto& element : elems){
      std::unique_ptr<Acts::TrackingVolume> vol{};
      if (m_alignableVolumes) {
          vol = std::make_unique<Acts::TrackingVolume>(*element->boundingVolume(*context),
                                                       element->identString());
      } else {
          vol = std::make_unique<Acts::TrackingVolume>(element->localToGlobalTransform(*context),
                                                      element->bounds(),
                                                      element->identString());
      }
      // //the chamber geometry id
      Acts::GeometryIdentifier chId = id.withLayer(chamberId++);
      vol->assignGeometryId(chId);
      //build the inner structure of the chamber this will return inner sensitive surfaces 
      //or volumes that have already constructed as blueptint nodes and will nbe assigned as children to the element node
      std::pair<std::vector<blueprintNodePtr>,std::vector<surfacePtr>> innerStructure = getSensitiveElements(*context, *element, chId, boundsFactory);
      for(auto& surface: innerStructure.second){
        vol->addSurface(surface);
      }

      //calculate the bounds of the cylinder container
      for(const auto& surface: vol->volumeBounds().orientedSurfaces(vol->localToGlobalTransform(gctx))) {
        const auto& surfaceRepr = (*surface.surface);
        const Acts::Polyhedron& polyhedron = surfaceRepr.polyhedronRepresentation(gctx);
        const Amg::Vector3D& center = surfaceRepr.center(gctx);

        maxZ = std::max(maxZ, center.z());
        minZ = std::min(minZ, center.z());

        // Outer radius needs to be treated differently due to curvature of cylindrical surface
        for(const Amg::Vector3D& vertex: polyhedron.vertices){
          outerRadius = std::max(outerRadius, vertex.perp());
        }
      }

      std::variant<staticNodePtr, materialNodePtr> chamberNode;      
      const bool isSingleMdt =
          (element->readoutEles().size() == 1 &&
          element->readoutEles().front()->detectorType() == DetectorType::Mdt);
      //for the single MDT elements we build the material node during the volume construction 
      //and the node returned is the material node already
      if (isSingleMdt) {
          // Take ownership of the single existing node where we have already included the static node as child
          // if we allow active material assignment the node is the material node, otherwise it is the static node
        chamberNode = buildChamberNode(innerStructure.first.front());
      } else {
        //for the non single MDT elements we build the material node that has as child the static node representing the chamber volume if we build with material
        // or it is a static node with the other static nodes as children if not active material is assigned 
        chamberNode = buildChamberNode(element, vol, innerStructure.first);
        innerStructure.first.clear();
      }
      if (isNullVariant(chamberNode)) {
        THROW_EXCEPTION("No blueprint node constructed");
      }     
      nodes.push_back(std::move(chamberNode));
  
      //keep the elements of the stations we want to assign passive material surfaces     
      if(!Acts::rangeContainsValue(passiveStationIds, element->chamberIndex())){
        continue;
      }

      DetIdx detIdx = toDetectorRegionIndex(element->chamberIndex(), element->side());    
      elementsPerStation[regionChamberHash(detIdx, element->chamberIndex())].push_back(element);
    }
    //construct the surfaces we want to map passive material on using the elements' geometrical parameters
    passiveSurfaces = getPassiveMaterialSurfaces(gctx, std::move(elementsPerStation));

    }, elements);

    double halfLengthZ = 0.5 * std::abs(maxZ - minZ);
    ATH_MSG_DEBUG("Inner radius: " << innerRadius<<", outer radius: " << outerRadius
                 <<", max Z: " << maxZ<<", min Z: " << minZ<<", half length Z: " << halfLengthZ);

    Amg::Transform3D trf = Amg::getTranslateZ3D(halfLengthZ + minZ);

    auto bounds = boundsFactory.makeBounds<Acts::CylinderVolumeBounds>(innerRadius, outerRadius, halfLengthZ);
    auto volume = std::make_unique<Acts::TrackingVolume>(trf, bounds, name);
    volume->assignGeometryId(id);
    
    //put the passive material surfaces into the volume
    std::ranges::for_each(passiveSurfaces, [&volume](auto& surf){
      volume->addSurface(surf);
    });

    auto muonNode = std::make_shared<Acts::Experimental::StaticBlueprintNode>(std::move(volume));
    ATH_MSG_DEBUG("There are " << nodes.size() << " nodes");
    //loop through the nodes-material pairs to add the nodes to the muon node and assign the material to the faces 
    std::ranges::for_each(nodes, [&muonNode](auto& nodeVariant){
      std::visit([&](auto&& ptr) {
        muonNode->addChild(ptr);
      }, nodeVariant);     
    });
    return muonNode;
  }


std::variant<MuonBlueprintNodeBuilder::staticNodePtr, MuonBlueprintNodeBuilder::materialNodePtr>
MuonBlueprintNodeBuilder::buildChamberNode(const blueprintNodePtr& chamberVolumeNode) const{
    if (m_assignActiveMaterial) {
        auto materialNode = std::dynamic_pointer_cast<Acts::Experimental::MaterialDesignatorBlueprintNode>(chamberVolumeNode);  
        return materialNode;
    }
    auto staticNode = std::dynamic_pointer_cast<Acts::Experimental::StaticBlueprintNode>(chamberVolumeNode);
    return staticNode;   
}

template<typename T>
std::variant<MuonBlueprintNodeBuilder::staticNodePtr, MuonBlueprintNodeBuilder::materialNodePtr>
MuonBlueprintNodeBuilder::buildChamberNode(const T& element,
                                          std::unique_ptr<Acts::TrackingVolume>& vol,
                                          const std::vector<blueprintNodePtr>& innerStructure) const{
  //copy of the volume bounds
  const Acts::VolumeBounds& bounds = vol->volumeBounds();
  staticNodePtr staticNode = std::make_shared<Acts::Experimental::StaticBlueprintNode>(std::move(vol));
  for (auto& childNode : innerStructure) {
      auto node = std::dynamic_pointer_cast<Acts::Experimental::StaticBlueprintNode>(childNode);
      if (node) {
          staticNode->addChild(std::move(node));
      } 
  }
  if(!m_assignActiveMaterial){
    return staticNode;
  }
  auto materialNode = std::make_shared<Acts::Experimental::MaterialDesignatorBlueprintNode>(element->identString() + "_MaterialNode");
  configureMaterialFaces(*materialNode, bounds, getActiveMaterial(*element));
  materialNode->addChild(staticNode);
  return materialNode;
}


template<typename T>
MuonBlueprintNodeBuilder::BluePrintSurfPairs_t  
  MuonBlueprintNodeBuilder::getSensitiveElements(const ActsTrk::GeometryContext& gctx,
                                                 const T& element, 
                                                 const Acts::GeometryIdentifier& chId,
                                                 Acts::VolumeBoundFactory& boundsFactory) const 
      requires(std::is_same_v<T, MuonGMR4::Chamber> || std::is_same_v<T, MuonGMR4::SpectrometerSector>){
  
  std::vector<blueprintNodePtr> readoutVolumes;
  std::vector<surfacePtr> readoutSurfaces;
  Acts::GeometryIdentifier::Value mdtId{1};

  for (const MuonGMR4::MuonReadoutElement* readoutEle : element.readoutEles()) {

    std::vector<surfacePtr> detSurfaces = readoutEle->getSurfaces();
    switch(readoutEle->detectorType()){
      case DetectorType::Mdt: {    
          const auto* mdtReadoutEle = static_cast<const MuonGMR4::MdtReadoutElement*>(readoutEle);
          const MuonGMR4::MdtReadoutElement::parameterBook& parameters{mdtReadoutEle->getParameters()};

          std::unique_ptr<ActsTrk::VolumePlacement> placement{};

          // create the MDT multilayer volume with the dedicated builder
          Acts::Experimental::MultiWireVolumeBuilder::Config mwCfg;
          mwCfg.name = m_detMgr->idHelperSvc()->toStringDetEl(mdtReadoutEle->identify());
          mwCfg.mlSurfaces = detSurfaces;
          mwCfg.transform = readoutEle->localToGlobalTransform(gctx);

          //initialize a nullptr material node which will be filled in the case of single MDT readout elements 
          //and used to assign the material to the volume and add the static node as child of the material node
          std::shared_ptr<Acts::Experimental::MaterialDesignatorBlueprintNode> mdtMaterialNode;

          //special treatment of BIS78 MDT multilayer
          //use different shape because of clashes with EIL chambers 
          if(isBIS78(readoutEle) && mdtReadoutEle->multilayer() == 2){

            
            //find the minimum and the maximum tube length (x dimension of the diamond bounds)
            std::vector<double> tubeLengths;
            tubeLengths.reserve(mdtReadoutEle->numTubesInLay());
            for(std::size_t tube = 1; tube < mdtReadoutEle->numTubesInLay(); ++tube){
              const IdentifierHash tubeHash = MuonGMR4::MdtReadoutElement::measurementHash(1,tube);
              const auto& surface = mdtReadoutEle->surface(tubeHash);
              const auto& lBounds = static_cast<const Acts::LineBounds&>(surface.bounds());
              using BoundEnum = Acts::LineBounds::BoundValues;
              const double tubeLength = 2.*lBounds.get(BoundEnum::eHalfLengthZ);
              tubeLengths.push_back(tubeLength);
            }
            auto [minX,maxX] = std::ranges::minmax_element(tubeLengths);
            int nSmallTubes = std::count_if(tubeLengths.begin(), tubeLengths.end(), [minX](double length){
              return std::abs(*minX-length) < Acts::s_epsilon;
            });

            //create the diamond bounds for the volume
            constexpr double extraMargin = 1._cm;
            double y2 = (nSmallTubes+1.)*parameters.tubePitch;
            double y1 = 2.*parameters.halfY + extraMargin - y2;
            if (m_alignableVolumes) {         
                placement = std::make_unique<ActsTrk::VolumePlacement>(*readoutEle, 
                                                                       Amg::getTranslateY3D(parameters.halfY + extraMargin -y2));   
            }
            mwCfg.transform = mwCfg.transform * Amg::getTranslateY3D(parameters.halfY + extraMargin - y2);
            mwCfg.bounds = boundsFactory.makeBounds<Acts::DiamondVolumeBounds>(0.5*(*maxX), 0.5*(*maxX), 0.5*(*minX), 
                                                                               y1, y2, parameters.halfHeight);                                                                     
                    
          } else {
            if (m_alignableVolumes){
                placement = std::make_unique<ActsTrk::VolumePlacement>(*readoutEle);
            }
            //check for rectangular or trapezoidal shape bounds
            if(std::abs(parameters.shortHalfX - parameters.longHalfX) < Acts::s_epsilon){
              mwCfg.bounds = boundsFactory.makeBounds<Acts::CuboidVolumeBounds>(parameters.shortHalfX, 
                                                                                parameters.halfY, 
                                                                                parameters.halfHeight);
            } else { 
              mwCfg.bounds = boundsFactory.makeBounds<Acts::TrapezoidVolumeBounds>(parameters.shortHalfX,
                                                                                   parameters.longHalfX,
                                                                                   parameters.halfY,
                                                                                   parameters.halfHeight);
            }
          }
          mwCfg.alignablePlacement = placement.get();
          if (m_alignableVolumes){
              element.addPlacement(std::move(placement));
          }
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
          // check if this is a single mdt (single multilayer) chamber so we assign the material directly to the multilayer
          if(element.readoutEles().size() == 1 && m_assignActiveMaterial){

            mdtMaterialNode = std::make_shared<Acts::Experimental::MaterialDesignatorBlueprintNode>(element.identString() + "_MaterialNode");
            configureMaterialFaces(*mdtMaterialNode, mdtVolume->volumeBounds(), getActiveMaterial(element));
            auto staticNode = std::make_shared<Acts::Experimental::StaticBlueprintNode>(std::move(mdtVolume));
            mdtMaterialNode->addChild(std::move(staticNode));
            readoutVolumes.push_back(std::move(mdtMaterialNode));
            break;
          }
          auto mdtNode = std::make_shared<Acts::Experimental::StaticBlueprintNode>(std::move(mdtVolume));
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
              THROW_EXCEPTION("Unknown detector type for readout element: " << readoutEle->detectorType());
              break;
     
    }
  }

  return std::make_pair(std::move(readoutVolumes), std::move(readoutSurfaces));
}


bool MuonBlueprintNodeBuilder::isBIS78(const MuonGMR4::MuonReadoutElement* element) const {   
  int stEta = element->stationEta();
  if(m_isRun4){
    stEta = std::abs(element->stationEta());
  }
  return  element->detectorType() == ActsTrk::DetectorType::Mdt && 
          element->chamberIndex() == ChIndex::BIS && 
          stEta >= 7;
  }

template<typename ElementSet_t>
std::vector<std::shared_ptr<Acts::Surface>> 
MuonBlueprintNodeBuilder::getPassiveMaterialSurfaces(
  const Acts::GeometryContext& gctx,
  const std::unordered_map<unsigned int, ElementSet_t>& elementsPerStation) const {

  if (!m_buildPassiveVolumes) {
      return {};
  }
  //this is a margin to put the surfaces along Z 
  //(a margin distance from the corresponding chamber's boundary surface)
  constexpr double margin{4._mm};

  std::vector<std::shared_ptr<Acts::Surface>> surfaces;
  surfaces.reserve(elementsPerStation.size());
  LayIdx layIdx = LayIdx::LayerIndexMax;
  DetIdx detIdx = DetIdx::DetectorRegionIndexMax;
  
  const ActsTrk::GeometryContext* context = gctx.get<const ActsTrk::GeometryContext* >();

  //lamda function to reject BIS78 chambers from the extension of the passive surface
  //otherwise they create overlap with the NSW sectors - stop a little bit before the cylinder of the passive surface
  const auto rejectBIS78 = [&](const MuonGMR4::MuonReadoutElement* readoutEle) {
    bool reject{false};
    if(readoutEle->chamberIndex() != ChIdx::BIS){ 
      return reject;
    }
    int stEta = readoutEle->stationEta();
    if(m_isRun4){
      stEta = std::abs(readoutEle->stationEta());
    }
    switch (readoutEle->detectorType()) {
      case DetectorType::Mdt: {
        const auto* techEle =
          static_cast<const MuonGMR4::MdtReadoutElement*>(readoutEle);
        if (techEle->multilayer() == 2 && stEta >= 7) {
          reject = true;
        }
        break;
      }
      case DetectorType::Rpc: {
        const auto* techEle =
        static_cast<const MuonGMR4::RpcReadoutElement*>(readoutEle);
        if (techEle->doubletZ() == 2 && stEta >= 7) {
          reject = true;
        }
        break;
      }
      default:
        break;
    }
    return reject;
  };
 
  for(const auto& [hash, elements] : elementsPerStation){

    //decompose the layer hash to the detector region idx and layer index
    const auto& [detIdxVal, chIdx] = decomposeRegionChamberHash(hash);
    layIdx = toLayerIndex(chIdx);
    detIdx = detIdxVal;

    double maxZ{std::numeric_limits<double>::lowest()};
    double minZ{std::numeric_limits<double>::max()};
    double rMin{std::numeric_limits<double>::max()};
    double rMax{std::numeric_limits<double>::lowest()};
       //loop through the elements of every station to construct the cylinder/disc  surfaces
    for(const auto& el : elements){  

      if(rejectBIS78(el->readoutEles().front())){
        continue;
      }
      const Amg::Transform3D& locToGlobal = el->localToGlobalTransform(*context);
      const auto& bounds = el->bounds();
      for(const auto& surface : bounds->orientedSurfaces(locToGlobal)){
        const auto& surfaceRepr = (*surface.surface);
        const Amg::Vector3D& center = surfaceRepr.center(gctx);
        rMin = std::min(rMin, center.perp());
        minZ = std::min(minZ, center.z());
        maxZ = std::max(maxZ, center.z());
        rMax = std::max(rMax, center.perp());
      }

    }
    double halfZ = 0.5*std::abs(maxZ-minZ);  
    Amg::Transform3D trf = Amg::Transform3D::Identity();
    double zShift{0.};
    // the chambers are groupd per chamber index and detector region(side) - 
    // we can use the first one for the distinction
    const auto& testCh = elements.front();
    int8_t side = testCh->side();
    switch (testCh->chamberIndex()) {      
      //small NSW sectors (disc passive surface in front of NSW and one in front of EMS)
      case ChIdx::EIS : 
      case ChIdx::EMS :{
        side > 0 ? zShift = minZ - margin : zShift = maxZ + margin;
        trf = Amg::getTranslateZ3D(zShift);
        auto surface = Acts::Surface::makeShared<Acts::DiscSurface>(trf, std::make_shared<Acts::RadialBounds>(rMin, rMax));
        const auto [nBins1, nBins2] = getMaterialBins(testCh->chamberIndex());
        surface->assignSurfaceMaterial(preparePassiveMaterial(surface->bounds(), nBins1, nBins2));
        surfaces.push_back(surface);
        break;
        //large sectors (disc passive surface after NSW/EIL and after EML)
    } case ChIdx::EIL :
      case ChIdx::EML : {
        // HARDCODED!! (maybe think a better solution in the future) 
        // But for the EIL that we put after the EIS/EIL chambers we extend the radius of the disc surface 
        // in order to have a better coverage for the projections from EE
        if(testCh->chamberIndex() == ChIdx::EIL){
          rMax += 60*margin;
        }
        side > 0 ? zShift = maxZ + margin : zShift = minZ - margin;
        trf = Amg::getTranslateZ3D(zShift);
        auto surface = Acts::Surface::makeShared<Acts::DiscSurface>(trf, 
                             std::make_shared<Acts::RadialBounds>(rMin, rMax));
        const auto [nBins1, nBins2] = getMaterialBins(testCh->chamberIndex());
        surface->assignSurfaceMaterial(preparePassiveMaterial(surface->bounds(), nBins1, nBins2));
        surfaces.push_back(surface);
        break;
      } case ChIdx::BIS :
        case ChIdx::BML :
        case ChIdx::BOL : {
        //hack for run3 because of overlaps with eta = -7 BIS chambers
      
        if(!m_isRun4 && testCh->chamberIndex() == ChIdx::BIS){
          halfZ -= 130.;
          
        }
        auto surface = Acts::Surface::makeShared<Acts::CylinderSurface>(trf, 
                             std::make_shared<Acts::CylinderBounds>(rMin - margin, halfZ));
        
        if (m_assignEmptyMaterial) {
          const auto [nBins1, nBins2] = getMaterialBins(testCh->chamberIndex());
          surface->assignSurfaceMaterial(preparePassiveMaterial(surface->bounds(), nBins1, nBins2));
        }
        surfaces.push_back(surface);
        break;
    } default :
        THROW_EXCEPTION("No implementation of passive material surface for this station!!!! - sorry :) ");
    }    
    ATH_MSG_VERBOSE("Putting passive material surface for station " << layerName(layIdx) << "/ "<< regionName(detIdx) << ": minZ = " << minZ << ", maxZ = " << maxZ<< "and radius "<< rMax);
  }

  if(msgLvl(MSG::VERBOSE)){
    std::stringstream stream{};
    for(const auto& surf : surfaces){
      stream<< " at position  : "<< Amg::toString(surf->center(gctx))
             << "with bounds "<< surf->bounds()<<std::endl;
    }
    ATH_MSG_VERBOSE("Constructed "<< surfaces.size()
        << " surfaces for passive material description : "<<std::endl<<stream.str());
  }

  return surfaces;
}

template<typename T>
std::shared_ptr<const Acts::ISurfaceMaterial>
MuonBlueprintNodeBuilder::getActiveMaterial(const T& element) const
  requires(std::is_same_v<T, MuonGMR4::Chamber> ||
           std::is_same_v<T, MuonGMR4::SpectrometerSector>) {
  
  const float thickness = element.halfZ();
  PVConstLink parentVolume = element.readoutEles().front()->getMaterialGeom()->getParent();
  GeoModelTools::GeoMaterialHelper geoMaterialHelper;
  std::pair<GeoModelTools::GeoMaterialPtr, double> geoMaterials = geoMaterialHelper.collectMaterial(parentVolume);

  const Acts::Material aMat = ActsPlugins::GeoModel::geoMaterialConverter(*geoMaterials.first);
  Acts::MaterialSlab slab{aMat, thickness};
  std::shared_ptr<Acts::HomogeneousSurfaceMaterial> material = std::make_shared<Acts::HomogeneousSurfaceMaterial>(slab);
  material->scale(0.5); // we want to split the active material in two and put it on the two faces of the chamber bounds

  return material;
  
}

template<typename T>
bool MuonBlueprintNodeBuilder::isElementInTheStation(const T& element, 
                                                     const std::vector<StIdx>& stationIndex, 
                                                     const EndcapSide side) const
  requires(std::is_same_v<T, MuonGMR4::Chamber> ||
           std::is_same_v<T, MuonGMR4::SpectrometerSector>) {
  bool etaSignCorrect = (side == EndcapSide::Both) ||
                        (side == EndcapSide::A && element.side() > 0) || 
                        (side == EndcapSide::C && element.side() < 0);
  return etaSignCorrect && 
         Acts::rangeContainsValue(stationIndex, toStationIndex(element.chamberIndex()));
}


std::shared_ptr<Acts::ISurfaceMaterial> 
    MuonBlueprintNodeBuilder::preparePassiveMaterial(const Acts::SurfaceBounds& bounds,
                                                     const std::size_t nBins1,
                                                     const std::size_t nBins2) const {
    if (nBins1 == 0 || nBins2 == 0) {
      ATH_MSG_ERROR("Cannot create material for "<<bounds
        <<" as one of the bin dimensions is zero. nBins1: "<<nBins1<<", nBins2: "<<nBins2);
      return nullptr;
    }
    if (nBins1 == 1 && nBins1 == nBins2) {
      return std::make_shared<Acts::HomogeneousSurfaceMaterial>();
    }

    std::vector<Acts::DirectedProtoAxis> pmBinning = {};

    switch (bounds.type()) {
       using enum Acts::SurfaceBounds::BoundsType;
      case eCylinder: {
          pmBinning = {{Acts::AxisDirection::AxisZ, Acts::AxisBoundaryType::Bound, nBins1},
                       {Acts::AxisDirection::AxisRPhi, Acts::AxisBoundaryType::Bound, nBins2}};        
          break;
      } case eDisc: {
          pmBinning = {{Acts::AxisDirection::AxisR, Acts::AxisBoundaryType::Bound, nBins1},
                       {Acts::AxisDirection::AxisPhi, Acts::AxisBoundaryType::Bound, nBins2}};
      
          break;
      } default:
        ATH_MSG_ERROR("Unsupoorted type "<<bounds<<".");
        return nullptr;
    }
    return std::make_shared<Acts::ProtoGridSurfaceMaterial>(pmBinning);
}
std::pair<std::size_t, std::size_t> 
    MuonBlueprintNodeBuilder::getMaterialBins(const ChIndex chIdx) const {
    switch(chIdx) {
      using enum ChIndex;
      case BIS:
      case BIL:
        return std::make_pair(1ul * m_nZBinsBI, 1ul * m_nPhiBinsBI);
      case BML:
      case BMS:
        return std::make_pair(1ul * m_nZBinsBM, 1ul * m_nPhiBinsBM);
      case BOL:
      case BOS:
        return std::make_pair(1ul * m_nZBinsBO, 1ul * m_nPhiBinsBO);
      case EIS:
        return std::make_pair(1ul*  m_nRBinsEI1, 1ul* m_nPhiBinsEI1);
      case EIL:
        return std::make_pair(1ul*  m_nRBinsEI2, 1ul* m_nPhiBinsEI2);
      case EMS:
        return std::make_pair(1ul*  m_nRBinsEM1, 1ul* m_nPhiBinsEM1);
      case EML:
        return std::make_pair(1ul*  m_nRBinsEM2, 1ul* m_nPhiBinsEM2);
      default:
        THROW_EXCEPTION("getMaterialBins() - "<<chName(chIdx)<<" is not yet implemented");
    }
    return std::make_pair(0ul, 0ul);
}
} //namespace ActsTrk
