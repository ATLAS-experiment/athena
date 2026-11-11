/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "ForwardRegionGeoModelTool.h"
#include "ForwardRegionGeoModelFactory.h"
#include "ForwardRegionGeoModelManager.h"
#include "GeoModelUtilities/GeoModelExperiment.h"
#include "GaudiKernel/IService.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "StoreGate/StoreGateSvc.h"

/**
 ** Constructor(s)
 **/
ForwardRegionGeoModelTool::ForwardRegionGeoModelTool( const std::string& type, const std::string& name, const IInterface* parent )
: GeoModelTool( type, name, parent )
{
}

/**
 ** Destructor
 **/
ForwardRegionGeoModelTool::~ForwardRegionGeoModelTool()
{
        // This will need to be modified once we register the  DetectorNode in
	// the Transient Detector Store
	if ( 0 != m_detector ) {
		delete m_detector;
		m_detector = 0;
	}
}

/**
 ** Copy the property values into the configuration structure
 **/
void ForwardRegionGeoModelTool::fillConfig()
{
    m_Config.clear();
    m_Config.TCL4JawDistB1I = m_tcl4JawDistB1I;
    m_Config.TCL4JawDistB2I = m_tcl4JawDistB2I;
    m_Config.TCL5JawDistB1I = m_tcl5JawDistB1I;
    m_Config.TCL5JawDistB2I = m_tcl5JawDistB2I;
    m_Config.TCL6JawDistB1I = m_tcl6JawDistB1I;
    m_Config.TCL6JawDistB2I = m_tcl6JawDistB2I;
    m_Config.TCL4JawDistB1O = m_tcl4JawDistB1O;
    m_Config.TCL4JawDistB2O = m_tcl4JawDistB2O;
    m_Config.TCL5JawDistB1O = m_tcl5JawDistB1O;
    m_Config.TCL5JawDistB2O = m_tcl5JawDistB2O;
    m_Config.TCL6JawDistB1O = m_tcl6JawDistB1O;
    m_Config.TCL6JawDistB2O = m_tcl6JawDistB2O;
    m_Config.vp1Compatibility = m_vp1Compatibility;
    m_Config.buildTCL4 = m_buildTCL4;
    m_Config.buildTCL6 = m_buildTCL6;
    m_Config.ALFAInNewPosition = m_alfaInNewPosition;
    m_Config.newPosB7L1 = m_newPosB7L1;
    m_Config.newPosB7R1 = m_newPosB7R1;
    m_Config.posAFPL1 = m_posAFPL1;
    m_Config.posAFPR1 = m_posAFPR1;
    m_Config.posAFPL2 = m_posAFPL2;
    m_Config.posAFPR2 = m_posAFPR2;
    m_Config.posZDC1 = m_posZDC1;
    m_Config.posZDC2 = m_posZDC2;
}

/**
 ** Create the Detector Node corresponding to this tool
 **/
StatusCode
ForwardRegionGeoModelTool::create()
{ 
  MsgStream log(msgSvc(), name()); 
  // 
  // Locate the top level experiment node 
  // 
  GeoModelExperiment* theExpt = nullptr;
  StatusCode sc = detStore()->retrieve( theExpt, "ATLAS" );
  if (StatusCode::SUCCESS != sc) {
    log << MSG::ERROR 
	<< "Could not find GeoModelExperiment ATLAS" 
	<< endmsg; 
    return (StatusCode::FAILURE); 
  } 

  fillConfig();

  ForwardRegionGeoModelFactory theFactory(detStore().operator->(), &m_Config);
  if ( 0 == m_detector ) {
    // Create the DetectorNode instance
    try {   
      //
      // This strange way of casting is to avoid an
      // utterly brain damaged compiler warning.
      //
      GeoPhysVol *world=&*theExpt->getPhysVol();
      theFactory.create(world);
    } catch (const std::bad_alloc&) {
      log << MSG::FATAL << "Could not create new DetectorNode!" << endmsg;
      return StatusCode::FAILURE; 
    }
    // Register the DetectorNode instance with the Transient Detector Store
    theExpt->addManager(theFactory.getDetectorManager());
    sc = detStore()->record(theFactory.getDetectorManager(),theFactory.getDetectorManager()->getName());
    if (StatusCode::SUCCESS != sc) {
      log << MSG::ERROR
          << "Could not register DetectorNode"
          << endmsg;
      return (StatusCode::FAILURE);
    }
    return StatusCode::SUCCESS;
  }
  return StatusCode::FAILURE;
}
