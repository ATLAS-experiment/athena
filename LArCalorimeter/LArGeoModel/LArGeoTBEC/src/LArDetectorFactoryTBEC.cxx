/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// LArDetectorFactory.cxx
// 24-Aug-2006 : V. Niess. Put back initialisation for
// VDetectorParameters according to J. Toth scheme. 

#include "LArDetectorFactoryTBEC.h"

#include "LArDetectorConstructionTBEC.h"

#include "GeoModelKernel/GeoPhysVol.h"  
#include "GeoModelKernel/GeoTransform.h"
#include "GeoModelKernel/GeoNameTag.h"
#include "GeoModelKernel/GeoShapeUnion.h"

#include "CLHEP/Geometry/Transform3D.h" 

#include "GeoModelInterfaces/StoredMaterialManager.h"

#include "StoreGate/StoreGateSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/Bootstrap.h"
#include "GeoModelUtilities/StoredPhysVol.h"

#include "LArReadoutGeometry/EMECDetectorManager.h"
#include "LArReadoutGeometry/EMECDetectorRegion.h"
#include "LArReadoutGeometry/EMECDetDescr.h"
#include "GeoModelKernel/CellBinning.h"

#include "AthenaKernel/getMessageSvc.h"

// V.N :  Patch LAr materials
#include "LArGeoCode/LArMaterialManager.h"
#include "LArGeoRAL/RAL.h"
#include "LArGeoCode/VDetectorParameters.h"

// Place the cryostats into a container physical volume.
void LArGeo::LArDetectorFactoryTBEC::create( GeoPhysVol* a_container )
{
  // V.N :  Patch LAr materials ??? is this needed?
  VDetectorParameters::SetInstance(std::make_unique<LArGeo::RAL>());

  EMECDetectorManager *emecDetectorManager = new EMECDetectorManager();
  
  // Get access to the material manager:
  MsgStream log(Athena::getMessageSvc(), "LArGeo::LArDetectorFactoryTBEC");

  log << MSG::INFO << "+++++++++++++++++++++++++++++++++++++++++++++++++++++" << endmsg;
  log << MSG::INFO << "+                                                   +" << endmsg;
  log << MSG::INFO << "+         HELLO from LAr::DetectorFactoryTBEC       +" << endmsg;
  log << MSG::INFO << "+                                                   +" << endmsg;
  log << MSG::INFO << "+++++++++++++++++++++++++++++++++++++++++++++++++++++" << endmsg;
  
  SmartIF<StoreGateSvc> detStore{Gaudi::svcLocator()->service("DetectorStore")};
  if(!detStore.isValid()) {
    throw std::runtime_error("Error in LArDetectorFactoryTBEC, cannot access DetectorStore");
  }
  
  // V.N :  Patch LAr materials
  LArMaterialManager lArMaterialManager(detStore);
  lArMaterialManager.buildMaterials();
  
  LArDetectorConstructionTBEC CryostatConstructionTB;
  
  PVLink Envelope = nullptr;
  
  Envelope = CryostatConstructionTB.GetEnvelope();
  
  a_container->add(new GeoNameTag("LAr"));
  a_container->add(Envelope);
  
  double startPhi = M_PI/2-M_PI/8 - 2*M_PI/768/2;
  double endPhi   = M_PI/2+M_PI/8 - 2*M_PI/768/2;

  StoredPhysVol *sEmecOuterWheel;   
  if (StatusCode::SUCCESS==detStore->retrieve(sEmecOuterWheel, "EMEC_OUTER_WHEEL_POS" )) {        
    GeoIntrusivePtr<GeoFullPhysVol>emecEnvelope= sEmecOuterWheel->getPhysVol();


    // Outer Wheel Sampling 1 Region 0:
    {
      CellBinning phiBinning(startPhi,endPhi,8,12);
      EMECDetDescr *detDescr = new EMECDetDescr(emecDetectorManager,1,0,0,phiBinning);
      EMECDetectorRegion *region = new EMECDetectorRegion(emecEnvelope,detDescr,EMECDetectorRegion::POS);
      emecDetectorManager->addDetectorRegion(region);
    }
    
    // Outer Wheel Sampling 1 Region 1:
    {
      CellBinning phiBinning(startPhi,endPhi,8,12);
      EMECDetDescr *detDescr = new EMECDetDescr(emecDetectorManager,1,1,0,phiBinning);
      EMECDetectorRegion *region = new EMECDetectorRegion(emecEnvelope,detDescr,EMECDetectorRegion::POS);
      emecDetectorManager->addDetectorRegion(region);
    }
    // Outer Wheel Sampling 1 Region 2:
    {
      CellBinning phiBinning(startPhi,endPhi,8,12);
      EMECDetDescr *detDescr = new EMECDetDescr(emecDetectorManager,1,2,0,phiBinning);
      EMECDetectorRegion *region = new EMECDetectorRegion(emecEnvelope,detDescr,EMECDetectorRegion::POS);
      emecDetectorManager->addDetectorRegion(region);
    }
    // Outer Wheel Sampling 1 Region 3:
    {
      CellBinning phiBinning(startPhi,endPhi,8,12);
      EMECDetDescr *detDescr = new EMECDetDescr(emecDetectorManager,1,3,0,phiBinning);
      EMECDetectorRegion *region = new EMECDetectorRegion(emecEnvelope,detDescr,EMECDetectorRegion::POS);
      emecDetectorManager->addDetectorRegion(region);
    }
    // Outer Wheel Sampling 1 Region 4:
    {
      CellBinning phiBinning(startPhi,endPhi,8,12);
      EMECDetDescr *detDescr = new EMECDetDescr(emecDetectorManager,1,4,0,phiBinning);
      EMECDetectorRegion *region = new EMECDetectorRegion(emecEnvelope,detDescr,EMECDetectorRegion::POS);
      emecDetectorManager->addDetectorRegion(region);
    }
    // Outer Wheel Sampling 1 Region 5:
    {
      CellBinning phiBinning(startPhi,endPhi,8,12);
      EMECDetDescr *detDescr = new EMECDetDescr(emecDetectorManager,1,5,0,phiBinning);
      EMECDetectorRegion *region = new EMECDetectorRegion(emecEnvelope,detDescr,EMECDetectorRegion::POS);
      emecDetectorManager->addDetectorRegion(region);
    }
      // Outer Wheel Sampling 2 Region 0:
    {
      CellBinning phiBinning(startPhi,endPhi,32,48);
      EMECDetDescr *detDescr = new EMECDetDescr(emecDetectorManager,2,0,0,phiBinning);
      EMECDetectorRegion *region = new EMECDetectorRegion(emecEnvelope,detDescr,EMECDetectorRegion::POS);
      emecDetectorManager->addDetectorRegion(region);
    }
    // Outer Wheel Sampling 2 Region 1:
    {
      CellBinning phiBinning(startPhi,endPhi,32,48);
      EMECDetDescr *detDescr = new EMECDetDescr(emecDetectorManager,2,1,0,phiBinning);
      EMECDetectorRegion *region = new EMECDetectorRegion(emecEnvelope,detDescr,EMECDetectorRegion::POS);
      emecDetectorManager->addDetectorRegion(region);
    }
    // Outer Wheel Sampling 3 Region 0:
    {
      CellBinning phiBinning(startPhi,endPhi,32,48);
      EMECDetDescr *detDescr = new EMECDetDescr(emecDetectorManager,3,0,0,phiBinning);
      EMECDetectorRegion *region = new EMECDetectorRegion(emecEnvelope,detDescr,EMECDetectorRegion::POS);
      emecDetectorManager->addDetectorRegion(region);
    }
  }
  StoredPhysVol *sEmecInnerWheel;   
  if (StatusCode::SUCCESS==detStore->retrieve(sEmecInnerWheel, "EMEC_INNER_WHEEL_POS" )) {        
    GeoIntrusivePtr<GeoFullPhysVol>emecEnvelope= sEmecInnerWheel->getPhysVol();
    // Inner Wheel Sampling 1 Region 0:
    {
      CellBinning phiBinning(startPhi,endPhi,8,12);
      EMECDetDescr *detDescr = new EMECDetDescr(emecDetectorManager,1,0,1,phiBinning);
      EMECDetectorRegion *region = new EMECDetectorRegion(emecEnvelope,detDescr,EMECDetectorRegion::POS);
      emecDetectorManager->addDetectorRegion(region);
    }
    // Inner Wheel Sampling 2 Region 0:
    
    {
      CellBinning phiBinning(startPhi,endPhi,8,12);
      EMECDetDescr *detDescr = new EMECDetDescr(emecDetectorManager,2,0,1,phiBinning);
      EMECDetectorRegion *region = new EMECDetectorRegion(emecEnvelope,detDescr,EMECDetectorRegion::POS);
      emecDetectorManager->addDetectorRegion(region);
    }
  }


  StoredPhysVol *sPresamplerEnvelope;   
  if (StatusCode::SUCCESS==detStore->retrieve(sPresamplerEnvelope, "PRESAMPLER_EC_POS" )) {        
    GeoIntrusivePtr<GeoFullPhysVol> PresamplerEnvelope=sPresamplerEnvelope->getPhysVol();
    CellBinning presamplerPhiBinning(startPhi,endPhi,8,12);
    EMECDetDescr *presamplerDetDescr = new EMECDetDescr(emecDetectorManager,0,0,0,presamplerPhiBinning);
    EMECDetectorRegion *presamplerRegion = new EMECDetectorRegion(PresamplerEnvelope,presamplerDetDescr,EMECDetectorRegion::POS);
    emecDetectorManager->addDetectorRegion(presamplerRegion);
  }                                                                  
  detStore->record(emecDetectorManager,emecDetectorManager->getName()).ignore();
  m_detectorManager = new LArDetectorManager(nullptr,emecDetectorManager,nullptr,nullptr);
  m_detectorManager->addTreeTop(Envelope);

}


const LArDetectorManager* LArGeo::LArDetectorFactoryTBEC::getDetectorManager() const
{
  return m_detectorManager;
}

