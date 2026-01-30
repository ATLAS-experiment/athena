/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloBlueprintNodeBuilder.h"

#include "Acts/Geometry/Blueprint.hpp"
#include "Acts/Geometry/PortalShell.hpp"
#include "Acts/Geometry/Volume.hpp"
#include "Acts/Geometry/StaticBlueprintNode.hpp"

#include <Acts/Utilities/AxisDefinitions.hpp>
#include <Acts/Geometry/ContainerBlueprintNode.hpp>

#include "CaloDetDescrUtils/CaloDetDescrBuilder.h"

#include "Acts/Surfaces/Surface.hpp"
#include "CaloIdentifier/CaloCell_ID.h"

using namespace Acts;
using namespace Acts::Experimental;

StatusCode ActsTrk::CaloBlueprintNodeBuilder::initialize() {
  ATH_MSG_DEBUG("Initializing CaloBlueprintNodeBuilder");

  m_caloDetSecrMgr = buildCaloDetDescrNoAlign(serviceLocator()
                                                                              , Athena::getMessageSvc());
  
  return StatusCode::SUCCESS;
}

std::shared_ptr<BlueprintNode> ActsTrk::CaloBlueprintNodeBuilder::buildBlueprintNode(const GeometryContext& /*gctx*/,
                                      std::shared_ptr<BlueprintNode>&& childNode) {

  caloSampleSurfaceMap_t caloSampleSurfaceMap;
  caloSampleDDEElementsMap_t caloSampleDDEElementsMap;
  fillMaps(caloSampleSurfaceMap, caloSampleDDEElementsMap);

  ATH_MSG_DEBUG("Have filled first two maps");

  caloDimensionMap_t caloDimensionMap;
  fillCaloDimensionsMap(caloDimensionMap, caloSampleDDEElementsMap);

  ATH_MSG_DEBUG("Have filled calo dimensions map");

  generateCylinderSurfaces(caloSampleSurfaceMap, caloSampleDDEElementsMap);

  ATH_MSG_DEBUG("Have generated calorimeter cylindrical surfaces");

  //The calo node is a container node that will hold the calo layers
  auto caloNode = std::make_shared<CylinderContainerBlueprintNode>("CaloNode", AxisDirection::AxisR);
  if (childNode) caloNode->addChild(std::move(childNode));

  ATH_MSG_DEBUG("Top level calorimeter node created");

  CylinderContainerBlueprintNode& EMBarrelCylinder = caloNode->addCylinderContainer("EMBarrel", AxisDirection::AxisR);
  // Use Gap attachment strategy to fill gaps between layers
  EMBarrelCylinder.setAttachmentStrategy(VolumeAttachmentStrategy::Gap);

  ATH_MSG_DEBUG("EM Barrel container node created");

  //Add all cylindrical layers that are symmetric about zero in Z.
  addCylindricalTrackingVolumeToCaloNode(EMBarrelCylinder, caloDimensionMap, "PreSamplerB", caloSampleSurfaceMap[CaloCell_ID::PreSamplerB]);
  addCylindricalTrackingVolumeToCaloNode(EMBarrelCylinder, caloDimensionMap, "EMB1", caloSampleSurfaceMap[CaloCell_ID::EMB1]);
  addCylindricalTrackingVolumeToCaloNode(EMBarrelCylinder, caloDimensionMap, "EMB2", caloSampleSurfaceMap[CaloCell_ID::EMB2]);
  addCylindricalTrackingVolumeToCaloNode(EMBarrelCylinder, caloDimensionMap, "EMB3", caloSampleSurfaceMap[CaloCell_ID::EMB3]);
  addCylindricalTrackingVolumeToCaloNode(EMBarrelCylinder, caloDimensionMap, "TileBar0", caloSampleSurfaceMap[CaloCell_ID::TileBar0]);
  addCylindricalTrackingVolumeToCaloNode(EMBarrelCylinder, caloDimensionMap, "TileBar1", caloSampleSurfaceMap[CaloCell_ID::TileBar1]);
  addCylindricalTrackingVolumeToCaloNode(EMBarrelCylinder, caloDimensionMap, "TileBar2", caloSampleSurfaceMap[CaloCell_ID::TileBar2]);

  ATH_MSG_DEBUG("Have added all Barrel layers to EMBarrelCylinder node");

  return caloNode;
}

StatusCode ActsTrk::CaloBlueprintNodeBuilder::finalize() {
  ATH_MSG_DEBUG("Finalizing CaloBlueprintNodeBuilder");
  return StatusCode::SUCCESS;
}

void  ActsTrk::CaloBlueprintNodeBuilder::fillMaps(caloSampleSurfaceMap_t& caloSampleSurfaceMap, caloSampleDDEElementsMap_t& caloSampleDDEElementsMap) const {


  //loop over all possible calo sampling layers
  //and create empty vectors of surfaces in the map

  //Create map between each calorimeter sampling and a vector of
  //cylinder surfaces. We can have N cylinders in a given sampling,
  //and the value of N is determined by how often the average radius 
  //calculated for a given phi ring, at fixed Z, changes by more than 
  //a tolerance value

  //Use the same loop to create map bwteeen sampling and vectors of DDE
  for (auto currentSample : m_caloCylinderSampleList) {
    caloSampleSurfaceMap[currentSample] = std::vector<std::shared_ptr<Surface> >();
    caloSampleDDEElementsMap[currentSample] = std::vector<const CaloDetDescrElement*>();
  }

  for (auto currentSample : m_caloDiscSampleList) {
    caloSampleSurfaceMap[currentSample] = std::vector<std::shared_ptr<Surface> >();
    caloSampleDDEElementsMap[currentSample] = std::vector<const CaloDetDescrElement*>();
  }

  //for each calo sampling collect all the DDE in a vector    
  for (const CaloDetDescrElement* theDDE : m_caloDetSecrMgr->element_range()){
    if (!theDDE){ 
      ATH_MSG_ERROR("Null pointer to CaloDetDescrElement");
      continue;
    }
    CaloCell_ID::CaloSample currentSample=theDDE->getSampling();
    caloSampleDDEElementsMap[currentSample].push_back(theDDE);
  }

  auto sortAllLayersInZ = [&caloSampleDDEElementsMap](const std::vector<CaloCell_ID::CaloSample>& caloSampleList) {
    for (auto currentSample : caloSampleList) {
      std::vector<const CaloDetDescrElement*> currentElements = caloSampleDDEElementsMap[currentSample];
      std::sort(currentElements.begin(), currentElements.end(), [](const CaloDetDescrElement* a, const CaloDetDescrElement* b) {return a->z() < b->z();});
      caloSampleDDEElementsMap[currentSample] = std::move(currentElements);
    }
  };

  //Sort the DDE, by Z, in all possible layers
  sortAllLayersInZ(m_caloCylinderSampleList);
  sortAllLayersInZ(m_caloDiscSampleList);

}

void ActsTrk::CaloBlueprintNodeBuilder::generateCylinderSurfaces(caloSampleSurfaceMap_t& caloSampleSurfaceMap, caloSampleDDEElementsMap_t& caloSampleDDEElementsMap) const{

  for (auto currentSample : m_caloCylinderSampleList) {

    std::vector<const CaloDetDescrElement*> currentElements = caloSampleDDEElementsMap[currentSample];

    double maxLArBRadius = 0.0, minLArBRadius = 999999.0;
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
    double initialZ = -999999.0;

    for (const CaloDetDescrElement* theDDE : currentElements){

      double z = theDDE->z();   
      double radius = theDDE->r();

      ATH_MSG_DEBUG(" Calo Sampling is " << currentSample);            

      if (firstCellInPhiRing) {
        ATH_MSG_DEBUG("First Cell in phi ring " << currentSample << " has z = " << z << " and r = " << radius);
        initialZ = z;
        firstCellInPhiRing = false;
      }

      if (firstPhiRing) {
        ATH_MSG_DEBUG("First Cell in layer " << currentSample << " has z = " << z << " and r = " << radius);
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
            ATH_MSG_DEBUG("CYLINDER: Create cylinder for layer " << currentSample);                
            ATH_MSG_DEBUG("CYLINDER: Create Cylinder: Min and Max LAr B radius are " << minLArBRadius << " " << maxLArBRadius);
            ATH_MSG_DEBUG("CYLINDER: Create Cylinder: Min and Max LAr B z are " << lowZLarB << " " << highZLarB);

            caloSampleSurfaceMap[currentSample].push_back(generateCylinderSurface(maxLArBRadius, minLArBRadius, lowZLarB, highZLarB));

            //reset the dimensions of the cylinder to the initial conditions, in 
           //preparation for the next cylinder
           firstPhiRing = true;
           minLArBRadius = 999999.0;
           maxLArBRadius = 0.0;
           lowZLarB = 0.0;
           highZLarB = 0.0;
          }//if radius changes by more than tolerance
        }//if at least one cell in phi (should always be the case!)
        else ATH_MSG_ERROR("phiCounter is zero!");
      }//if z has changed
    }//loop over calorimeter DDE

    if (0 == caloSampleSurfaceMap[currentSample].size()){
      ATH_MSG_DEBUG("CYLINDER: Zero size Vector: Create cylinder for layer " << currentSample);   
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

  auto surface = Surface::makeShared<CylinderSurface>(Transform3::Identity(), LArBRadius, LArBLength);
    
  return surface;

}

void ActsTrk::CaloBlueprintNodeBuilder::fillCaloDimensionsMap(caloDimensionMap_t& caloDimensionsMap, caloSampleDDEElementsMap_t& caloSampleDDEElementsMap) const{
 
  //initialise parameters we will use to find the dimensions of the calorimeter
  double minR = 99999999999999;
  double maxR = 0.0;
  double minZ = 99999999999999;
  double maxZ = -99999999999999;    

  //define functions we will use
  auto checkMinR = [&minR](const CaloDetDescrElement* theDDE){double r = theDDE->r();if (r < minR) minR = r;};
  auto checkMaxR = [&maxR](const CaloDetDescrElement* theDDE){double r = theDDE->r();if (r > maxR) maxR = r;};
  auto checkMinMaxZ = [&minZ,&maxZ](const CaloDetDescrElement* theDDE){double z = theDDE->z();if (z > maxZ) maxZ = z;if (z < minZ) minZ = z;};
  auto calcHalfLengthZ = [&minZ, &maxZ](){return (maxZ - minZ) / 2.0;};

  //function to use the above to get minR, maxR and halfLengthZ
  auto getMinRMaxRHalfLengthZ = [&minR, &maxR, &minZ, &maxZ, &caloSampleDDEElementsMap, &checkMinR, &checkMaxR, &checkMinMaxZ, &calcHalfLengthZ](CaloCell_ID::CaloSample currentSample) {

    minR = 99999999999999;
    maxR = 0.0;
    minZ = 99999999999999;
    maxZ = -99999999999999;    

    for (const CaloDetDescrElement* theDDE : caloSampleDDEElementsMap[currentSample]) {
      checkMinR(theDDE);
      checkMaxR(theDDE);   
      checkMinMaxZ(theDDE);
    }

    return calcHalfLengthZ();
  };

  //find minR from PreSamplerB
  for (const CaloDetDescrElement* theDDE : caloSampleDDEElementsMap[CaloCell_ID::PreSamplerB]) checkMinR(theDDE); 

  //find maxR from TileBar2, TileGap2 and Tile Ext2
  std::vector<const CaloDetDescrElement*> DDE_TileBar2 = caloSampleDDEElementsMap[CaloCell_ID::TileBar2];
  std::vector<const CaloDetDescrElement*> DDE_TileGap2 = caloSampleDDEElementsMap[CaloCell_ID::TileGap2];
  std::vector<const CaloDetDescrElement*> DDE_TileExt2 = caloSampleDDEElementsMap[CaloCell_ID::TileExt2];

  for (auto currentSample : {CaloCell_ID::TileBar2, CaloCell_ID::TileGap2, CaloCell_ID::TileExt2}) {
    for (const CaloDetDescrElement* theDDE : caloSampleDDEElementsMap[currentSample]) checkMaxR(theDDE);
  }

  ATH_MSG_DEBUG("Min R in PreSamplerB is " << minR);
  ATH_MSG_DEBUG("Max R in TileBar2, TileGap2 and TileExt2 is " << maxR);

  caloDimensionsMap["CaloMinR"] = minR;
  caloDimensionsMap["CaloMaxR"] = maxR;

  //find minZ and maxZ from all barrel samplings
  for (auto currentSample : {CaloCell_ID::PreSamplerB, CaloCell_ID::EMB1, CaloCell_ID::EMB2, CaloCell_ID::EMB3, CaloCell_ID::TileBar0, CaloCell_ID::TileBar1, CaloCell_ID::TileBar2, CaloCell_ID::TileGap1, CaloCell_ID::TileGap2, CaloCell_ID::TileExt0, CaloCell_ID::TileExt1, CaloCell_ID::TileExt2}) {
    for (const CaloDetDescrElement* theDDE : caloSampleDDEElementsMap[currentSample]) checkMinMaxZ(theDDE);
  }

  caloDimensionsMap["CaloMinZ"] = minZ;
  caloDimensionsMap["CaloMaxZ"] = maxZ;

  double halfLengthZ = calcHalfLengthZ();
  ATH_MSG_DEBUG("Half length in Z for entire barrel is " << halfLengthZ);
  caloDimensionsMap["CaloHalfLengthZ"] = halfLengthZ;

  auto putMinRMaxRHalfLengthZInMap = [&caloDimensionsMap,&minR,&maxR,&halfLengthZ](const std::string& sampleString){
    //PresamplerB, TileBar0/2, TileGap1/2, TileExt0/1/2 return the same radial value for all DDE, so need to make a small offset because a zero thickness
    //cylinder is not allowed by Acts volumes etc.
    float smallOffset = 0.1;
    if (std::fabs((maxR - minR)) < smallOffset) maxR = minR + smallOffset;
    caloDimensionsMap[sampleString + "MinR"] = minR;
    caloDimensionsMap[sampleString + "MaxR"] = maxR;
    caloDimensionsMap[sampleString + "HalfLengthZ"] = halfLengthZ;
  };

  halfLengthZ = getMinRMaxRHalfLengthZ(CaloCell_ID::PreSamplerB);
  putMinRMaxRHalfLengthZInMap("PreSamplerB");
  halfLengthZ = getMinRMaxRHalfLengthZ(CaloCell_ID::EMB1);
  putMinRMaxRHalfLengthZInMap("EMB1");
  halfLengthZ = getMinRMaxRHalfLengthZ(CaloCell_ID::EMB2);
  putMinRMaxRHalfLengthZInMap("EMB2");
  halfLengthZ = getMinRMaxRHalfLengthZ(CaloCell_ID::EMB3);
  putMinRMaxRHalfLengthZInMap("EMB3");
  halfLengthZ = getMinRMaxRHalfLengthZ(CaloCell_ID::TileBar0);
  putMinRMaxRHalfLengthZInMap("TileBar0");
  halfLengthZ = getMinRMaxRHalfLengthZ(CaloCell_ID::TileBar1);
  putMinRMaxRHalfLengthZInMap("TileBar1");
  halfLengthZ = getMinRMaxRHalfLengthZ(CaloCell_ID::TileBar2);
  putMinRMaxRHalfLengthZInMap("TileBar2");
  halfLengthZ = getMinRMaxRHalfLengthZ(CaloCell_ID::TileGap1);

}

void ActsTrk::CaloBlueprintNodeBuilder::addCylindricalTrackingVolumeToCaloNode(CylinderContainerBlueprintNode& containerNode, caloDimensionMap_t& caloDimensionMap, const std::string& volumeName,const std::vector<std::shared_ptr<Surface>>& surfaces) const{

  CylinderContainerBlueprintNode& cylinder = containerNode.addCylinderContainer(volumeName, AxisDirection::AxisZ);

  ATH_MSG_DEBUG("Creating TrackingVolume for " << volumeName << " with minR, maxR and halfLengthZ of " << caloDimensionMap[volumeName+"MinR"] << ", " << caloDimensionMap[volumeName+"MaxR"] << " and " << caloDimensionMap[volumeName+"HalfLengthZ"]);

  auto trackingVolume = std::make_unique<TrackingVolume>(
      Transform3::Identity(),
      std::make_shared<CylinderVolumeBounds>(caloDimensionMap[volumeName+"MinR"], caloDimensionMap[volumeName+"MaxR"], caloDimensionMap[volumeName+"HalfLengthZ"]),
      volumeName);

  for (auto surface : surfaces) trackingVolume->addSurface(std::move(surface));


  cylinder.addStaticVolume(std::move(trackingVolume));

}