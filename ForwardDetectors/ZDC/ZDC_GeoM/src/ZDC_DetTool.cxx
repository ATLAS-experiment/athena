/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "ZDC_DetTool.h"
#include "ZDC_DetFactory.h" 
#include "ZDC_ZDCModule.h" 
#include "ZDC_RPDModule.h" 
#include "ZDC_BRANModule.h" 
#include "ZDC_DetManager.h" 
#include "ZdcConditions/ZdcGeometryDB.h" 
#include "GeoModelUtilities/GeoModelExperiment.h"
#include "GaudiKernel/IService.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "StoreGate/StoreGateSvc.h"
#include "AthenaKernel/getMessageSvc.h"
#include <GeoModelKernel/GeoDefinitions.h>
#include <memory>

ZDC_DetTool::ZDC_DetTool(const std::string& type, const std::string& name, const IInterface* parent)
  : GeoModelTool(type, name, parent)
{

  if (msgLevel(MSG::DEBUG))
    msg(MSG::DEBUG) << "INSIDE CONSTRUCTOR OF DETTOOL"                            << endmsg
		    << "INSIDE CONSTRUCTOR OF DETTOOL string& type "      << type << endmsg
		    << "INSIDE CONSTRUCTOR OF DETTOOL std::string& name " << name << endmsg;
}

ZDC_DetTool::~ZDC_DetTool()
{
  // This will need to be modified once we register the Toy DetectorNode in the Transient Detector Store
  
  if (nullptr != m_detector) {

    delete m_detector;
    m_detector = nullptr;
  }
}

StatusCode ZDC_DetTool::create()
{ 
  if (msgLevel(MSG::DEBUG)) msg(MSG::DEBUG) << " Building ZDC geometry " << endmsg;
  
  // Locate the top level experiment node  
  GeoModelExperiment* theExpt = nullptr;
  if (StatusCode::SUCCESS != detStore()->retrieve(theExpt, "ATLAS")) {
    if (msgLevel(MSG::ERROR)) msg(MSG::ERROR) << " Could not find GeoModelExperiment ATLAS " << endmsg; 
    return (StatusCode::FAILURE); 
  } 
  
  // Create the ZDC Detector Factory
  ZDC_DetFactory theZDCFactory(detStore().operator->());

  //Retrieve the ZDC geometry from the database

  const IZdcGeometryDB *theZdcGeoDB = ZdcGeoDBGeometryDB::getInstance();  
  const nlohmann::json& zdcGeo = theZdcGeoDB->getDB();
  
  /*************************************************
  * Get the TAN/TAXN slots and hold onto the transform
  **************************************************/
  std::array<GeoTrf::Transform3D, 2> tanTrf;
  for (auto slot : zdcGeo["TAN/TAXN"].items()) {
    // Retrieve all of the values from the json file
    int side = slot.value()["side"].get<int>();
    int iside = side == -1 ? 0 : 1;
    double x = slot.value()["x"].get<double>();
    double y = slot.value()["y"].get<double>();
    double z = slot.value()["z"].get<double>();
    double height = slot.value()["height"].get<double>();
    double width = slot.value()["width"].get<double>();
    double depth = slot.value()["depth"].get<double>();
    std::string name = slot.value()["name"].get<std::string>();
    tanTrf.at(iside) = GeoTrf::Translate3D(x * Gaudi::Units::mm, y * Gaudi::Units::mm, z * Gaudi::Units::mm);

    // Add the TAN/TAXN slot to the factory
    theZDCFactory.setTANSlot(iside, width, height, depth, tanTrf.at(iside), name);
  }

  /*************************************************
  * Add the ZDC/RPD/BRAN modules to the factory
  **************************************************/
  for (auto det : zdcGeo["Detector"].items()) {
    std::unique_ptr<ZDC_ModuleBase> pDet;

    std::string name = det.value()["name"].get<std::string>();
    int side = det.value()["side"].get<int>();
    int iside = side == -1 ? 0 : 1;
    int module = det.value()["module"].get<int>();
    double x = det.value()["x"].get<double>();
    double y = det.value()["y"].get<double>();
    double z = det.value()["z"].get<double>();
    double q = det.value()["q"].get<double>();
    double i = det.value()["i"].get<double>();
    double j = det.value()["j"].get<double>();
    double k = det.value()["k"].get<double>();

    if (std::string(det.key()).find("ZDC") != std::string::npos) {
      pDet = std::make_unique<ZDC_ZDCModule>( name, side, module, det.value()["type"].get<int>() );

    }else if (std::string(det.key()).find("RPD") != std::string::npos) {
      pDet = std::make_unique<ZDC_RPDModule>( name, side, module );

    }else if (std::string(det.key()).find("BRAN") != std::string::npos) {
      pDet = std::make_unique<ZDC_BRANModule>( name, side, module );

    }else {
      if (msgLevel(MSG::ERROR)) msg(MSG::ERROR) << "Unknown detector type " << det.key() << endmsg;
      return StatusCode::FAILURE;
    }

    // Set the transform and subtract the TAN translation for this side since that's the mother volume of the detector
    pDet->setTransform(GeoTrf::Translate3D(x - tanTrf.at(iside).translation().x(), y - tanTrf.at(iside).translation().y(), z - tanTrf.at(iside).translation().z()) * GeoTrf::Rotation3D(q, i, j, k) );

    // Add the module to the factory
    theZDCFactory.addModule(std::move(pDet));
  }

  if (nullptr == m_detector) { // Create the ZDCDetectorNode instance
    
    try { 
      // This strange way of casting is to avoid an utterly brain damaged compiler warning.
      GeoPhysVol* world = &*theExpt->getPhysVol();
      theZDCFactory.create(world);  
    } 
    catch (const std::bad_alloc&) {
      
      if (msgLevel(MSG::FATAL)) msg(MSG::FATAL) << "Could not create new ZDC DetectorNode!" << endmsg;
      return StatusCode::FAILURE; 
    }
    
    // Register the ZDC DetectorNode instance with the Transient Detector Store
    theExpt->addManager(theZDCFactory.getDetectorManager());
    if(detStore()->record(theZDCFactory.getDetectorManager(),theZDCFactory.getDetectorManager()->getName())==StatusCode::SUCCESS){
      return StatusCode::SUCCESS;}
    else{
      msg(MSG::FATAL) << "Could not register ZDC detector manager" << endmsg;}

  }
  
  return StatusCode::FAILURE;
}
