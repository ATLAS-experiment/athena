/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloBlueprintNodeBuilder.h"

#include "GeoPrimitives/GeoPrimitivesHelpers.h"

#include "Acts/Geometry/Blueprint.hpp"
#include "Acts/Geometry/PortalShell.hpp"
#include "Acts/Geometry/Volume.hpp"
#include "Acts/Geometry/StaticBlueprintNode.hpp"
#include "Acts/Geometry/GeometryIdentifierBlueprintNode.hpp"
#include <Acts/Navigation/SurfaceArrayNavigationPolicy.hpp>
#include <Acts/Navigation/TryAllNavigationPolicy.hpp>
#include <Acts/Utilities/AxisDefinitions.hpp>
#include <Acts/Geometry/ContainerBlueprintNode.hpp>
#include <Acts/Geometry/Extent.hpp>
#include <Acts/Definitions/Units.hpp>
#include <Acts/Geometry/LayerBlueprintNode.hpp>
#include <Acts/Geometry/VolumeResizeStrategy.hpp>
#include <Acts/Geometry/VolumeAttachmentStrategy.hpp>
#include <Acts/Geometry/TrackingVolume.hpp>
#include <Acts/Geometry/CylinderVolumeBounds.hpp>

#include "CaloDetDescrUtils/CaloDetDescrBuilder.h"

#include <Acts/Surfaces/SurfaceArray.hpp>
#include "CaloIdentifier/CaloCell_ID.h"

using namespace Acts;
using namespace Acts::UnitLiterals;
using AttachmentStrategy = Acts::VolumeAttachmentStrategy;
using ResizeStrategy = Acts::VolumeResizeStrategy;

using namespace ActsTrk::detail::GeoVolIds;

StatusCode ActsTrk::CaloBlueprintNodeBuilder::initialize() {
  ATH_MSG_DEBUG("Initializing CaloBlueprintNodeBuilder");

  m_caloDetSecrMgr = buildCaloDetDescrNoAlign(serviceLocator(), Athena::getMessageSvc());
  
  return StatusCode::SUCCESS;
}

std::shared_ptr<BlueprintNode> ActsTrk::CaloBlueprintNodeBuilder::buildBlueprintNode(const GeometryContext& /*gctx*/,
                                      std::shared_ptr<BlueprintNode>&& childNode) {


  std::map<caloRegion, caloSampleSurfaceMap_t> caloRegionSampleSurfaceMap;
  std::map<caloRegion, caloSampleDDEElementsMap_t> caloRegionSampleDDEElementsMap;
  std::map<std::string, double> caloDimensions;

  fillMaps(caloRegionSampleSurfaceMap, caloRegionSampleDDEElementsMap, caloDimensions);

  ATH_MSG_DEBUG("Have filled first two maps");

  generateCylinderSurfaces(caloRegionSampleSurfaceMap[caloRegion::CylinderSymmetricZZero], caloRegionSampleDDEElementsMap[caloRegion::CylinderSymmetricZZero],false);
  generateCylinderSurfaces(caloRegionSampleSurfaceMap[caloRegion::CylinderNegativeZ], caloRegionSampleDDEElementsMap[caloRegion::CylinderNegativeZ],true);
  generateCylinderSurfaces(caloRegionSampleSurfaceMap[caloRegion::CylinderPositiveZ], caloRegionSampleDDEElementsMap[caloRegion::CylinderPositiveZ],true);
  generateDiscSurfaces(caloRegionSampleSurfaceMap[caloRegion::DiscNegativeZ], caloRegionSampleDDEElementsMap[caloRegion::DiscNegativeZ]);
  generateDiscSurfaces(caloRegionSampleSurfaceMap[caloRegion::DiscPositiveZ], caloRegionSampleDDEElementsMap[caloRegion::DiscPositiveZ]);

  ATH_MSG_DEBUG("Have generated calorimeter cylindrical surfaces");

  // The calo node is a container node that will hold the itk and calo nodes as children. 
  // The calo cylinder is static in order to avoid merging issues with the itk portals 
  // that are supposed to carry material.
  // The envelope of the calo node is set to the maximum radius and half length in z of the calorimeter, 
  // which is via a loop over all CaloDetDescrElements in the CaloDetDescrManager in the fillMaps function.

  double caloEvelopeMaxR = caloDimensions.at("maxR");
  double caloEnvelopeHalfLenghtZ = (caloDimensions.at("maxPosZ") - caloDimensions.at("minNegZ"))/2.0;

  ATH_MSG_INFO("Calo envelope dimensions: maxR = " << caloEvelopeMaxR << ", halfLengthZ = " << caloEnvelopeHalfLenghtZ);
  std::shared_ptr<StaticBlueprintNode> itkCaloNode{};
  {
    auto envelope = std::make_unique<TrackingVolume>(Amg::Transform3D::Identity(),
                                                     std::make_shared<CylinderVolumeBounds>(0., caloEvelopeMaxR, caloEnvelopeHalfLenghtZ),"ITkCalo");
    envelope->assignGeometryId(Acts::GeometryIdentifier{}.withVolume(s_caloEnvelopeID));
    itkCaloNode = std::make_shared<StaticBlueprintNode>(std::move(envelope));
  }
  if (childNode) {
    itkCaloNode->addChild(std::move(childNode));
  }

  ATH_MSG_DEBUG("Top level calorimeter node created");
            
  auto caloNode = std::make_shared<CylinderContainerBlueprintNode>("CaloNode", AxisDirection::AxisZ);
  CylinderContainerBlueprintNode& caloBarrelCylinderNode = caloNode->addCylinderContainer("CaloBarrelSymmetricZZeroCylinders", AxisDirection::AxisR);
  caloBarrelCylinderNode.setAttachmentStrategy(VolumeAttachmentStrategy::Gap);
  caloBarrelCylinderNode.setResizeStrategy(ResizeStrategy::Gap);
  
  ATH_MSG_DEBUG("EM Barrel container node created");

  unsigned int volumeCounter = 0;

  //Barrel cylinders symmetric about z = 0
  for (unsigned int sampleIndex = 0; sampleIndex < m_caloCylinderSymmetricSampleList.size(); ++sampleIndex) {
    auto& sampleName = m_caloCylinderSymmetricSampleList.at(sampleIndex).first;
    addCylindricalTrackingVolumeToCaloNode(caloBarrelCylinderNode, sampleName, caloRegionSampleSurfaceMap[caloRegion::CylinderSymmetricZZero].at({sampleName, getSampleEnum(sampleName)}), volumeCounter, false);
    volumeCounter++;
  }

  //now do asymmetric cylinders, first negative z
  
  CylinderContainerBlueprintNode& caloBarrelCylinderNegativeZNode = caloNode->addCylinderContainer("CaloBarrelNegativeZAsymmetricCylinders", AxisDirection::AxisR);
  caloBarrelCylinderNegativeZNode.setAttachmentStrategy(VolumeAttachmentStrategy::Gap);
  caloBarrelCylinderNegativeZNode.setResizeStrategy(ResizeStrategy::Gap);

  //then positive z
  CylinderContainerBlueprintNode& caloBarrelCylinderPositiveZNode = caloNode->addCylinderContainer("CaloBarrelPositiveZAsymmetricCylinders", AxisDirection::AxisR);
  caloBarrelCylinderPositiveZNode.setAttachmentStrategy(VolumeAttachmentStrategy::Gap);
  caloBarrelCylinderPositiveZNode.setResizeStrategy(ResizeStrategy::Gap);

  for (unsigned int sampleIndex = 0; sampleIndex < m_caloCylinderAsymmetricSampleList.size(); ++sampleIndex) {
    auto& sampleName = m_caloCylinderAsymmetricSampleList.at(sampleIndex).first;
    //We only add TileGap1 and 2 here, because the TileExt0,1,2
    //will need a special treatment to avoid clashes in Z.
    if (sampleIndex < 2){
      addCylindricalTrackingVolumeToCaloNode(caloBarrelCylinderNegativeZNode, sampleName+"NegZ", caloRegionSampleSurfaceMap[caloRegion::CylinderNegativeZ].at({sampleName, getSampleEnum(sampleName)}), volumeCounter, false);
      volumeCounter++;
      addCylindricalTrackingVolumeToCaloNode(caloBarrelCylinderPositiveZNode, sampleName+"PosZ", caloRegionSampleSurfaceMap[caloRegion::CylinderPositiveZ].at({sampleName, getSampleEnum(sampleName)}), volumeCounter, false);
      volumeCounter++;
    }
    else{
      //Tile extended barrel surfaces must be added to top level node directly because they always overlap in R
      //or Z with other calorimeter surfaces, volumes etc.
      //Note there is a speed penalty to do it this way.
      itkCaloNode->addLayer(sampleName+"NegZ" + "_Layer", [&](auto& layer) {
        layer.setSurfaces(caloRegionSampleSurfaceMap[caloRegion::CylinderNegativeZ].at({sampleName, getSampleEnum(sampleName)}));
        layer.setEnvelope(Acts::ExtentEnvelope{{
            .z = {0.1_mm, 0.1_mm},
            .r = {2_mm, 2_mm},
        }});
      });
      itkCaloNode->addLayer(sampleName+"PosZ" + "_Layer", [&](auto& layer) {
        layer.setSurfaces(caloRegionSampleSurfaceMap[caloRegion::CylinderPositiveZ].at({sampleName, getSampleEnum(sampleName)}));
        layer.setEnvelope(Acts::ExtentEnvelope{{
            .z = {0.1_mm, 0.1_mm},
            .r = {2_mm, 2_mm},
        }});
      });
    }
  }
  

  CylinderContainerBlueprintNode& caloEndCapDiscNegativeZNode = caloNode->addCylinderContainer("CaloEndCapDiscNegativeZ", AxisDirection::AxisZ);
  caloEndCapDiscNegativeZNode.setAttachmentStrategy(VolumeAttachmentStrategy::Gap);
  // The -z end of this container defines the calorimeter's global minZ, so the
  // enclosing envelope's -z edge coincides with this container's own -z edge to
  // within floating-point rounding. With ResizeStrategy::Gap on that side, the
  // resize mints a degenerate (~1e-13 mm) end-gap volume whose two disc faces
  // coincide, which CylinderNavigationPolicy cannot resolve. Expand the
  // outermost wheel on the boundary (-z) side instead; keep Gap on the interior
  // (+z) side. NOTE: (inner, outer) map to (minZ, maxZ) for an AxisZ stack.
  caloEndCapDiscNegativeZNode.setResizeStrategies(ResizeStrategy::Expand, ResizeStrategy::Gap);

  CylinderContainerBlueprintNode& caloEndCapDiscPositiveZNode = caloNode->addCylinderContainer("CaloEndCapDiscPositiveZ", AxisDirection::AxisZ);
  caloEndCapDiscPositiveZNode.setAttachmentStrategy(VolumeAttachmentStrategy::Gap);
  // Mirror of the negative endcap: the +z (maxZ, outer) end is the global maxZ
  // boundary, so Expand there and keep Gap on the interior (-z) side.
  caloEndCapDiscPositiveZNode.setResizeStrategies(ResizeStrategy::Gap, ResizeStrategy::Expand);
  
  for (unsigned int sampleIndex = 0; sampleIndex < m_caloDiscSampleList.size(); ++sampleIndex) {
    auto& sampleName = m_caloDiscSampleList.at(sampleIndex).first;
    addCylindricalTrackingVolumeToCaloNode(caloEndCapDiscNegativeZNode, sampleName+"NegZ", caloRegionSampleSurfaceMap[caloRegion::DiscNegativeZ].at({sampleName, getSampleEnum(sampleName)}), volumeCounter, true);
    volumeCounter++;
    addCylindricalTrackingVolumeToCaloNode(caloEndCapDiscPositiveZNode, sampleName+"PosZ", caloRegionSampleSurfaceMap[caloRegion::DiscPositiveZ].at({sampleName, getSampleEnum(sampleName)}), volumeCounter, true);
    volumeCounter++;
  }

  ATH_MSG_DEBUG("Have added all Barrel layers to caloBarrelCylinderNode");

  // Add calo barrel node to the top level calo node.
  itkCaloNode->addChild(caloNode);

  //return the top level calo node
  return itkCaloNode;
}

StatusCode ActsTrk::CaloBlueprintNodeBuilder::finalize() {
  ATH_MSG_DEBUG("Finalizing CaloBlueprintNodeBuilder");
  return StatusCode::SUCCESS;
}

void ActsTrk::CaloBlueprintNodeBuilder::fillMaps(std::map<caloRegion, caloSampleSurfaceMap_t>& caloRegionSampleSurfaceMap,
                      std::map<caloRegion, caloSampleDDEElementsMap_t>& caloRegionSampleDDEElementsMap, std::map<std::string, double>& caloDimensions) const {


  //loop over all possible calo sampling layers
  //and create empty vectors of surfaces in the map

  //Create map between each calorimeter sampling and a vector of
  //cylinder surfaces. We can have N cylinders in a given sampling,
  //and the value of N is determined by how often the average radius 
  //calculated for a given phi ring, at fixed Z, changes by more than 
  //a tolerance value

  //Use the same loop to create map bwteeen sampling and vectors of DDE
  for (const auto & currentSample : m_caloCylinderSymmetricSampleList) {
    caloRegionSampleSurfaceMap[caloRegion::CylinderSymmetricZZero][currentSample] = std::vector<std::shared_ptr<Surface> >();
    caloRegionSampleDDEElementsMap[caloRegion::CylinderSymmetricZZero][currentSample] = std::vector<const CaloDetDescrElement*>();
  }

  for (const auto & currentSample : m_caloCylinderAsymmetricSampleList) {
    caloRegionSampleSurfaceMap[caloRegion::CylinderNegativeZ][currentSample] = std::vector<std::shared_ptr<Surface> >();
    caloRegionSampleDDEElementsMap[caloRegion::CylinderNegativeZ][currentSample] = std::vector<const CaloDetDescrElement*>();
    caloRegionSampleSurfaceMap[caloRegion::CylinderPositiveZ][currentSample] = std::vector<std::shared_ptr<Surface> >();
    caloRegionSampleDDEElementsMap[caloRegion::CylinderPositiveZ][currentSample] = std::vector<const CaloDetDescrElement*>();
  }

  for (const auto & currentSample : m_caloDiscSampleList) {
    caloRegionSampleSurfaceMap[caloRegion::DiscNegativeZ][currentSample] = std::vector<std::shared_ptr<Surface> >();
    caloRegionSampleDDEElementsMap[caloRegion::DiscNegativeZ][currentSample] = std::vector<const CaloDetDescrElement*>();
    caloRegionSampleSurfaceMap[caloRegion::DiscPositiveZ][currentSample] = std::vector<std::shared_ptr<Surface> >();
    caloRegionSampleDDEElementsMap[caloRegion::DiscPositiveZ][currentSample] = std::vector<const CaloDetDescrElement*>();
  }

  double maxR = 0.0;
  double maxPosZ = 0.0;
  double minNegZ = 0.0;

  //for each calo sampling collect all the DDE in a vector    
  for (const CaloDetDescrElement* theDDE : m_caloDetSecrMgr->element_range()){
    if (!theDDE){ 
      ATH_MSG_ERROR("Null pointer to CaloDetDescrElement");
      continue;
    }

    if (theDDE->r() > maxR) maxR = theDDE->r();
    if (theDDE->z() > maxPosZ) maxPosZ = theDDE->z();
    if (theDDE->z() < minNegZ) minNegZ = theDDE->z();

    CaloCell_ID::CaloSample currentSample=theDDE->getSampling();
    std::pair <std::string, CaloCell_ID::CaloSample> samplePair = std::make_pair(getSampleName(currentSample), currentSample);
    
    //check if have cylinder symmetric about z = 0
    if (std::find(m_caloCylinderSymmetricSampleList.begin(), m_caloCylinderSymmetricSampleList.end(), samplePair) != m_caloCylinderSymmetricSampleList.end()){
      caloRegionSampleDDEElementsMap[caloRegion::CylinderSymmetricZZero][{samplePair.first,samplePair.second}].push_back(theDDE);
    }
    
    //check if have cylinder asymmetric about z = 0, if so check if in negative or positive z
    if (std::find(m_caloCylinderAsymmetricSampleList.begin(), m_caloCylinderAsymmetricSampleList.end(), samplePair) != m_caloCylinderAsymmetricSampleList.end()){
      if (theDDE->z() < 0.0) {
        caloRegionSampleDDEElementsMap[caloRegion::CylinderNegativeZ][{samplePair.first,samplePair.second}].push_back(theDDE);
      }
      else {
        caloRegionSampleDDEElementsMap[caloRegion::CylinderPositiveZ][{samplePair.first,samplePair.second}].push_back(theDDE);
      }
    }

    //check if sampling is in disc list
    if (std::find(m_caloDiscSampleList.begin(), m_caloDiscSampleList.end(), std::make_pair(samplePair.first,samplePair.second)) != m_caloDiscSampleList.end()) {
      //check if in negative or positive z
      if (theDDE->z() < 0.0) {
        caloRegionSampleDDEElementsMap[caloRegion::DiscNegativeZ][{getSampleName(currentSample), currentSample}].push_back(theDDE);
      }
      else {
        caloRegionSampleDDEElementsMap[caloRegion::DiscPositiveZ][{getSampleName(currentSample), currentSample}].push_back(theDDE);
      }
    }

  }

  caloDimensions["maxR"] = maxR;
  caloDimensions["maxPosZ"] = maxPosZ;
  caloDimensions["minNegZ"] = minNegZ;

  auto sortAllLayersInZ = [&caloRegionSampleDDEElementsMap](const std::vector<std::pair<std::string, CaloCell_ID::CaloSample>>& caloSampleList, const caloRegion& region) {
    for (const auto & currentSample : caloSampleList) {
      std::vector<const CaloDetDescrElement*> currentElements = caloRegionSampleDDEElementsMap[region][currentSample];
      std::sort(currentElements.begin(), currentElements.end(), [](const CaloDetDescrElement* a, const CaloDetDescrElement* b) {return a->z() < b->z();});
      caloRegionSampleDDEElementsMap[region][currentSample] = std::move(currentElements);
    }
  };

  //Sort the DDE, by Z, in all possible layers
  sortAllLayersInZ(m_caloCylinderSymmetricSampleList, caloRegion::CylinderSymmetricZZero);
  sortAllLayersInZ(m_caloCylinderAsymmetricSampleList, caloRegion::CylinderNegativeZ);
  sortAllLayersInZ(m_caloCylinderAsymmetricSampleList, caloRegion::CylinderPositiveZ);

  auto sortAllLayersInR = [&caloRegionSampleDDEElementsMap](const std::vector<std::pair<std::string, CaloCell_ID::CaloSample>>& caloSampleList, const caloRegion& region) {
    for (const auto& currentSample : caloSampleList) {
      std::vector<const CaloDetDescrElement*> currentElements = caloRegionSampleDDEElementsMap[region][currentSample];
      std::sort(currentElements.begin(), currentElements.end(), [](const CaloDetDescrElement* a, const CaloDetDescrElement* b) {return a->r() < b->r();});
      caloRegionSampleDDEElementsMap[region][currentSample] = std::move(currentElements);
    }
  };

  //Sort the DDE, by R, in all possible layers
  sortAllLayersInR(m_caloDiscSampleList, caloRegion::DiscNegativeZ);
  sortAllLayersInR(m_caloDiscSampleList, caloRegion::DiscPositiveZ);

}

void ActsTrk::CaloBlueprintNodeBuilder::generateCylinderSurfaces(caloSampleSurfaceMap_t& caloSampleSurfaceMap, caloSampleDDEElementsMap_t& caloSampleDDEElementsMap, bool asymmetricZ) const{

  std::vector<std::pair<std::string, CaloCell_ID::CaloSample>> sampleList;
  if (asymmetricZ) sampleList = m_caloCylinderAsymmetricSampleList;
  else sampleList = m_caloCylinderSymmetricSampleList;

  for (const auto & currentSample : sampleList) {

    std::vector<const CaloDetDescrElement*> currentElements = caloSampleDDEElementsMap[currentSample];

    double maxLArBRadius = 0.0, minLArBRadius = std::numeric_limits<double>::max();
    double lowZLarB = 0.0, highZLarB = 0.0;

    //loop over cells runs from -z to +z in a given sampling layer
    //There are many cells with the same z value, but different phi values
    //We will find the average radius of the cells in a given phi ring
    double totalRadiusFixedPhi = 0.0;
    bool firstCellInPhiRing = true;
    unsigned int phiCounter = 0;

    //Then we will also track changes in radius as we move in Z from
    //each ring of cells in phi to the next ring of cells in phi
    bool firstPhiRing = true;
    double initialRadius = 0.0;
    double initialZ = -std::numeric_limits<double>::max();

    //check if we ever move along in Z value
    bool movedInZ = false;

    for (const CaloDetDescrElement* theDDE : currentElements){

      double z = theDDE->z();   
      double radius = theDDE->r();

      ATH_MSG_DEBUG(" Calo Sampling is " << currentSample.first);

      if (firstCellInPhiRing) {
        ATH_MSG_DEBUG("First Cell in phi ring " << currentSample.first << " has z = " << z << " and r = " << radius);
        initialZ = z;
        firstCellInPhiRing = false;
      }

      if (firstPhiRing) {
        ATH_MSG_DEBUG("First Cell in layer " << currentSample.first << " has z = " << z << " and r = " << radius);
        initialRadius = theDDE->r();
        firstPhiRing = false;
        lowZLarB = z;
      }

      ATH_MSG_DEBUG("Z and initialZ are " << z << " and " << initialZ);

      //if z has not changed then we add the radius to the summed radius for this phi ring
      //and increment the counter of cells in this phi ring
      if (std::abs(z - initialZ) < 0.0001) {
        ATH_MSG_DEBUG("phiCounter is " << phiCounter << " and radius is " << radius << " and totalRadiusFixedPhi is " << totalRadiusFixedPhi << " and hash is " << theDDE->calo_hash());
        totalRadiusFixedPhi += radius;
        phiCounter++;
        continue;
      }
      else {
        movedInZ = true;
        firstCellInPhiRing = true;
        if (phiCounter > 0) {
          double cellRingRadius = totalRadiusFixedPhi / phiCounter;
          totalRadiusFixedPhi = 0.0;
          phiCounter = 0;

          if (cellRingRadius > maxLArBRadius) maxLArBRadius = radius;
          if (cellRingRadius < minLArBRadius) minLArBRadius = radius;

          //if radius changes by more than tolerance, then we will create a cylinder
          //with the average cell radius and length from neg to pos z
          highZLarB = z;
          ATH_MSG_DEBUG("Values of cellRingRadius, initialRadius, highZLarB and lowZLarB are " << cellRingRadius << ", " << initialRadius << ", " << highZLarB << " and " << lowZLarB);
          if (std::abs(cellRingRadius - initialRadius) > m_radiusTolerance && highZLarB - lowZLarB > 0.0) {
            ATH_MSG_DEBUG("CYLINDER: Create cylinder for layer " << currentSample.first);                
            ATH_MSG_DEBUG("CYLINDER: Create Cylinder: Min and Max LAr B radius are " << minLArBRadius << " " << maxLArBRadius);
            ATH_MSG_DEBUG("CYLINDER: Create Cylinder: Min and Max LAr B z are " << lowZLarB << " " << highZLarB);

            caloSampleSurfaceMap[currentSample].push_back(generateCylinderSurface(maxLArBRadius, minLArBRadius, lowZLarB, highZLarB, asymmetricZ));

            //reset the dimensions of the cylinder to the initial conditions, in 
           //preparation for the next cylinder
           firstPhiRing = true;
           minLArBRadius = std::numeric_limits<double>::max();
           maxLArBRadius = 0.0;
           lowZLarB = 0.0;
           highZLarB = 0.0;
          }//if radius changes by more than tolerance
        }//if at least one cell in phi (should always be the case!)
        else ATH_MSG_ERROR("phiCounter is zero!");
      }//if z has changed
    }//loop over calorimeter DDE

    if (0 == caloSampleSurfaceMap[currentSample].size()){
      //If we never found any shift in Z whilst looping over the DDE
      //then thee min/max Z and radius were not set
      //so we set them here.
      if (!movedInZ) {
        lowZLarB = initialZ;
        highZLarB = initialZ+0.001;
        if (phiCounter > 0) {
          maxLArBRadius = totalRadiusFixedPhi / phiCounter;
          minLArBRadius = 0.0;
        }
      }
      ATH_MSG_DEBUG("CYLINDER: Zero size Vector: Create cylinder for layer " << currentSample.first);   
      ATH_MSG_DEBUG("CYLINDER: Create Cylinder: Min and Max LAr B radius are " << minLArBRadius << " " << maxLArBRadius);
      ATH_MSG_DEBUG("CYLINDER: Create Cylinder: Min and Max LAr B z are " << lowZLarB << " " << highZLarB);
      caloSampleSurfaceMap[currentSample].push_back(generateCylinderSurface(maxLArBRadius, minLArBRadius, lowZLarB, highZLarB, asymmetricZ));
    }
  }   
}

std::shared_ptr<CylinderSurface> ActsTrk::CaloBlueprintNodeBuilder::generateCylinderSurface(const double& maxLArBRadius, const double& minLArBRadius, const double& lowZLarB, const double& highZLarB, bool asymmetricZ) const{

  //Characterise the dimensions of the  cylinder
  double LArBRadius = (maxLArBRadius + minLArBRadius) / 2.0;
  double LArBLength = std::abs(highZLarB - lowZLarB);

  ATH_MSG_DEBUG("Cylinder radius and length are " << LArBRadius << " and " << LArBLength);

  if (asymmetricZ) {
    double zShift = (highZLarB + lowZLarB) / 2.0;
    ATH_MSG_DEBUG("Cylinder is asymmetric in Z, with shift of " << zShift);
     return Surface::makeShared<CylinderSurface>(Transform3(Translation3(0.0, 0.0, zShift)), LArBRadius, LArBLength/2);
  }
  else return Surface::makeShared<CylinderSurface>(Transform3::Identity(), LArBRadius, LArBLength/2);

}

void ActsTrk::CaloBlueprintNodeBuilder::generateDiscSurfaces(caloSampleSurfaceMap_t& caloSampleSurfaceMap, caloSampleDDEElementsMap_t& caloSampleDDEElementsMap) const{

  for (const auto & currentSample : m_caloDiscSampleList) {

        const std::vector<const CaloDetDescrElement*> & currentElements = caloSampleDDEElementsMap[currentSample];

        //FCAL is treated differently because it is non-projective
        //We create one surface for each of FCAL0, FCAL1 and FCAL2
        if (CaloCell_ID::FCAL0 == currentSample.second || CaloCell_ID::FCAL1 == currentSample.second || CaloCell_ID::FCAL2 == currentSample.second) {
          ATH_MSG_DEBUG("TOM3: FCAL sampling " << currentSample.second);    
          
          double fcalRMin = std::numeric_limits<double>::max();
          double fcalRMax = std::numeric_limits<double>::min();
          double fcalZSum = 0;
          size_t cellCount = 0;

          for (const CaloDetDescrElement*  theDDE : currentElements) {
            double r = theDDE->r();
            if (r < fcalRMin) fcalRMin = r;
            if (r > fcalRMax) fcalRMax = r;
            fcalZSum += theDDE->z();
            cellCount++;
          }
          if (cellCount == 0)[[unlikely]]{
            ATH_MSG_WARNING("cellCount is zero in CaloBlueprintNodeBuilder::generateDiscSurfaces");
            continue;
          }
          double fcalZ = fcalZSum / cellCount;
          caloSampleSurfaceMap[currentSample].push_back(generateDiscSurface(fcalZ,fcalRMax, fcalRMin));
          //now continue to the next sampling
          continue;
        }

        //For other disc calorimeter layers we proceed to create many surfaces as needed.
        double totalZFixedPhi = 0.0;
        bool firstCellInPhiRing = true;
        unsigned int phiCounter = 0;

        bool firstPhiRing = true;
        double initialRadius = 0.0;
        double initialZ = -std::numeric_limits<double>::max();

        //loop over cells runs from -z to +z in a given sampling layer
        //There are many cells with the same z value, but different phi values
        //We will find a min amd max radius in this phi ring.
        double minDiscRadius = std::numeric_limits<double>::max(), maxDiscRadius = 0.0;

        unsigned int currentElementsSize = currentElements.size();
        unsigned int DDECounter = 0;

        for (const CaloDetDescrElement* theDDE : currentElements){

            bool isLastDDE = (DDECounter == (currentElementsSize-1));

            ATH_MSG_DEBUG("Disc DDE with sampling, r and z of " << currentSample << ", " << theDDE->r() << ", " << theDDE->z());
            ATH_MSG_DEBUG("isLastDDE is " << isLastDDE);

            double z = theDDE->z();   
            double radius = theDDE->r();

            if (firstCellInPhiRing) {
                initialRadius = radius;
                firstCellInPhiRing = false;
            }

            if (firstPhiRing) {
                initialZ = z;
                firstPhiRing = false;
                minDiscRadius = radius;
            }

            //if radius is unchanged then we add the z to the summed z for this phi ring
            //and increment the counter of cells in this phi ring
            if (std::abs(radius - initialRadius) < 0.0001 && !isLastDDE) {
                totalZFixedPhi += z;
                phiCounter++;
                DDECounter++;
                continue;
            }
            else {
                firstCellInPhiRing = true;
                if (phiCounter > 0) {
                    double cellRingZ = totalZFixedPhi / phiCounter;
                    totalZFixedPhi = 0.0;
                    phiCounter = 0;

                    //if z changes by more than tolerance, then we will create a disc
                    //with the min and max radius found
                    //if we reach the last DDE and no new surface has been created, then we also create a disc surface
                    maxDiscRadius = radius;
                    ATH_MSG_DEBUG("cellRingZ, initialZ and z tolerance are " << cellRingZ << ", " << initialZ << " and " << m_zTolerance);
                    if (std::abs(cellRingZ - initialZ) > m_zTolerance || isLastDDE) {
                      ATH_MSG_DEBUG("DISC: About to create disc surface for sampling " << currentSample);
                        caloSampleSurfaceMap[currentSample].push_back(generateDiscSurface(cellRingZ,maxDiscRadius, minDiscRadius));
                        //reset the dimensions of the disc to the initial conditions, in 
                        //preparation for the next disc
                        minDiscRadius = std::numeric_limits<double>::max();
                        maxDiscRadius = 0.0;
                        firstPhiRing = true; 
                    }//if z changes by more than tolerance
                }//if at least one cell in phi (should always be the case!)
                else ATH_MSG_ERROR("phiCounter is zero!");
            }//if radius has changed
            DDECounter++;
        }//loop over calorimeter DDE
  }//loop over calo samplings        
}

std::shared_ptr<Acts::DiscSurface> ActsTrk::CaloBlueprintNodeBuilder::generateDiscSurface(const double& z, const double& maxLArBRadius, const double& minLArBRadius) const{

  ATH_MSG_DEBUG("DISC: Disc min and max radius are " << minLArBRadius << " and " << maxLArBRadius << " with z of " << z);
  auto surface = Surface::makeShared<DiscSurface>(Amg::getTranslateZ3D(z), minLArBRadius, maxLArBRadius);

  return surface;

}

void ActsTrk::CaloBlueprintNodeBuilder::addCylindricalTrackingVolumeToCaloNode(CylinderContainerBlueprintNode& containerNode, const std::string& volumeName,const std::vector<std::shared_ptr<Acts::Surface>>& surfaces, int layerIndex, const bool& isDisc) const{

  // Construct the container node with geometry identifier and layer, and add the surfaces to the layer.
  Acts::GeometryIdentifierBlueprintNode& geoIdNode = containerNode.withGeometryIdentifier();
  geoIdNode.setAllVolumeIdsTo(s_caloBarrelId +
  layerIndex);

  AxisDirection axis = AxisDirection::AxisZ;
  if (isDisc) axis = AxisDirection::AxisR;
  CylinderContainerBlueprintNode& cylinder = geoIdNode.addCylinderContainer(volumeName,
  axis);

  cylinder.addLayer(volumeName + "_Layer", [&](auto& layer) {
        layer.setSurfaces(surfaces);
        layer.setEnvelope(Acts::ExtentEnvelope{{
            .z = {0.1_mm, 0.1_mm},
            .r = {2_mm, 2_mm},
        }});
    });

}
