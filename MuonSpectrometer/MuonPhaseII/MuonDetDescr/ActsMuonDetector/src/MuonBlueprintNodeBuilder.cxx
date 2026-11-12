/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonBlueprintNodeBuilder.h"
#include "MuonReadoutGeometryR4/CylinderBoundsExpander.h"
#include "MuonReadoutGeometryR4/TrapezoidBoundsExpander.h"

#include "Acts/Geometry/BlueprintNode.hpp"
#include "Acts/Geometry/StaticBlueprintNode.hpp"
#include "Acts/Geometry/CylinderVolumeBounds.hpp"
#include "Acts/Geometry/ContainerBlueprintNode.hpp"
#include "Acts/Geometry/MultiWireVolumeBuilder.hpp"
#include "Acts/Surfaces/TrapezoidBounds.hpp"
#include "Acts/Geometry/MaterialDesignatorBlueprintNode.hpp"
#include "Acts/Geometry/GeometryIdentifierBlueprintNode.hpp"
#include "Acts/Geometry/VolumeAttachmentStrategy.hpp"
#include "Acts/Geometry/VolumeResizeStrategy.hpp"
#include "Acts/Geometry/TrackingVolume.hpp"
#include "Acts/Geometry/TrapezoidVolumeBounds.hpp"
#include "Acts/Geometry/CuboidVolumeBounds.hpp"
#include "Acts/Geometry/DiamondVolumeBounds.hpp"
#include "Acts/Surfaces/PlaneSurface.hpp"
#include "Acts/Surfaces/CylinderSurface.hpp"
#include "Acts/Surfaces/DiscSurface.hpp"
#include "Acts/Surfaces/RadialBounds.hpp"
#include "ActsPlugins/GeoModel/GeoModelMaterialConverter.hpp"
#include "Acts/Visualization/ObjVisualization3D.hpp"
#include "Acts/Visualization/GeometryView3D.hpp"
#include "Acts/Surfaces/LineBounds.hpp"
#include "Acts/Material/HomogeneousSurfaceMaterial.hpp"
#include "Acts/Material/ProtoSurfaceMaterial.hpp"
#include "Acts/Utilities/AxisSpec.hpp"
#include "Acts/Utilities/MultiAxisSpec.hpp"
#include "Acts/Surfaces/RectangleBounds.hpp"
#include "Acts/Navigation/FrustumNavigationPolicy.hpp"
#include "ActsInterop/Logger.h"

#include "MuonReadoutGeometryR4/Chamber.h"
#include "MuonReadoutGeometryR4/SpectrometerSector.h"
#include "MuonReadoutGeometryR4/MdtReadoutElement.h"
#include "MuonStationIndex/MuonStationIndex.h"

#include "GeoModelValidation/GeoMaterialHelper.h"

using namespace Acts::UnitLiterals;
using namespace Muon::MuonStationIndex;
namespace {

  //Helper function to configure a material node with the correct Faces of the chambers' tracking volumes
  void configureMaterialFaces(
    Acts::MaterialDesignatorBlueprintNode& node,
    const Acts::VolumeBounds& bounds,
    std::shared_ptr<const Acts::ISurfaceMaterial> material){

    switch (bounds.type()) {
      case Acts::VolumeBounds::BoundsType::eCuboid: {
        node.configureFace(
            Acts::CuboidVolumeBounds::Face::NegativeZFace, material);
        node.configureFace(
            Acts::CuboidVolumeBounds::Face::PositiveZFace, std::move(material));
        break;
      }
      case Acts::VolumeBounds::BoundsType::eTrapezoid: {
        node.configureFace(
            Acts::TrapezoidVolumeBounds::Face::NegativeZFaceXY, material);
        node.configureFace(
            Acts::TrapezoidVolumeBounds::Face::PositiveZFaceXY, std::move(material));
        break;
      }
      case Acts::VolumeBounds::BoundsType::eDiamond: {
        node.configureFace(
            Acts::DiamondVolumeBounds::Face::NegativeZFaceXY, material);
        node.configureFace(
            Acts::DiamondVolumeBounds::Face::PositiveZFaceXY, std::move(material));
        break;
      }
      default:
        THROW_EXCEPTION(
            "Unsupported volume bounds for material configuration");
    }
  }
}

namespace ActsTrk {
  using BlueprintNodePtr_t = MuonBlueprintNodeBuilder::BlueprintNodePtr_t;
  StatusCode MuonBlueprintNodeBuilder::initialize(){
        ATH_CHECK(detStore()->retrieve(m_detMgr));
        return StatusCode::SUCCESS;
  }


BlueprintNodePtr_t MuonBlueprintNodeBuilder::buildBlueprintNode(const Acts::GeometryContext& tgContext, BlueprintNodePtr_t&& childNode) {

EnvelopeSet_t elements;
EnvelopeSet_t barrelStations, endcapOuterAStations, endcapOuterCStations, 
              endcapMiddleAStations, endcapMiddleCStations;

if (m_useSectors) {
  elements = m_detMgr->getAllSectors();
} else {
  elements = m_detMgr->getAllChambers();
}

std::visit([&](auto& elems) {
  using Set_t = std::decay_t<decltype(elems)>;

  // Initialize station containers of the same type
  Set_t barrel, endcapA, endcapC, endcapMiddleA, endcapMiddleC;

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
auto muonNode = std::make_shared<Acts::CylinderContainerBlueprintNode>("MuonNode", Acts::AxisDirection::AxisZ);

Acts::VolumeBoundFactory boundsFactory{};
using namespace ActsTrk::detail::GeoVolIds;
auto barrelNode = buildMuonNode(tgContext, barrelStations, "BI_BM_BO_EE_EI", Acts::GeometryIdentifier().withVolume(s_muonBarrelId), boundsFactory, {ChIdx::BIS, ChIdx::BML, ChIdx::BOL, 
                                                                                                                                               ChIdx::EIS, ChIdx::EIL});
auto endcapANode = buildMuonNode(tgContext, endcapOuterAStations, "EO_A", Acts::GeometryIdentifier().withVolume(s_muonEndcapAId), boundsFactory);
auto endcapCNode = buildMuonNode(tgContext, endcapOuterCStations, "EO_C", Acts::GeometryIdentifier().withVolume(s_muonEndcapCId), boundsFactory);
auto endcapMiddleANode = buildMuonNode(tgContext, endcapMiddleAStations, "EM_A", Acts::GeometryIdentifier().withVolume(s_muonEndcapMiddleAId), boundsFactory, {ChIdx::EML, ChIdx::EMS});
auto endcapMiddleCNode = buildMuonNode(tgContext, endcapMiddleCStations, "EM_C", Acts::GeometryIdentifier().withVolume(s_muonEndcapMiddleCId), boundsFactory, {ChIdx::EML, ChIdx::EMS});
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

BlueprintNodePtr_t
MuonBlueprintNodeBuilder::buildMuonNode(const Acts::GeometryContext& tgContext, 
                                        const EnvelopeSet_t& elements, 
                                        const std::string& nodeName, 
                                        const Acts::GeometryIdentifier& volumeId,
                                        Acts::VolumeBoundFactory& boundsFactory, 
                                        const std::vector<ChIdx>& passiveStationIds) const {

    const ActsTrk::GeometryContext* gctx = tgContext.get<const ActsTrk::GeometryContext* >();

    //build the material nodes that will have as children the static nodes bult from the tracking volumes of the chambers   
    std::vector<BlueprintNodePtr_t> nodes{};
  
    /** Cache the list of nodes around which we want to construct an envelope */
    EnvelopeNodeVec_t nodesForEnvelope{};
    
    MuonGMR4::CylinderBoundsExpander expandBounds{makeActsAthenaLogger(this, name())};
    expandBounds.rMin() = 0.;
    std::uint32_t chamberCounter{1u};  
    std::vector<std::shared_ptr<Acts::Surface>> passiveSurfaces{};

    std::visit([&](const auto& elems){
    
      using Set_t = std::decay_t<decltype(elems)>;
      std::vector<std::pair<unsigned, Set_t>> elementsPerStation{};
  
      for(const auto& element : elems) {
        std::unique_ptr<Acts::TrackingVolume> vol{};
        if (m_alignableVolumes) {
            vol = std::make_unique<Acts::TrackingVolume>(*element->boundingVolume(),
                                                         element->identString());
        } else {
            vol = std::make_unique<Acts::TrackingVolume>(element->localToGlobalTransform(tgContext),
                                                         element->bounds(),
                                                         element->identString());
        }
        // //the chamber geometry id
        Acts::GeometryIdentifier chamberId = volumeId.withLayer(chamberCounter++);
        vol->assignGeometryId(chamberId);
        //build the inner structure of the chamber this will return inner sensitive surfaces 
        //or volumes that have already constructed as blueptint nodes and will nbe assigned as children to the element node
        std::pair<std::vector<BlueprintNodePtr_t>,
                  std::vector<SurfacePtr_t>> innerStructure = getSensitiveElements(*gctx, *element, chamberId, boundsFactory);
        for(auto& surface: innerStructure.second){
          vol->addSurface(surface);
        }

        //calculate the bounds of the cylinder container
        expandBounds.expand(tgContext, *vol);

        BlueprintNodePtr_t chamberNode;
        const bool isSingleMdt = (element->readoutEles().size() == 1 &&
                                  element->readoutEles().front()->detectorType() == DetectorType::Mdt);
        //for the single MDT elements we build the material node during the volume construction 
        //and the node returned is the material node already
        if (isSingleMdt) {
            // Take ownership of the single existing node where we have already included the static node as child
            // if we allow active material assignment the node is the material node, otherwise it is the static node
          chamberNode = std::move(innerStructure.first.front());
        } else {
          //for the non single MDT elements we build the material node that has as child the static node representing the chamber volume if we build with material
          // or it is a static node with the other static nodes as children if not active material is assigned 
          chamberNode = buildChamberNode(element, vol, std::move(innerStructure.first));
        }
        if (!chamberNode) {
          THROW_EXCEPTION("No blueprint node constructed");
        }
        nodes.push_back(std::move(chamberNode));
        //keep the elements of the stations we want to assign passive material surfaces     
        if(!Acts::rangeContainsValue(passiveStationIds, element->chamberIndex())){
          continue;
        }
        const DetIdx detIdx = toDetectorRegionIndex(element->chamberIndex(), 
                                                    element->side());
        const LayIdx layIdx = toLayerIndex(element->chamberIndex());

        const unsigned regHash = sectorLayerHash(detIdx, layIdx);
        auto insert_itr = std::ranges::find_if(elementsPerStation, [regHash](const auto& hashElePair){
            return hashElePair.first == regHash;
        });

        if (insert_itr != elementsPerStation.end()) {
            insert_itr->second.push_back(element);
        } else {
          elementsPerStation.emplace_back(regHash, Set_t{element});
        }
      }
      //construct the surfaces we want to map passive material on using the elements' geometrical parameters
      passiveSurfaces = getPassiveMaterialSurfaces(tgContext, std::move(elementsPerStation));

    }, elements);

    auto volume = expandBounds.makeEnvelope(boundsFactory, nodeName);
    volume->assignGeometryId(volumeId);
    
    //put the passive material surfaces into the volume
    std::ranges::for_each(passiveSurfaces, [&volume](std::shared_ptr<Acts::Surface>& surf){
      volume->addSurface(std::move(surf));
    });
    ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<< " - There are " << nodes.size() << " nodes under volume "
              <<volume->volumeName()<<", "<<volume->geometryId()<<", "<<volume->volumeBounds());
    auto muonNode = std::make_shared<Acts::StaticBlueprintNode>(std::move(volume));
    if(m_useFrustumNav){
    	ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Using Frustum Navigation Policy");
      Acts::Experimental::FrustumNavigationPolicy::Config frustumConfig;
      muonNode->setNavigationPolicyFactory(std::make_shared<Acts::NavigationPolicyFactory>(Acts::NavigationPolicyFactory{}.add<Acts::Experimental::FrustumNavigationPolicy>(frustumConfig)));
    }

    //loop through the nodes-material pairs to add the nodes to the muon node and assign the material to the faces 
    std::ranges::for_each(nodes, [&](auto& ptr){ muonNode->addChild(std::move(ptr)); });
    return muonNode;
  }

template<typename T>
BlueprintNodePtr_t
MuonBlueprintNodeBuilder::buildChamberNode(const T& element,
                                          std::unique_ptr<Acts::TrackingVolume>& vol,
                                          std::vector<BlueprintNodePtr_t>&& innerStructure) const{
  //copy of the volume bounds
  const Acts::VolumeBounds& bounds = vol->volumeBounds();
  auto staticNode = std::make_shared<Acts::StaticBlueprintNode>(std::move(vol));
  for (auto& childNode : innerStructure) {
      staticNode->addChild(std::move(childNode));
  }
  if(!m_assignActiveMaterial){
    return staticNode;
  }
  auto materialNode = std::make_shared<Acts::MaterialDesignatorBlueprintNode>(std::format("{:}_MaterialNode",element->identString()));
  configureMaterialFaces(*materialNode, bounds, getActiveMaterial(*element));
  materialNode->addChild(staticNode);
  return materialNode;
}


template<typename T>
MuonBlueprintNodeBuilder::BluePrintSurfPairs_t  
  MuonBlueprintNodeBuilder::getSensitiveElements(const ActsTrk::GeometryContext& gctx,
                                                 const T& element, 
                                                 const Acts::GeometryIdentifier& chamberId,
                                                 Acts::VolumeBoundFactory& boundsFactory) const 
      requires(std::is_same_v<T, MuonGMR4::Chamber> || std::is_same_v<T, MuonGMR4::SpectrometerSector>){
  
  std::vector<BlueprintNodePtr_t> readoutVolumes;
  std::vector<SurfacePtr_t> readoutSurfaces;
  Acts::GeometryIdentifier::Value mdtId{1};
  std::unique_ptr<ActsTrk::VolumePlacement> placement{};
  std::array<std::vector<const MuonGMR4::MdtReadoutElement*>, 2> reEIL{};

  Acts::MultiWireVolumeBuilder::Config mwCfg;
  mwCfg.binning = {{Acts::AxisDirection::AxisY, 2u},
                   {Acts::AxisDirection::AxisZ, 1u}};
  mwCfg.shiftDirection = Acts::AxisDirection::AxisY;

  for (const MuonGMR4::MuonReadoutElement* readoutEle : element.readoutEles()) {

    std::vector<SurfacePtr_t> detSurfaces = readoutEle->getSurfaces();
    switch(readoutEle->detectorType()){
      case DetectorType::Mdt: {    
          const auto* mdtReadoutEle = static_cast<const MuonGMR4::MdtReadoutElement*>(readoutEle);
          // create the MDT multilayer volume with the dedicated builder
          
          mwCfg.name = m_detMgr->idHelperSvc()->toStringDetEl(mdtReadoutEle->identify());
          mwCfg.mlSurfaces = std::move(detSurfaces);
          mwCfg.transform = readoutEle->localToGlobalTransform(gctx);

          //initialize a nullptr material node which will be filled in the case of single MDT readout elements 
          //and used to assign the material to the volume and add the static node as child of the material node
          std::shared_ptr<Acts::MaterialDesignatorBlueprintNode> mdtMaterialNode;
          //special treatment of BIS78 MDT multilayer
          //use different shape because of clashes with EIL chambers 
          if (mdtReadoutEle->chamberIndex() == ChIndex::EIL) {
            reEIL[mdtReadoutEle->multilayer() -1].push_back(mdtReadoutEle);
            break;
          } else if(isBIS78(mdtReadoutEle) && mdtReadoutEle->multilayer() == 2){
            const MuonGMR4::MdtReadoutElement::parameterBook& parameters{mdtReadoutEle->getParameters()};
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
            mwCfg.bounds = MuonGMR4::BoundsExpander::makeBounds(*readoutEle, boundsFactory);
            if (m_alignableVolumes) {
               placement = std::make_unique<ActsTrk::VolumePlacement>(*readoutEle);
            }
          }
          
          
          if (placement) {     
            mwCfg.alignablePlacement = placement.get();
            element.addPlacement(std::move(placement));
          }

          Acts::MultiWireVolumeBuilder mdtBuilder{mwCfg};
          std::unique_ptr<Acts::TrackingVolume> mdtVolume = mdtBuilder.buildVolume();

          mdtVolume->assignGeometryId(chamberId.withExtra(mdtId++));
          //create the blueprint node for the mdt multilayers
          // check if this is a single mdt (single multilayer) chamber so we assign the material directly to the multilayer
          if(element.readoutEles().size() == 1 && m_assignActiveMaterial) {
            mdtMaterialNode = std::make_shared<Acts::MaterialDesignatorBlueprintNode>(element.identString() + "_MaterialNode");
            configureMaterialFaces(*mdtMaterialNode, mdtVolume->volumeBounds(), getActiveMaterial(element));
            auto staticNode = std::make_shared<Acts::StaticBlueprintNode>(std::move(mdtVolume));
            mdtMaterialNode->addChild(std::move(staticNode));
            readoutVolumes.push_back(std::move(mdtMaterialNode));
            break;
          }
          auto mdtNode = std::make_shared<Acts::StaticBlueprintNode>(std::move(mdtVolume));
          mdtNode->setNavigationPolicyFactory(mdtBuilder.createNavigationPolicyFactory(gctx.context()));
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
  /** Special treatment of the EIL 4/5 readout elements */
  for (const auto& reEles : reEIL) {
      if (reEles.empty()) {
          continue;
      }
      MuonGMR4::TrapezoidBoundsExpander expander{reEles.front()->globalToLocalTransform(gctx),
                                                 makeActsAthenaLogger(this, name())};
      std::vector<SurfacePtr_t> surfaces{};
      for (const MuonGMR4::MdtReadoutElement* reEle : reEles) {
          expander.expand(gctx.context(), *reEle, boundsFactory);
          std::ranges::copy(reEle->getSurfaces(), std::back_inserter(surfaces));
      }
      Acts::MultiWireVolumeBuilder::Config mwCfg;
      mwCfg.name = std::format("Mdt EIL45{:}{:}ML{:}", element.side() > 0 ? 'A' : 'C',
                                element.sector(),mdtId);
      mwCfg.transform = expander.toVertexCenter().inverse();
      if (m_alignableVolumes) {         
        placement = std::make_unique<ActsTrk::VolumePlacement>(*reEles.front(), 
                                                               expander.boxMidPoint());
        mwCfg.alignablePlacement = placement.get();
        element.addPlacement(std::move(placement));
      }
      mwCfg.bounds = expander.makeBounds(boundsFactory);
      mwCfg.mlSurfaces = std::move(surfaces);
      Acts::MultiWireVolumeBuilder mdtBuilder{mwCfg};
      std::unique_ptr<Acts::TrackingVolume> mdtVolume = mdtBuilder.buildVolume();
      mdtVolume->assignGeometryId(chamberId.withExtra(mdtId++));
      auto staticNode = std::make_shared<Acts::StaticBlueprintNode>(std::move(mdtVolume));
      readoutVolumes.push_back(std::move(staticNode));
  }

  return std::make_pair(std::move(readoutVolumes), std::move(readoutSurfaces));
}


bool MuonBlueprintNodeBuilder::isBIS78(const MuonGMR4::MuonReadoutElement* element) const {   
  int stEta = element->stationEta();
  if(m_isRun4){
    stEta = std::abs(element->stationEta());
  }
  return element->chamberIndex() == ChIndex::BIS && (stEta == 7 || std::abs(stEta) == 8);
  }

template<typename ElementSet_t>
std::vector<std::shared_ptr<Acts::Surface>> 
MuonBlueprintNodeBuilder::getPassiveMaterialSurfaces(const Acts::GeometryContext& tgContext,
                          std::vector<std::pair<unsigned, ElementSet_t>>&& elementsPerStation) const {

  if (!m_buildPassiveVolumes) {
      return {};
  }
  ATH_MSG_DEBUG(__func__<<"() "<<"Enter function");
  //this is a margin to put the surfaces along Z 
  //(a margin distance from the corresponding chamber's boundary surface)
  constexpr double margin{4._mm};

  std::vector<std::shared_ptr<Acts::Surface>> surfaces;
  surfaces.reserve(elementsPerStation.size());
  Acts::VolumeBoundFactory factory{};
  
  //lamda function to reject BIS78 chambers from the extension of the passive surface
  //otherwise they create overlap with the NSW sectors - stop a little bit before the cylinder of the passive surface
  const auto rejectBIS78 = [&](const MuonGMR4::MuonReadoutElement* readoutEle) {
    if(!isBIS78(readoutEle)) { 
      return false;
    }
    if (readoutEle->detectorType() == DetectorType::Mdt) {
        const auto* techEle = static_cast<const MuonGMR4::MdtReadoutElement*>(readoutEle);
        return techEle->multilayer() == 2 || std::abs(techEle->stationEta()) == 8;
    } else if (readoutEle->detectorType() == DetectorType::Rpc) {
        const auto* techEle = static_cast<const MuonGMR4::RpcReadoutElement*>(readoutEle);
        return techEle->doubletZ() == 2;
    }
    return false;
  };
  for(const auto& [hash, elements] : elementsPerStation){
    auto [regIdx, layIdx] = decomposeSectorLayerHash(hash);
    ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Construct passive surface for "<<regIdx<<", "<<layIdx);
    std::optional<double> discZmin{}, discZmax{};
    MuonGMR4::CylinderBoundsExpander expandBounds{makeActsAthenaLogger(this, name())};
    //loop through the elements of every station to construct the cylinder/disc  surfaces
    for(const auto& el : elements) {
      const Amg::Vector3D center = el->localToGlobalTransform(tgContext).translation();
      if(el->chamberIndex() == ChIdx::BIS &&
         std::ranges::any_of(el->readoutEles() ,[&](const MuonGMR4::MuonReadoutElement* re) { 
          return rejectBIS78(re);
        })){
          discZmax = std::max(discZmax.value_or(-40._m), center.z() - el->halfY());
          discZmin = std::min(discZmin.value_or(40._m), center.z() + el->halfY());
          ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Update "<<(*discZmin)<<", "<<(*discZmax)<< ", "<<el->identString());
          continue;
      }
      expandBounds.expand(tgContext, *el);
      if (regIdx != DetIdx::Barrel && (layIdx == LayIdx::Inner || 
                                       layIdx == LayIdx::Middle)) {
        const double lowerZ = center.z() - (std::copysign(el->halfZ(), center.z()));
        const double upperZ = center.z() + (std::copysign(el->halfZ(), center.z()));
        if (lowerZ > 0) {
            discZmin = std::min(discZmin.value_or(40._m), lowerZ - margin);  
            discZmax = std::max(discZmax.value_or(-40._m), upperZ  + margin);

        } else {
            discZmax = std::min(discZmax.value_or(40._m), lowerZ - margin);  
            discZmin = std::max(discZmin.value_or(-40._m), upperZ + margin);
        }
      }
    }
    // the chambers are groupd per chamber index and detector region(side) - 
    // we can use the first one for the distinction
    const bool aSide = (elements.front()->side() > 0);
    using namespace ActsTrk::detail::GeoVolIds;
    expandBounds.makeBounds(factory).reset();
    if (regIdx != DetIdx::Barrel) {
      ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - "<<layIdx<<" disc surface at "<<(*discZmin)<<" & "<<(*discZmax));
      auto bounds = std::make_shared<Acts::RadialBounds>(expandBounds.rMin(), expandBounds.rMax());
      auto frontSurface = Acts::Surface::makeShared<Acts::DiscSurface>(Amg::getTranslateZ3D(*discZmin), bounds);
      auto backSurface  = Acts::Surface::makeShared<Acts::DiscSurface>(Amg::getTranslateZ3D(*discZmax), bounds);
      /// Bins EIS / EMS -> front surface,  EIL / EML ->  back surface
            if (m_assignEmptyMaterial) {
        const auto[fBins1, fBins2] = getMaterialBins(layIdx == LayIdx::Inner ? ChIdx::EIS : ChIdx::EMS);
        frontSurface->assignSurfaceMaterial(preparePassiveMaterial(*bounds, fBins1, fBins2, 
                                         std::format("Material{:}FrontDisc{:}", layIdx, aSide ? 'A' : 'C' )));
        const auto[bBins1, bBins2] = getMaterialBins(layIdx == LayIdx::Inner ? ChIdx::EIL : ChIdx::EML);
        backSurface->assignSurfaceMaterial(preparePassiveMaterial(*bounds, bBins1, bBins2, 
                                         std::format("Material{:}BackDisc{:}", layIdx, aSide ? 'A' : 'C' )));
      }
      if (layIdx == LayIdx::Inner) {
        auto geoId = Acts::GeometryIdentifier{}.withVolume(s_muonBarrelId);
        frontSurface->assignGeometryId(geoId.withSensitive(aSide ? 5 : 1));
        backSurface->assignGeometryId(geoId.withSensitive(aSide ? 2: 3));
      } else if (layIdx == LayIdx::Middle) {
        auto geoId = Acts::GeometryIdentifier{}.withVolume(aSide ? 
                          s_muonEndcapMiddleAId : s_muonEndcapMiddleCId);
        frontSurface->assignGeometryId(geoId.withSensitive(1));
        backSurface->assignGeometryId(geoId.withSensitive(2)); 
      }
      surfaces.push_back(frontSurface);
      surfaces.push_back(backSurface);
    } else {
        //hack for run3 because of overlaps with eta = -7 BIS chambers
        double halfZ = 0.5*(discZmax.value_or(expandBounds.zMax()) - 
                            discZmin.value_or(expandBounds.zMin()));
        ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Create cylinder with "<<halfZ<<", "<<expandBounds.rMin());
        ChIdx elChIdx{ChIdx::ChUnknown};
        int layer{0};
        if (layIdx == LayIdx::Inner) {
          layer = 7;
          elChIdx = ChIdx::BIS;
        } else if (layIdx == LayIdx::Middle) {
          layer = 6;
          elChIdx = ChIdx::BML;
        } else if (layIdx == LayIdx::Outer) {
          layer = 4;
          elChIdx = ChIdx::BOL;
        }

        const auto [nBins1, nBins2] = getMaterialBins(elChIdx);
        
        auto surface = Acts::Surface::makeShared<Acts::CylinderSurface>(expandBounds.toVertexCenter(), 
                             std::make_shared<Acts::CylinderBounds>(expandBounds.rMin() - margin, halfZ - margin));
        if (m_assignEmptyMaterial) {
           surface->assignSurfaceMaterial(preparePassiveMaterial(surface->bounds(), nBins1, nBins2, 
                                         std::format("{:}MaterialCylinder", layIdx)));
        }        
        surface->assignGeometryId(Acts::GeometryIdentifier{}.withVolume(s_muonBarrelId).withSensitive(layer));      
        surfaces.push_back(surface);
    }
  }

  if(msgLvl(MSG::VERBOSE)){
    std::stringstream stream{};
    for(const auto& surf : surfaces){
      stream<< " at position  : "<< Amg::toString(surf->center(tgContext))
             << "with bounds "<< surf->bounds()<<std::endl;
    }
    ATH_MSG_VERBOSE("Constructed "<< surfaces.size()
        << " surfaces for passive material description : "<<std::endl<<stream.str());
  }
  std::ranges::sort(surfaces, [](const auto& a, const auto& b) {
    return a->geometryId() < b->geometryId();
  });
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
                                                     const std::size_t nBins2,
                                                     const std::string_view surfaceName) const {
    if (nBins1 == 0 || nBins2 == 0) {
      ATH_MSG_ERROR("Cannot create material for "<<bounds
        <<" as one of the bin dimensions is zero. nBins1: "<<nBins1<<", nBins2: "<<nBins2);
      return nullptr;
    }
    if (nBins1 == 1 && nBins1 == nBins2) {
      if (surfaceName.size()) {
          ATH_MSG_WARNING(__func__<<"() "<<__LINE__<<" - Surface name defined for homogenous material: "
                        <<surfaceName);
      }
      return std::make_shared<Acts::HomogeneousSurfaceMaterial>();
    }

    // the ranges and boundary types are left to the surface the material ends
    // up on, they are resolved from its bounds during material mapping
    auto protoMaterial = [&](Acts::AxisDirection dir1, std::size_t bins1,
                            Acts::AxisDirection dir2, std::size_t bins2) {
      return std::make_shared<Acts::ProtoGridSurfaceMaterial>(
          Acts::MultiAxisSpec2D({Acts::AxisSpec::DeferredEquidistant(bins1, dir1),
                                 Acts::AxisSpec::DeferredEquidistant(bins2, dir2)}),
                                 Acts::MappingType::Default,
                                 std::make_optional<std::string>(surfaceName));
    };

    switch (bounds.type()) {
       using enum Acts::SurfaceBounds::BoundsType;
      case eCylinder: {
          return protoMaterial(Acts::AxisDirection::AxisZ, nBins1,
                               Acts::AxisDirection::AxisRPhi, nBins2);
      } case eDisc: {
          return protoMaterial(Acts::AxisDirection::AxisR, nBins1,
                               Acts::AxisDirection::AxisPhi, nBins2);
      } default:
        ATH_MSG_ERROR("Unsupoorted type "<<bounds<<".");
        return nullptr;
    }
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
        THROW_EXCEPTION("getMaterialBins() - "<<chIdx<<" is not yet implemented");
    }
    return std::make_pair(0ul, 0ul);
}
} //namespace ActsTrk
