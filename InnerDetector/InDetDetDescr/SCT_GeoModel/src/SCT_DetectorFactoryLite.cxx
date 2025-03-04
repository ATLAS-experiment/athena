/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#include "SCT_DetectorFactoryLite.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "DetDescrConditions/AlignableTransformContainer.h"

//
// GeoModel include files:
//
#include "GeoModelInterfaces/IGeoDbTagSvc.h"
#include "GeoModelInterfaces/StoredMaterialManager.h"

#include "GeoModelRead/ReadGeoModel.h"
#include "GeoModelKernel/GeoVolumeCursor.h"
#include "GeoModelKernel/GeoAlignableTransform.h"
#include "GeoModelKernel/GeoDefinitions.h"
#include "GeoModelKernel/GeoIdentifierTag.h"
#include "GeoModelKernel/GeoLogVol.h"
#include "GeoModelKernel/GeoMaterial.h"
#include "GeoModelKernel/GeoNameTag.h"
#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelKernel/GeoShape.h"
#include "GeoModelKernel/GeoShapeShift.h"
#include "GeoModelKernel/GeoShapeUnion.h"
#include "GeoModelKernel/GeoTransform.h"
#include "GeoModelKernel/GeoTube.h"
#include "GeoModelKernel/GeoVPhysVol.h"

#include "GeoModelUtilities/StoredAlignX.h"
#include "GeoModelUtilities/StoredPhysVol.h"

#include "ReadoutGeometryBase/InDetDD_Defs.h"
#include "ReadoutGeometryBase/SiCommonItems.h"
#include "InDetReadoutGeometry/Version.h"

#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "RDBAccessSvc/IRDBRecord.h"
#include "RDBAccessSvc/IRDBRecordset.h"

#include "SCT_Barrel.h"
#include "SCT_DataBase.h"
#include "SCT_Forward.h"
#include "SCT_GeneralParameters.h"
#include "SCT_GeoModelAthenaComps.h"
#include "SCT_Identifier.h"
#include "SCT_MaterialManager.h"

#include "SCT_ReadoutGeometry/SCT_ModuleSideDesign.h"

#include "StoreGate/StoreGateSvc.h"

#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/SystemOfUnits.h"

#include <iomanip>
#include <iostream>
#include <string>
 
using InDetDD::SCT_DetectorManager; 
using InDetDD::SiCommonItems; 

SCT_DetectorFactoryLite::SCT_DetectorFactoryLite(GeoModelIO::ReadGeoModel *sqliteReader,
						 SCT_GeoModelAthenaComps * athenaComps,
						 const SCT_Options & options)
  : InDetDD::DetectorFactoryBase(athenaComps),
    m_sqliteReader (sqliteReader)
{ 
  // Create the detector manager
  m_detectorManager = new SCT_DetectorManager(detStore());
  // Create the database
  m_db = std::make_unique<SCT_DataBase>(athenaComps);
  
  // Create the Si common items
  std::unique_ptr<InDetDD::SiCommonItems> commonItems{std::make_unique<InDetDD::SiCommonItems>(athenaComps->getIdHelper())};

  // Create the geometry manager.
  m_geometryManager = std::make_unique<SCT_GeometryManager>(m_db.get());
  m_geometryManager->setOptions(options);
  m_geometryManager->setCommonItems(commonItems.get());

  // Add SiCommonItems to SCT_DetectorManager to hold and delete it.
  m_detectorManager->setCommonItems(std::move(commonItems));

  m_useDynamicAlignFolders = options.dynamicAlignFolders();
  // Set Version information
  // Get the geometry tag
  IRDBRecordset_ptr switchSet
    = rdbAccessSvc()->getRecordsetPtr("SctSwitches", "");
  const IRDBRecord    *switches   = (*switchSet)[0];
  
  std::string layout = "Final";
  std::string description;
  if (!switches->isFieldNull("LAYOUT")) {
    layout = switches->getString("LAYOUT");
  }
  if (!switches->isFieldNull("DESCRIPTION")) {
    description = switches->getString("DESCRIPTION");
  }
  std::string versionName = switches->getString("VERSIONNAME");
  int versionMajorNumber = 3;
  int versionMinorNumber = 6;
  int versionPatchNumber = 0;
  InDetDD::Version version("", // Redundant when GeoModel is read from SQLite
                           versionName, 
                           layout, 
                           description, 
                           versionMajorNumber,
                           versionMinorNumber,
                           versionPatchNumber);
  m_detectorManager->setVersion(version);

  m_mapFPV = std::shared_ptr<FPVMap>(new FPVMap(m_sqliteReader->getPublishedNodes<std::string, GeoFullPhysVol*>("SCT")));
  m_mapAXF = std::shared_ptr<AXFMap>(new AXFMap(m_sqliteReader->getPublishedNodes<std::string, GeoAlignableTransform*>("SCT")));
} 
 
void SCT_DetectorFactoryLite::create(GeoPhysVol*)
{ 

  msg(MSG::INFO) << "Building SCT Detector." << endmsg;
  msg(MSG::INFO) << " " << m_detectorManager->getVersion().fullDescription() << endmsg;

  // Change precision.
  int oldPrecision = std::cout.precision(6);

  // The tree tops get added to world.
  const SCT_GeneralParameters * sctGeneral = m_geometryManager->generalParameters();

  std::string barrelLabel = "Barrel";
  std::string forwardPlusLabel = "EndcapA";
  std::string forwardMinusLabel = "EndcapC";

  bool barrelPresent       = sctGeneral->partPresent(barrelLabel);
  bool forwardPlusPresent  = sctGeneral->partPresent(forwardPlusLabel);
  bool forwardMinusPresent = sctGeneral->partPresent(forwardMinusLabel);

  //
  // The Barrel
  //  
  if (barrelPresent) {
   
    msg(MSG::DEBUG) << "Building the SCT Barrel." << endmsg;

    m_detectorManager->numerology().addBarrel(0);

    // Create the SCT Barrel
    SCT_Barrel sctBarrel("SCT_Barrel", m_detectorManager, m_geometryManager.get(), nullptr, m_sqliteReader, m_mapFPV, m_mapAXF);
      
    SCT_Identifier id{m_geometryManager->athenaComps()->getIdHelper()};
    id.setBarrelEC(0);
    //GeoVPhysVol * barrelPV =
    sctBarrel.build(id);
    GeoFullPhysVol *barrelPV = (*m_mapFPV)["SCT_Barrel"];
    GeoAlignableTransform * barrelTransform = (*m_mapAXF)["SCT_Barrel"];
    m_detectorManager->addTreeTop(barrelPV);

    // Store alignable transform
    m_detectorManager->addAlignableTransform(3, id.getWaferId(), barrelTransform, barrelPV);
    
  }

  //
  // The Positive Z Endcap (EndcapA)
  //  
  if (forwardPlusPresent) {

    msg(MSG::DEBUG) << "Building the SCT Endcap A (positive z)." << endmsg;

    m_detectorManager->numerology().addEndcap(2);

    // Create the Forward
    SCT_Forward sctForwardPlus("SCT_ForwardA", +2, m_detectorManager, m_geometryManager.get(), nullptr, m_sqliteReader, m_mapFPV, m_mapAXF);
    SCT_Identifier idFwdPlus{m_geometryManager->athenaComps()->getIdHelper()};
    idFwdPlus.setBarrelEC(2);
    //GeoVPhysVol * forwardPlusPV =
    sctForwardPlus.build(idFwdPlus);
    
    GeoFullPhysVol *forwardPlusPV = (*m_mapFPV)["SCT_ForwardPlus"];
    GeoAlignableTransform * fwdGeoTransformPlus = (*m_mapAXF)["SCT_ForwardPlus"];
    
    m_detectorManager->addTreeTop(forwardPlusPV);

    // Store alignable transform
    m_detectorManager->addAlignableTransform(3, idFwdPlus.getWaferId(), fwdGeoTransformPlus, forwardPlusPV);
  }
  
  //
  // The Negative Z Endcap (EndcapC)
  //  

  if (forwardMinusPresent) {

    msg(MSG::DEBUG) << "Building the SCT Endcap C (negative z)." << endmsg;

    m_detectorManager->numerology().addEndcap(-2);
    
    SCT_Forward sctForwardMinus("SCT_ForwardC", -2, m_detectorManager, m_geometryManager.get(), nullptr, m_sqliteReader, m_mapFPV, m_mapAXF);

    SCT_Identifier idFwdMinus{m_geometryManager->athenaComps()->getIdHelper()};
    idFwdMinus.setBarrelEC(-2);
    //GeoVPhysVol * forwardMinusPV =
    sctForwardMinus.build(idFwdMinus);
      
    GeoFullPhysVol *forwardMinusPV = (*m_mapFPV)["SCT_ForwardMinus"];
    GeoAlignableTransform * fwdGeoTransformMinus = (*m_mapAXF)["SCT_ForwardMinus"];
    m_detectorManager->addTreeTop(forwardMinusPV);


    // Store alignable transform
    m_detectorManager->addAlignableTransform(3, idFwdMinus.getWaferId(), fwdGeoTransformMinus, forwardMinusPV);
  }  

  // Set the neighbours
  m_detectorManager->initNeighbours();

  // Set maximum number of strips in numerology.
  for (int iDesign = 0;  iDesign <  m_detectorManager->numDesigns(); iDesign++) {
    m_detectorManager->numerology().setMaxNumPhiCells(m_detectorManager->getSCT_Design(iDesign)->cells());
  }

  // Register the keys and the level corresponding to the key
  // and whether it expects a global or local shift.
  // level 0: sensor, level 1: module, level 2, layer/disc, level 3: whole barrel/enccap


  if (!m_useDynamicAlignFolders){

    m_detectorManager->addAlignFolderType(InDetDD::static_run1);
    m_detectorManager->addFolder("/Indet/Align");
    m_detectorManager->addChannel("/Indet/Align/ID",3,InDetDD::global);
    m_detectorManager->addChannel("/Indet/Align/SCT",2,InDetDD::global);
    
    if (barrelPresent) {
      m_detectorManager->addChannel("/Indet/Align/SCTB1",1,InDetDD::local);
      m_detectorManager->addChannel("/Indet/Align/SCTB2",1,InDetDD::local);
      m_detectorManager->addChannel("/Indet/Align/SCTB3",1,InDetDD::local);
      m_detectorManager->addChannel("/Indet/Align/SCTB4",1,InDetDD::local);
    }
    if (forwardPlusPresent) {
      m_detectorManager->addChannel("/Indet/Align/SCTEA1",1,InDetDD::local);
      m_detectorManager->addChannel("/Indet/Align/SCTEA2",1,InDetDD::local);
      m_detectorManager->addChannel("/Indet/Align/SCTEA3",1,InDetDD::local);
      m_detectorManager->addChannel("/Indet/Align/SCTEA4",1,InDetDD::local);
      m_detectorManager->addChannel("/Indet/Align/SCTEA5",1,InDetDD::local);
      m_detectorManager->addChannel("/Indet/Align/SCTEA6",1,InDetDD::local);
      m_detectorManager->addChannel("/Indet/Align/SCTEA7",1,InDetDD::local);
      m_detectorManager->addChannel("/Indet/Align/SCTEA8",1,InDetDD::local);
      m_detectorManager->addChannel("/Indet/Align/SCTEA9",1,InDetDD::local);
    }
    if  (forwardMinusPresent) {
      m_detectorManager->addChannel("/Indet/Align/SCTEC1",1,InDetDD::local);
      m_detectorManager->addChannel("/Indet/Align/SCTEC2",1,InDetDD::local);
      m_detectorManager->addChannel("/Indet/Align/SCTEC3",1,InDetDD::local);
      m_detectorManager->addChannel("/Indet/Align/SCTEC4",1,InDetDD::local);
      m_detectorManager->addChannel("/Indet/Align/SCTEC5",1,InDetDD::local);
      m_detectorManager->addChannel("/Indet/Align/SCTEC6",1,InDetDD::local);
      m_detectorManager->addChannel("/Indet/Align/SCTEC7",1,InDetDD::local);
      m_detectorManager->addChannel("/Indet/Align/SCTEC8",1,InDetDD::local);
      m_detectorManager->addChannel("/Indet/Align/SCTEC9",1,InDetDD::local);
    }
  }
  
  else {
    m_detectorManager->addAlignFolderType(InDetDD::timedependent_run2);
    m_detectorManager->addGlobalFolder("/Indet/AlignL1/ID");
    m_detectorManager->addGlobalFolder("/Indet/AlignL2/SCT");
    m_detectorManager->addChannel("/Indet/AlignL1/ID",3,InDetDD::global);
    m_detectorManager->addChannel("/Indet/AlignL2/SCT",2,InDetDD::global);
    m_detectorManager->addFolder("/Indet/AlignL3");

    if (barrelPresent) {
      m_detectorManager->addChannel("/Indet/AlignL3/SCTB1",1,InDetDD::local);
      m_detectorManager->addChannel("/Indet/AlignL3/SCTB2",1,InDetDD::local);
      m_detectorManager->addChannel("/Indet/AlignL3/SCTB3",1,InDetDD::local);
      m_detectorManager->addChannel("/Indet/AlignL3/SCTB4",1,InDetDD::local);
    }
    if (forwardPlusPresent) {
      m_detectorManager->addChannel("/Indet/AlignL3/SCTEA1",1,InDetDD::local);
      m_detectorManager->addChannel("/Indet/AlignL3/SCTEA2",1,InDetDD::local);
      m_detectorManager->addChannel("/Indet/AlignL3/SCTEA3",1,InDetDD::local);
      m_detectorManager->addChannel("/Indet/AlignL3/SCTEA4",1,InDetDD::local);
      m_detectorManager->addChannel("/Indet/AlignL3/SCTEA5",1,InDetDD::local);
      m_detectorManager->addChannel("/Indet/AlignL3/SCTEA6",1,InDetDD::local);
      m_detectorManager->addChannel("/Indet/AlignL3/SCTEA7",1,InDetDD::local);
      m_detectorManager->addChannel("/Indet/AlignL3/SCTEA8",1,InDetDD::local);
      m_detectorManager->addChannel("/Indet/AlignL3/SCTEA9",1,InDetDD::local);
    }
    if  (forwardMinusPresent) {
      m_detectorManager->addChannel("/Indet/AlignL3/SCTEC1",1,InDetDD::local);
      m_detectorManager->addChannel("/Indet/AlignL3/SCTEC2",1,InDetDD::local);
      m_detectorManager->addChannel("/Indet/AlignL3/SCTEC3",1,InDetDD::local);
      m_detectorManager->addChannel("/Indet/AlignL3/SCTEC4",1,InDetDD::local);
      m_detectorManager->addChannel("/Indet/AlignL3/SCTEC5",1,InDetDD::local);
      m_detectorManager->addChannel("/Indet/AlignL3/SCTEC6",1,InDetDD::local);
      m_detectorManager->addChannel("/Indet/AlignL3/SCTEC7",1,InDetDD::local);
      m_detectorManager->addChannel("/Indet/AlignL3/SCTEC8",1,InDetDD::local);
      m_detectorManager->addChannel("/Indet/AlignL3/SCTEC9",1,InDetDD::local);
    }
  }

  // Return precision to its original value
  std::cout.precision(oldPrecision);

} 
 

const SCT_DetectorManager * SCT_DetectorFactoryLite::getDetectorManager() const
{
  return m_detectorManager;
}
 

