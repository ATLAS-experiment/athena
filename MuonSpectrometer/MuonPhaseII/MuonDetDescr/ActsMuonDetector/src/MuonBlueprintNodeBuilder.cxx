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
#include <Acts/Geometry/DiamondVolumeBounds.hpp>
#include <Acts/Surfaces/PlaneSurface.hpp>
#include <Acts/Surfaces/CylinderSurface.hpp>
#include <ActsPlugins/GeoModel/GeoModelMaterialConverter.hpp>
#include <Acts/Visualization/ObjVisualization3D.hpp>
#include <Acts/Visualization/GeometryView3D.hpp>


#include <MuonReadoutGeometryR4/Chamber.h>
#include <MuonReadoutGeometryR4/SpectrometerSector.h>
#include <MuonReadoutGeometryR4/MdtReadoutElement.h>
#include <MuonStationIndex/MuonStationIndex.h>

#include "GeoModelValidation/GeoMaterialHelper.h"

using namespace Acts::UnitLiterals;
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
auto barrelNode = buildMuonNode(gctx, barrelStations, "BI_BM_BO_EE_EI", Acts::GeometryIdentifier().withVolume(s_muonBarrelId), boundsFactory, {StIdx::BI, StIdx::BM, StIdx::BO});
auto endcapANode = buildMuonNode(gctx, endcapOuterAStations, "EO_A", Acts::GeometryIdentifier().withVolume(s_muonEndcapAId), boundsFactory, {StIdx::EO});
auto endcapCNode = buildMuonNode(gctx, endcapOuterCStations, "EO_C", Acts::GeometryIdentifier().withVolume(s_muonEndcapCId), boundsFactory, {StIdx::EO});
auto endcapMiddleANode = buildMuonNode(gctx, endcapMiddleAStations, "EM_A", Acts::GeometryIdentifier().withVolume(s_muonEndcapMiddleAId), boundsFactory, {StIdx::EM});
auto endcapMiddleCNode = buildMuonNode(gctx, endcapMiddleCStations, "EM_C", Acts::GeometryIdentifier().withVolume(s_muonEndcapMiddleCId), boundsFactory, {StIdx::EM});

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
                                        const std::vector<StIdx>& passiveStationIds) const {

    const ActsTrk::GeometryContext* context = gctx.get<const ActsTrk::GeometryContext* >();
    std::vector<std::string> stationNames;
  
    std::vector<std::shared_ptr<Acts::Experimental::StaticBlueprintNode>> nodes;
  
    double innerRadius{0.0};
    double outerRadius{std::numeric_limits<double>::lowest()};
    double maxZ{std::numeric_limits<double>::lowest()};
    double minZ{std::numeric_limits<double>::max()};
    int chamberId = 1;  
    std::vector<std::shared_ptr<Acts::Surface>> passiveSurfaces;
    passiveSurfaces.reserve(passiveStationIds.size());

    std::visit([&](const auto& elems){
    
    using SetType = std::decay_t<decltype(elems)>;
    std::unordered_map<StIdx, SetType> elementsPerStation;
    passiveSurfaces.reserve(passiveStationIds.size());
  
    for(const auto& element : elems){
      auto vol = std::make_unique<Acts::TrackingVolume>(*element->boundingVolume(*context),
                                                        element->identString());     
      // //the chamber geometry id
      Acts::GeometryIdentifier chId = id.withLayer(chamberId++);
      vol->assignGeometryId(chId);
      std::pair<std::vector<staticNodePtr>,std::vector<surfacePtr>> innerStructure = getSensitiveElements(*context, *element, chId, boundsFactory);
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
     
      std::shared_ptr<Acts::Experimental::StaticBlueprintNode> node;
      const bool isSingleMdt =
          (element->readoutEles().size() == 1 &&
          element->readoutEles().front()->detectorType() == DetectorType::Mdt);

      if (isSingleMdt) {
          // Take ownership of the single existing node
          node = std::move(innerStructure.first.front());
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

      //keep the elements of the stations we want to assign passive material surfaces
      if(!Acts::rangeContainsValue(passiveStationIds, toStationIndex(element->chamberIndex()))){
        continue;
      }
      elementsPerStation[toStationIndex(element->chamberIndex())].push_back(element);
    }
    //construct the surfaces we want to map passive material on
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
    std::ranges::for_each(nodes, [&muonNode](auto& node) {
      muonNode->addChild(std::move(node));
    });
    return muonNode;
  }

template<typename T>
MuonBlueprintNodeBuilder::BluePrintSurfPairs_t  
  MuonBlueprintNodeBuilder::getSensitiveElements(const ActsTrk::GeometryContext& gctx,
                                                 const T& element, 
                                                 const Acts::GeometryIdentifier& chId,
                                                 Acts::VolumeBoundFactory& boundsFactory) const 
      requires(std::is_same_v<T, MuonGMR4::Chamber> || std::is_same_v<T, MuonGMR4::SpectrometerSector>){
  
  std::vector<staticNodePtr> readoutVolumes;
  std::vector<surfacePtr> readoutSurfaces;
  Acts::GeometryIdentifier::Value mdtId{1};

  //lamda function for BIS78 MDT case
  auto isBIS78 = [](const MuonGMR4::MuonReadoutElement* rElem) {
    return rElem->detectorType() == DetectorType::Mdt &&
           rElem->chamberIndex() == Muon::MuonStationIndex::ChIndex::BIS && 
           std::abs(rElem->stationEta()) >= 7;
  };

  for (const MuonGMR4::MuonReadoutElement* readoutEle : element.readoutEles()) {

    std::vector<surfacePtr> detSurfaces = readoutEle->getSurfaces();
    switch(readoutEle->detectorType()){
      case DetectorType::Mdt: {    
        const auto* mdtReadoutEle = static_cast<const MuonGMR4::MdtReadoutElement*>(readoutEle);
        const MuonGMR4::MdtReadoutElement::parameterBook& parameters{mdtReadoutEle->getParameters()};

          // get the transform to the sector's frame
          const Amg::Vector3D toChamber = element.globalToLocalTransform(gctx)*mdtReadoutEle->center(gctx);
          Acts::Transform3 mdtTransform = element.localToGlobalTransform(gctx) * Amg::getTranslate3D(toChamber);

          // create the MDT multilayer volume with the dedicated builder
          Acts::Experimental::MultiWireVolumeBuilder::Config mwCfg;
          mwCfg.name = m_detMgr->idHelperSvc()->toStringDetEl(mdtReadoutEle->identify());
          mwCfg.mlSurfaces = detSurfaces;
          mwCfg.transform = mdtTransform;

          //special treatment of BIS78 MDT multilayer
          //use different shape because of clashes with EIL chambers 
          std::shared_ptr<Acts::VolumeBounds> mdtBounds{nullptr};
          if(isBIS78(readoutEle) && mdtReadoutEle->multilayer() == 2){
            
            //find the minimum and the maximum tube length (x dimension of the diamond bounds)
            std::vector<double> tubeLengths;
            tubeLengths.reserve(mdtReadoutEle->numTubesInLay());
            for(std::size_t tube = 1; tube < mdtReadoutEle->numTubesInLay(); ++tube){
              const IdentifierHash tubeHash = MuonGMR4::MdtReadoutElement::measurementHash(1,tube);
              double tubeLength = mdtReadoutEle->tubeLength(tubeHash);
              tubeLengths.push_back(tubeLength);
            }
            auto [minX,maxX] = std::ranges::minmax_element(tubeLengths);
            int nSmallTubes = std::count_if(tubeLengths.begin(), tubeLengths.end(), [minX](double length){
              return std::abs(*minX-length) < Acts::s_epsilon;
            });

            //create the diamond bounds for the volume
            double y2 = (nSmallTubes+1)*parameters.tubePitch ;
            double y1 = 2*parameters.halfY - y2;           
            mdtTransform = mdtTransform*Amg::getTranslateY3D(parameters.halfY-y2);    
            mdtBounds = boundsFactory.makeBounds<Acts::DiamondVolumeBounds>(0.5*(*maxX), 0.5*(*maxX), 0.5*(*minX), 
                                                                            y1, y2, parameters.halfHeight);            
          }else{
          //check for rectangular or trapezoidal shape bounds       
          if(std::abs(parameters.shortHalfX - parameters.longHalfX) < Acts::s_epsilon){
            mdtBounds = boundsFactory.makeBounds<Acts::CuboidVolumeBounds>(parameters.shortHalfX, parameters.halfY, parameters.halfHeight);           
          } else {            
            mdtBounds = boundsFactory.makeBounds<Acts::TrapezoidVolumeBounds>(parameters.shortHalfX, 
              parameters.longHalfX, parameters.halfY, parameters.halfHeight);
          }
          }          
          mwCfg.bounds = mdtBounds;
          mwCfg.transform = mdtTransform;
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

template<typename ElementSet_t>
std::vector<std::shared_ptr<Acts::Surface>> 
MuonBlueprintNodeBuilder::getPassiveMaterialSurfaces(
  const Acts::GeometryContext& gctx,
  const std::unordered_map<StIdx, ElementSet_t>& elementsPerStation) const {

  constexpr double margin{5._mm};

  const ActsTrk::GeometryContext* context = gctx.get<const ActsTrk::GeometryContext* >();

  std::vector<std::shared_ptr<Acts::Surface>> surfaces;
  surfaces.reserve(elementsPerStation.size());
  //temporary build barrel passive material surfaces - we will think for the endcap
  if(elementsPerStation.contains(StIdx::EO) || 
     elementsPerStation.contains(StIdx::EM)){
    ATH_MSG_WARNING("Do not assign passive material surfaes for the endcaps for now - will do later !");
    return surfaces;
  }
  

  //handle barrel passive surfaces for now only
  for(const auto& [idx, elements] : elementsPerStation){

    double maxZ{std::numeric_limits<double>::lowest()};
    double minZ{std::numeric_limits<double>::max()};
    double radius{std::numeric_limits<double>::max()};

    //loop through the elements of every station to construct the cylinder surfaces
    for(const auto& el : elements){
      const Amg::Transform3D locToGlobal = el->localToGlobalTransform(*context);
      Amg::Vector3D center = locToGlobal.translation();
      double r = center.perp();
      radius = std::min(radius, r - el->halfZ());
      minZ = std::min(minZ, center.z() - el->halfXLong());
      maxZ = std::max(maxZ, center.z() + el->halfXLong());
    
    }

    double halfZ = 0.5*std::abs(maxZ-minZ);  
    Amg::Transform3D trf = Amg::getTranslateZ3D(halfZ + minZ);
    std::shared_ptr<Acts::Surface> surface = Acts::Surface::makeShared<Acts::CylinderSurface>(
    trf, std::make_shared<Acts::CylinderBounds>(radius - margin, halfZ));
    surfaces.push_back(surface);
    ATH_MSG_VERBOSE("Putting passive material surface for station " << stName(idx) << ": minZ = " << minZ << ", maxZ = " << maxZ<< "and radius "<< radius);
  }

  if(msgLvl(MSG::VERBOSE)){
    std::stringstream stream{};
    for(const auto& surf : surfaces){
      stream<< " at position  : "<< Amg::toString(surf->center(gctx))<< "with bounds "<< surf->bounds()<<std::endl;
    }
    ATH_MSG_VERBOSE("Constructed "<< surfaces.size()<< " surfaces for passive material description : "<<std::endl<<stream.str());
  }
  return surfaces;
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


} //namespace ActsTrk