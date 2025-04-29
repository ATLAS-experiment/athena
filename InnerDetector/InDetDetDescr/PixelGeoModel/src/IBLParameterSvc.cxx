/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "IBLParameterSvc.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/PathResolver.h"

//Includes related to determining presence of IBL
#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "RDBAccessSvc/IRDBRecord.h"
#include "RDBAccessSvc/IRDBRecordset.h"
#include "GeoModelInterfaces/IGeoDbTagSvc.h"
#include "GeoModelUtilities/DecodeVersionKey.h"

 
/**
 ** Constructor(s)
 **/
IBLParameterSvc::IBLParameterSvc(const std::string& name,ISvcLocator* svc)
  : base_class(name,svc),
    m_IBLpresent{},
    m_DBMpresent{},
    m_LayerColumnsPerFE{},
    m_LayerRowsPerFE{},
    m_LayerFEsPerHalfModule_planar{},
    m_LayerFEsPerHalfModule_3d{},
    m_layout{},
    m_geoDbTagSvc("GeoDbTagSvc",name),
    m_disablePixMapCondDB(false),
    m_disableSpecialPixels(false),
    m_disableAlignable(false),
    m_disableAllClusterSplitting(false),
    m_disableDCS(true) {
	declareProperty("GeoDbTagSvc",m_geoDbTagSvc);
 	declareProperty("DisablePixMapCondDB",m_disablePixMapCondDB);
 	declareProperty("DisableSpecialPixels",m_disableSpecialPixels);
 	declareProperty("DisableAlignable",m_disableAlignable);
 	declareProperty("DisableAllClusterSplitting",m_disableAllClusterSplitting);
 	declareProperty("DisableDCS",m_disableDCS);
}
 
IBLParameterSvc::~IBLParameterSvc()
{
}

/**
 ** Initialize Service
 **/
StatusCode IBLParameterSvc::initialize()
{
  //MsgStream log(msgSvc(), name());
 
  StatusCode result = AthService::initialize();
  if (result.isFailure())
  {
   ATH_MSG_FATAL ( "Unable to initialize the service!" );
   return result;
  }
  // read Charge Collection Probability Maps
  //
  // get the PixelDigitization random stream
  //
  
  if (!setIblParameters().isSuccess()) return StatusCode::FAILURE; 

  ATH_MSG_DEBUG ( "initialized service!" );
  return result;
 
}  

//Determine if IBL is present and set appropriate parameters
StatusCode IBLParameterSvc::setIblParameters() {
  ATH_CHECK(m_geoDbTagSvc.retrieve());

  
  std::string versionTag{}, versionNode{};
  if(!m_geoDbTagSvc->getSqliteReader()) {
    DecodeVersionKey detectorKey{m_geoDbTagSvc.get(),"Pixel"};
    versionTag = detectorKey.tag();
    versionNode = detectorKey.node();
  }

  SmartIF<IRDBAccessSvc> rdbAccess{Gaudi::svcLocator()->service(m_geoDbTagSvc->getParamSvcName())};
  if (!rdbAccess) {
    ATH_MSG_FATAL("Could not locate RDBAccessSvc");
     return StatusCode::FAILURE; 
  } 
  IRDBRecordset_ptr switchSet = rdbAccess->getRecordsetPtr("PixelSwitches", versionTag, versionNode);
  const IRDBRecord    *switchTable   = (*switchSet)[0];
  std::string versionName("");
  if (!switchTable->isFieldNull("VERSIONNAME")) versionName=switchTable->getString("VERSIONNAME");
  m_DBMpresent=false;
  if (!switchTable->isFieldNull("BUILDDBM")) m_DBMpresent=switchTable->getInt("BUILDDBM");
  if (versionName=="IBL") {
	m_IBLpresent = true;
	ATH_MSG_INFO("IBL geometry");
  }
  else {
	m_IBLpresent = false;
	ATH_MSG_VERBOSE("Default geometry");
  }
  m_LayerFEsPerHalfModule_3d = 0;
  m_LayerFEsPerHalfModule.clear();
  if (m_IBLpresent) {
	IRDBRecordset_ptr PixelReadout = rdbAccess->getRecordsetPtr("PixelReadout", versionTag, versionNode);
	IRDBRecordset_ptr PixelStave = rdbAccess->getRecordsetPtr("PixelStave", versionTag, versionNode);
	const IRDBRecord *IBLreadout   = (*PixelReadout)[1];
	if (!IBLreadout->isFieldNull("COLSPERCHIP")) m_LayerColumnsPerFE=IBLreadout->getInt("COLSPERCHIP");
	if (!IBLreadout->isFieldNull("ROWSPERCHIP")) m_LayerRowsPerFE=IBLreadout->getInt("ROWSPERCHIP");
	if (!IBLreadout->isFieldNull("NCHIPSETA")) m_LayerFEsPerHalfModule_planar=IBLreadout->getInt("NCHIPSETA");
	if ((*PixelReadout).size()>2) {
		const IRDBRecord *IBL3Dreadout   = (*PixelReadout)[2];
		if (!IBL3Dreadout->isFieldNull("NCHIPSETA")) m_LayerFEsPerHalfModule_3d=IBL3Dreadout->getInt("NCHIPSETA");
	}
	const IRDBRecord *IBLstave   = (*PixelStave)[1];
	if (!IBLstave->isFieldNull("LAYOUT")) m_layout=IBLstave->getInt("LAYOUT");
	if (m_layout==4) for (int i = 0; i < 16; i++) m_LayerFEsPerHalfModule.push_back(m_LayerFEsPerHalfModule_planar);
	if (m_layout==5) {
		for (int i =0; i < 4; i++) m_LayerFEsPerHalfModule.push_back(m_LayerFEsPerHalfModule_3d);
		for (int i =0; i < 12; i++) m_LayerFEsPerHalfModule.push_back(m_LayerFEsPerHalfModule_planar);
		for (int i =0; i < 4; i++) m_LayerFEsPerHalfModule.push_back(m_LayerFEsPerHalfModule_3d);
	}
  }
  return StatusCode::SUCCESS;
} 

StatusCode IBLParameterSvc::finalize()
{
        return StatusCode::SUCCESS;
}

