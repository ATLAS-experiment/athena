/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloBlueprintNodeBuilder.h"

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
using namespace Acts::Experimental;
using namespace Acts::UnitLiterals;
using AttachmentStrategy = Acts::VolumeAttachmentStrategy;
using ResizeStrategy = Acts::VolumeResizeStrategy;

namespace {
  // Calo IDs 
  constexpr std::size_t s_caloBarrelId = 40;
}

StatusCode ActsTrk::CaloBlueprintNodeBuilder::initialize() {
  ATH_MSG_DEBUG("Initializing CaloBlueprintNodeBuilder");

  m_caloDetSecrMgr = buildCaloDetDescrNoAlign(serviceLocator()
                                                                              , Athena::getMessageSvc());
  
  return StatusCode::SUCCESS;
}

std::shared_ptr<BlueprintNode> ActsTrk::CaloBlueprintNodeBuilder::buildBlueprintNode(const GeometryContext& /*gctx*/,
                                      std::shared_ptr<BlueprintNode>&& childNode) {


  std::map<caloRegion, caloSampleSurfaceMap_t> caloRegionSampleSurfaceMap;
  std::map<caloRegion, caloSampleDDEElementsMap_t> caloRegionSampleDDEElementsMap;

  fillMaps(caloRegionSampleSurfaceMap, caloRegionSampleDDEElementsMap);

  ATH_MSG_DEBUG("Have filled first two maps");

  // TODO: it seems that only the CaloMaxR and CaloHalfLengthZ 
  // m_radiusTolerancesions are used later on... do we need the others?
  caloDimensionMap_t caloDimensionMap;
  fillCaloDimensionsMap(caloDimensionMap, caloRegionSampleDDEElementsMap[caloRegion::Global]);

  ATH_MSG_DEBUG("Have filled calo dimensions map");

  generateCylinderSurfaces(caloRegionSampleSurfaceMap[caloRegion::Global], caloRegionSampleDDEElementsMap[caloRegion::Global]);
  generateDiscSurfaces(caloRegionSampleSurfaceMap[caloRegion::DiscNegativeZ], caloRegionSampleDDEElementsMap[caloRegion::DiscNegativeZ]);
  generateDiscSurfaces(caloRegionSampleSurfaceMap[caloRegion::DiscPositiveZ], caloRegionSampleDDEElementsMap[caloRegion::DiscPositiveZ]);

  ATH_MSG_DEBUG("Have generated calorimeter cylindrical surfaces");

  // The calo node is a container node that will hold the itk and calo nodes as children. 
  // The calo cylinder is static in order to avoid merging issues with the itk portals 
  // that are supposed to carry material.
  // Take the itk+calo cylinder dimensions based from the previously evaluated map adding some tolerance.
  // Use 2.0 as a reasonable first guess at the tolerance needed.
  auto itkCaloNode = std::make_shared<StaticBlueprintNode>( 
      std::make_unique<TrackingVolume>(Transform3::Identity(),
          std::make_shared<CylinderVolumeBounds>(0., 
                                                 caloDimensionMap["CaloMaxR"] + 2.0, 
                                                 caloDimensionMap["CaloHalfLengthZ"] + 2.0),"ITkCalo"));

  if (childNode) itkCaloNode->addChild(std::move(childNode));

  ATH_MSG_DEBUG("Top level calorimeter node created");
            
  auto caloNode = std::make_shared<CylinderContainerBlueprintNode>("CaloNode", AxisDirection::AxisZ);
  CylinderContainerBlueprintNode& caloBarrelCylinderNode = caloNode->addCylinderContainer("CaloBarrelCylinders", AxisDirection::AxisR);
  caloBarrelCylinderNode.setAttachmentStrategy(VolumeAttachmentStrategy::Gap);
  caloBarrelCylinderNode.setResizeStrategy(ResizeStrategy::Gap);
  
  ATH_MSG_DEBUG("EM Barrel container node created");

  //Barrel cylinders symmetric about z = 0
  std::vector<std::string> caloBarrelCylindricalLayerNames = {"PreSamplerB", "EMB1", "EMB2", "EMB3", "TileBar0", "TileBar1", "TileBar2"};
  for (unsigned int sampleIndex = 0; sampleIndex < caloBarrelCylindricalLayerNames.size(); ++sampleIndex) {
    auto& sampleName = caloBarrelCylindricalLayerNames.at(sampleIndex);
    addCylindricalTrackingVolumeToCaloNode(caloBarrelCylinderNode, sampleName, caloRegionSampleSurfaceMap[caloRegion::Global].at({sampleName, getSampleEnum(sampleName)}), sampleIndex, false);
  }

  CylinderContainerBlueprintNode& caloEndCapDiscNegativeZNode = caloNode->addCylinderContainer("CaloEndCapDiscNegativeZ", AxisDirection::AxisZ);
  caloEndCapDiscNegativeZNode.setAttachmentStrategy(VolumeAttachmentStrategy::Gap);
  caloEndCapDiscNegativeZNode.setResizeStrategy(ResizeStrategy::Gap);

  CylinderContainerBlueprintNode& caloEndCapDiscPositiveZNode = caloNode->addCylinderContainer("CaloEndCapDiscPositiveZ", AxisDirection::AxisZ);
  caloEndCapDiscPositiveZNode.setAttachmentStrategy(VolumeAttachmentStrategy::Gap);
  caloEndCapDiscPositiveZNode.setResizeStrategy(ResizeStrategy::Gap);
  
  for (unsigned int sampleIndex = 0; sampleIndex < m_caloDiscSampleList.size(); ++sampleIndex) {
    auto& sampleName = m_caloDiscSampleList.at(sampleIndex).first;
    addCylindricalTrackingVolumeToCaloNode(caloEndCapDiscNegativeZNode, sampleName+"NegZ", caloRegionSampleSurfaceMap[caloRegion::DiscNegativeZ].at({sampleName, getSampleEnum(sampleName)}), sampleIndex+caloBarrelCylindricalLayerNames.size(), true);
    addCylindricalTrackingVolumeToCaloNode(caloEndCapDiscPositiveZNode, sampleName+"PosZ", caloRegionSampleSurfaceMap[caloRegion::DiscPositiveZ].at({sampleName, getSampleEnum(sampleName)}), sampleIndex+caloBarrelCylindricalLayerNames.size()+m_caloDiscSampleList.size(), true);
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
                      std::map<caloRegion, caloSampleDDEElementsMap_t>& caloRegionSampleDDEElementsMap) const {


  //loop over all possible calo sampling layers
  //and create empty vectors of surfaces in the map

  //Create map between each calorimeter sampling and a vector of
  //cylinder surfaces. We can have N cylinders in a given sampling,
  //and the value of N is determined by how often the average radius 
  //calculated for a given phi ring, at fixed Z, changes by more than 
  //a tolerance value

  //Use the same loop to create map bwteeen sampling and vectors of DDE
  for (const auto & currentSample : m_caloCylinderSampleList) {
    caloRegionSampleSurfaceMap[caloRegion::Global][currentSample] = std::vector<std::shared_ptr<Surface> >();
    caloRegionSampleDDEElementsMap[caloRegion::Global][currentSample] = std::vector<const CaloDetDescrElement*>();
  }

  for (const auto & currentSample : m_caloDiscSampleList) {
    caloRegionSampleSurfaceMap[caloRegion::DiscNegativeZ][currentSample] = std::vector<std::shared_ptr<Surface> >();
    caloRegionSampleDDEElementsMap[caloRegion::DiscNegativeZ][currentSample] = std::vector<const CaloDetDescrElement*>();
    caloRegionSampleSurfaceMap[caloRegion::DiscPositiveZ][currentSample] = std::vector<std::shared_ptr<Surface> >();
    caloRegionSampleDDEElementsMap[caloRegion::DiscPositiveZ][currentSample] = std::vector<const CaloDetDescrElement*>();
  }

  //for each calo sampling collect all the DDE in a vector    
  for (const CaloDetDescrElement* theDDE : m_caloDetSecrMgr->element_range()){
    if (!theDDE){ 
      ATH_MSG_ERROR("Null pointer to CaloDetDescrElement");
      continue;
    }
    CaloCell_ID::CaloSample currentSample=theDDE->getSampling();
    caloRegionSampleDDEElementsMap[caloRegion::Global][{getSampleName(currentSample), currentSample}].push_back(theDDE);
    //check if sampling is in disc list
    if (std::find(m_caloDiscSampleList.begin(), m_caloDiscSampleList.end(), std::make_pair(getSampleName(currentSample), currentSample)) != m_caloDiscSampleList.end()) {
      //check if in negative or positive z
      if (theDDE->z() < 0.0) {
        caloRegionSampleDDEElementsMap[caloRegion::DiscNegativeZ][{getSampleName(currentSample), currentSample}].push_back(theDDE);
      }
      else {
        caloRegionSampleDDEElementsMap[caloRegion::DiscPositiveZ][{getSampleName(currentSample), currentSample}].push_back(theDDE);
      }
    }

  }

  auto sortAllLayersInZ = [&caloRegionSampleDDEElementsMap](const std::vector<std::pair<std::string, CaloCell_ID::CaloSample>>& caloSampleList) {
    for (const auto & currentSample : caloSampleList) {
      std::vector<const CaloDetDescrElement*> currentElements = caloRegionSampleDDEElementsMap[caloRegion::Global][currentSample];
      std::sort(currentElements.begin(), currentElements.end(), [](const CaloDetDescrElement* a, const CaloDetDescrElement* b) {return a->z() < b->z();});
      caloRegionSampleDDEElementsMap[caloRegion::Global][currentSample] = std::move(currentElements);
    }
  };

  //Sort the DDE, by Z, in all possible layers
  sortAllLayersInZ(m_caloCylinderSampleList);

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

void ActsTrk::CaloBlueprintNodeBuilder::generateCylinderSurfaces(caloSampleSurfaceMap_t& caloSampleSurfaceMap, caloSampleDDEElementsMap_t& caloSampleDDEElementsMap) const{

  for (const auto & currentSample : m_caloCylinderSampleList) {

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

            caloSampleSurfaceMap[currentSample].push_back(generateCylinderSurface(maxLArBRadius, minLArBRadius, lowZLarB, highZLarB));

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
      ATH_MSG_DEBUG("CYLINDER: Zero size Vector: Create cylinder for layer " << currentSample.first);   
      ATH_MSG_DEBUG("CYLINDER: Create Cylinder: Min and Max LAr B radius are " << minLArBRadius << " " << maxLArBRadius);
      ATH_MSG_DEBUG("CYLINDER: Create Cylinder: Min and Max LAr B z are " << lowZLarB << " " << highZLarB);
      caloSampleSurfaceMap[currentSample].push_back(generateCylinderSurface(maxLArBRadius, minLArBRadius, lowZLarB, highZLarB));
    }
  }   
}

std::shared_ptr<CylinderSurface> ActsTrk::CaloBlueprintNodeBuilder::generateCylinderSurface(const double& maxLArBRadius, const double& minLArBRadius, const double& lowZLarB, const double& highZLarB) const{

  //Characterise the dimensions of the  cylinder
  double LArBRadius = (maxLArBRadius + minLArBRadius) / 2.0;
  double LArBLength = std::abs(highZLarB - lowZLarB);

  ATH_MSG_DEBUG("Cylinder radius and length are " << LArBRadius << " and " << LArBLength);

  auto surface = Surface::makeShared<CylinderSurface>(Transform3::Identity(), LArBRadius, LArBLength/2);
    
  return surface;

}

void ActsTrk::CaloBlueprintNodeBuilder::generateDiscSurfaces(caloSampleSurfaceMap_t& caloSampleSurfaceMap, caloSampleDDEElementsMap_t& caloSampleDDEElementsMap) const{

  for (const auto & currentSample : m_caloDiscSampleList) {

        const std::vector<const CaloDetDescrElement*> & currentElements = caloSampleDDEElementsMap[currentSample];

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

  auto transform = Acts::Transform3(Acts::Translation3(0.0,0.0,z));
  auto surface = Surface::makeShared<DiscSurface>(transform, minLArBRadius, maxLArBRadius);

  return surface;

}



void ActsTrk::CaloBlueprintNodeBuilder::fillCaloDimensionsMap(caloDimensionMap_t& caloDimensionsMap, caloSampleDDEElementsMap_t& caloSampleDDEElementsMap) const{
 
  //initialise parameters we will use to find the dimensions of the calorimeter
  double minR = std::numeric_limits<double>::max();
  double maxR = 0.0;
  double minZ = std::numeric_limits<double>::max();
  double maxZ = -std::numeric_limits<double>::max();

  //define functions we will use
  auto checkMinR = [&minR](const CaloDetDescrElement* theDDE){double r = theDDE->r();if (r < minR) minR = r;};
  auto checkMaxR = [&maxR](const CaloDetDescrElement* theDDE){double r = theDDE->r();if (r > maxR) maxR = r;};
  auto checkMinMaxZ = [&minZ,&maxZ](const CaloDetDescrElement* theDDE){double z = theDDE->z();if (z > maxZ) maxZ = z;if (z < minZ) minZ = z;};
  auto calcHalfLengthZ = [&minZ, &maxZ](){return (maxZ - minZ) / 2.0;};

  //find minR from PreSamplerB
  for (const CaloDetDescrElement* theDDE : caloSampleDDEElementsMap.at({getSampleName(CaloCell_ID::PreSamplerB), CaloCell_ID::PreSamplerB})) checkMinR(theDDE);

  //find maxR from TileBar2, TileGap2 and Tile Ext2
  for (auto& currentSample : std::vector<std::pair<std::string, CaloCell_ID::CaloSample>>{{"TileBar2", CaloCell_ID::TileBar2}, {"TileGap2", CaloCell_ID::TileGap2}, {"TileExt2", CaloCell_ID::TileExt2}}) {
    for (const CaloDetDescrElement* theDDE : caloSampleDDEElementsMap.at(currentSample)) checkMaxR(theDDE);
  }

  ATH_MSG_DEBUG("Min R in PreSamplerB is " << minR);
  ATH_MSG_DEBUG("Max R in TileBar2, TileGap2 and TileExt2 is " << maxR);

  caloDimensionsMap["CaloMinR"] = minR;
  caloDimensionsMap["CaloMaxR"] = maxR;

  //find minZ and maxZ from all barrel samplings
  for (auto& currentSample : m_caloCylinderSampleList) {
    for (const CaloDetDescrElement* theDDE : caloSampleDDEElementsMap.at(currentSample)) checkMinMaxZ(theDDE);
  }

  caloDimensionsMap["CaloMinZ"] = minZ;
  caloDimensionsMap["CaloMaxZ"] = maxZ;

  double halfLengthZ = calcHalfLengthZ();
  ATH_MSG_DEBUG("Half length in Z for entire barrel is " << halfLengthZ);
  caloDimensionsMap["CaloHalfLengthZ"] = halfLengthZ;

}

void ActsTrk::CaloBlueprintNodeBuilder::addCylindricalTrackingVolumeToCaloNode(CylinderContainerBlueprintNode& containerNode, const std::string& volumeName,const std::vector<std::shared_ptr<Acts::Surface>>& surfaces, int layerIndex, const bool& isDisc) const{

  // Construct the container node with geometry identifier and layer, and add the surfaces to the layer.
  Acts::Experimental::GeometryIdentifierBlueprintNode& geoIdNode = containerNode.withGeometryIdentifier();
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
