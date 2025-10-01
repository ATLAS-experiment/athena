/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#include <utility>

#include "DBM_Module.h"

#include "GeoModelKernel/GeoTransform.h"
#include "GeoModelKernel/GeoAlignableTransform.h"
#include "GeoModelKernel/GeoNameTag.h"
#include "GeoModelKernel/GeoBox.h"
#include "GaudiKernel/SystemOfUnits.h"

#include "Identifier/Identifier.h"
#include "InDetIdentifier/PixelID.h"
#include "PixelReadoutGeometry/PixelDetectorManager.h"

#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "PixelReadoutGeometry/PixelModuleDesign.h"
#include "PixelGeoUtils.h"
using namespace InDetDD;

DBM_Module::DBM_Module(InDetDD::PixelDetectorManager* ddmgr,
                       PixelGeometryManager* mgr,
		       GeoModelIO::ReadGeoModel* sqliteReader,
                       std::shared_ptr<std::map<std::string, GeoFullPhysVol*>> mapFPV,
                       std::shared_ptr<std::map<std::string, GeoAlignableTransform*>> mapAX)
  : GeoVPixelFactory (ddmgr, mgr, sqliteReader, std::move(mapFPV), std::move(mapAX))
{

  double thickness = 0.5;

  int circuitsPerPhi = 1;
  int circuitsPerEta = 1;
  int cellRowPerCirc = 80;
  int cellColPerCirc = 336;
  int rowsPerCircuit = 80;
  int columnsPerCircuit = 336;
  int readoutSide = 1;

  double pitchEtaLongEnd =  0.05;
  double pitchEtaLong =  0.05;
  double pitchPhi = 0.25;
  double pitchEta = 0.05;

  constexpr auto readoutTechnology = InDetDD::PixelReadoutTechnology::FEI4;
  constexpr auto kNDirections = InDetDD::detail::kNDirections;
  constexpr auto kNPixelLocations = InDetDD::detail::kNPixelLocations;
  PixelDiodeTree diode_tree = InDetDD::detail::makePixelDiodeTree(m_gmt_mgr,
                                                   readoutTechnology,
                                                   std::array<int,kNDirections>{circuitsPerPhi,circuitsPerEta},     // [0]=phi/row, [1]=eta/column
                                                   std::array<int,kNDirections>{rowsPerCircuit,columnsPerCircuit}, // [0]=phi/row, [1]=eta/column
                                                   std::array<std::array<double,kNDirections>,kNPixelLocations>{   // regular/central,longEnd/outer,long/inner
                                                      std::array<double,kNDirections>{pitchPhi,pitchEta},
                                                      std::array<double,kNDirections>{0.,pitchEtaLongEnd},
                                                      std::array<double,kNDirections>{0.,pitchEtaLong}});
 
  std::unique_ptr<PixelModuleDesign> p_dbmdesign = std::make_unique<PixelModuleDesign>(thickness,
							     circuitsPerPhi,
							     circuitsPerEta,
							     cellColPerCirc,
							     cellRowPerCirc,
							     columnsPerCircuit,
							     rowsPerCircuit,
							     std::move(diode_tree),
							     InDetDD::electrons,
							     readoutSide,
							     false,              /* 3D */
							     InDetDD::Undefined, /* detector type */
							     readoutTechnology);


  m_design = m_DDmgr->addDesign(std::move(p_dbmdesign));


}

GeoVPhysVol* DBM_Module::Build()  
{
  const PixelID * idHelper = m_gmt_mgr->getIdHelper();
  int dbmdet = 4*m_gmt_mgr->GetSide();
  Identifier idwafer;
  idwafer = idHelper->wafer_id(dbmdet,m_gmt_mgr->GetLD(),m_gmt_mgr->Phi(),m_gmt_mgr->Eta());
  
  if(m_sqliteReader) {
    std::string key ="DBMDiamond_" + std::to_string(dbmdet) +"_"+ std::to_string(m_gmt_mgr->GetLD()) +"_"+ std::to_string(m_gmt_mgr->Phi()) +"_"+ std::to_string(m_gmt_mgr->Eta());
    SiDetectorElement* element = new SiDetectorElement(idwafer, m_design, (*m_mapFPV)[key], m_gmt_mgr->commonItems());
    // add the element to the manager
    m_DDmgr->addDetectorElement(element);
    m_DDmgr->addAlignableTransform(0, idwafer, (*m_mapAX)[key], (*m_mapFPV)[key]);
    return nullptr;
  }
  else {
    GeoIdentifierTag* diamondTag = new GeoIdentifierTag(400);


    double safety = 0.003*Gaudi::Units::mm;

    //diamond dimension
    double diamond_X = m_gmt_mgr->DBMDiamondX();
    double diamond_Y = m_gmt_mgr->DBMDiamondY();
    double diamond_Z = m_gmt_mgr->DBMDiamondZ();
    double air_gap = m_gmt_mgr->DBMAirGap();
    
    //chip, FEI4 dimension
    double chip_X = m_gmt_mgr->DBMFEI4X();
    double chip_Y = m_gmt_mgr->DBMFEI4Y();
    double chip_thick = m_gmt_mgr->DBMFEI4Z();

    //ceramic dimension
    double substrate_X = m_gmt_mgr->DBMCeramicX();
    double substrate_Y = m_gmt_mgr->DBMCeramicY();
    double substrate_Z = m_gmt_mgr->DBMCeramicZ();
    
    //distances from bottom of the ceramic
    //Hardcoded!
    double bot2Chip = 0.0*Gaudi::Units::mm;
    double bot2Diamond = 1.685*Gaudi::Units::mm;


    //---------------------------------------------
    // Get parameters for alignable Transform
  
    // Position of the corner closest to IP and beamline
    // Hardcoded, so if change then change in GeoPixelEnvelope and DBM_Det too
    double ZToIP = 887.002*Gaudi::Units::mm;
    double RToBeam = 46.678*Gaudi::Units::mm;

    // layer spacing
    double Zspacing = m_gmt_mgr->DBMSpacingZ();
    double Rspacing = m_gmt_mgr->DBMSpacingRadial();
    // gap between V-slide and first main plate
    double layer1Space = m_gmt_mgr->DBMSpace();

    // parameters for rotating the 3-layer unit
    double angle = m_gmt_mgr->DBMAngle(); // telescope tilting angle in degree
    double bracketZ = m_gmt_mgr->DBMBracketZ(); // total thickness of the bracket unit,
    double trapBackY = m_gmt_mgr->DBMTrapezBackY();
    double trapBackShortZ = m_gmt_mgr->DBMTrapezBackShortZ();
    double coolingSidePlateY = m_gmt_mgr->DBMCoolingSidePlateY();
    double brcktLockZ = m_gmt_mgr->DBMBrcktLockZ();
  
    //double lyRadius = sqrt(layerUnitY*layerUnitY/4 + layerUnitZ*layerUnitZ/4);
    //double lyAngle = atan(layerUnitY/layerUnitZ);
    // position of the 3-layer unit's corner closest to the IP and beamline, which is the rotation point
    double layerUnitPos_Y = (trapBackY/cos(angle) - coolingSidePlateY)*cos(angle);
    double layerUnitPos_Z = coolingSidePlateY*sin(angle) + trapBackShortZ + bracketZ - brcktLockZ; 
    //---------------------------------------------



    const GeoMaterial* air = m_mat_mgr->getMaterial("std::Air");
    //first try the Diamond
    const GeoMaterial* diamond = m_mat_mgr->getMaterial("pix::Diamond");
    if(diamond == nullptr)
    {
  	diamond = m_mat_mgr->getMaterial("std::Carbon");	
    }
  
    const GeoMaterial* chip_mat = m_mat_mgr->getMaterial("pix::ChipBase");
    
    double max_thick = diamond_Z + air_gap + chip_thick + substrate_Z;
    
    //create a outer box to contain all volumes
    const GeoBox* dbmModuleBox = new GeoBox(substrate_X/2.0 + safety, substrate_Y/2.0 + safety, max_thick/2.0 + safety);
    const GeoLogVol* dbmModuleLog = new GeoLogVol("dbmModuleLog", dbmModuleBox, air);
    GeoPhysVol* dbmModulePhys = new GeoPhysVol(dbmModuleLog);
    
    GeoTrf::Transform3D rm = GeoTrf::RotateZ3D(90.*Gaudi::Units::deg)*GeoTrf::RotateY3D(270.*Gaudi::Units::deg);

    //diamond
    const GeoBox* dbmDiamondBox = new GeoBox(diamond_Z/2.0, diamond_X/2.0, diamond_Y/2.0 );
    const GeoLogVol* dbmDiamondLog = new GeoLogVol("dbmDiamondLog", dbmDiamondBox, diamond);
    GeoFullPhysVol* dbmDiamondPhys = new GeoFullPhysVol(dbmDiamondLog);

    SiDetectorElement* element= new SiDetectorElement(idwafer, m_design, dbmDiamondPhys, m_gmt_mgr->commonItems());
    // add the element to the manager
    m_DDmgr->addDetectorElement(element);

    GeoTrf::Translation3D dbmDiamondPos(0, bot2Diamond+diamond_Y/2.0-substrate_Y/2.0, diamond_Z/2.0-max_thick/2.0);
    GeoTransform* xform = new GeoTransform(GeoTrf::Transform3D(dbmDiamondPos*rm));
     
    dbmModulePhys->add(diamondTag);
    dbmModulePhys->add(xform);
    dbmModulePhys->add(dbmDiamondPhys);
    
    //FEI4 chip
    const GeoBox* dbmFEI4Box = new GeoBox(chip_thick/2.0, chip_X/2.0, chip_Y/2.0 );
    const GeoLogVol* dbmFEI4Log = new GeoLogVol("dbmWallLogF4", dbmFEI4Box, chip_mat); //should this be "dbmdiamondLog"?
    GeoPhysVol* dbmFEI4Phys = new GeoPhysVol(dbmFEI4Log);
    
    GeoTrf::Translation3D dbmFEI4Pos(0, bot2Chip+chip_Y/2.0-substrate_Y/2.0, max_thick/2.0-substrate_Z-chip_thick/2.0);
    xform = new GeoTransform(GeoTrf::Transform3D(dbmFEI4Pos*rm));
     
    dbmModulePhys->add(xform);
    dbmModulePhys->add(dbmFEI4Phys);
    
    //ceramic support
    const GeoBox* dbmSubstBox = new GeoBox(substrate_X/2.0, substrate_Y/2.0, substrate_Z/2.0);
    const GeoMaterial* aluminiumNitride = m_mat_mgr->getMaterialForVolume("pix::DBMCeramic", dbmSubstBox->volume());
    const GeoLogVol* dbmSubstLog = new GeoLogVol("dbmWallLogCe", dbmSubstBox, aluminiumNitride);
    GeoPhysVol* dbmSubstPhys = new GeoPhysVol(dbmSubstLog);

    GeoTrf::Translate3D dbmSubstPos(0, 0, max_thick/2.0-substrate_Z/2.0);
    xform = new GeoTransform(dbmSubstPos);
     
    dbmModulePhys->add(xform);
    dbmModulePhys->add(dbmSubstPhys);
    
    //-----------------------------------------------------
    //Add to alignable transform
    //  DBM has only level 0 alignable transform.
    //  So, a transform w.r.t global position is created.
    //  This mean the alignable pos below should be 
    //  the global position of the sensor

    int layer = m_gmt_mgr->GetLD();
    double sensorPosInModuleCage_Z = layer1Space + layer*Zspacing - (substrate_Z + chip_thick + air_gap + diamond_Z/2.);
    double sensorPosInModuleCage_Y = Rspacing + bot2Diamond + diamond_Y/2.;
    double globPosZ = ZToIP + layerUnitPos_Z + (sensorPosInModuleCage_Z * cos(angle) - sensorPosInModuleCage_Y * sin(angle));
    double globPosY = RToBeam + layerUnitPos_Y + (sensorPosInModuleCage_Z * sin(angle) + sensorPosInModuleCage_Y * cos(angle));

    GeoTrf::RotateX3D rmX10(-10.*Gaudi::Units::deg);
    GeoTrf::Translation3D alignTransformPos(0, globPosY, globPosZ);
    GeoAlignableTransform *xformAlign = new GeoAlignableTransform(GeoTrf::Transform3D(alignTransformPos*rmX10));
    m_DDmgr->addAlignableTransform(0, idwafer, xformAlign, dbmDiamondPhys);
    //-----------------------------------------------------

    return dbmModulePhys;
  }
}
