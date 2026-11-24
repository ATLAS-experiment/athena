/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "BCM_Builder.h"
#include "BCM_GeometryManager.h"
#include "BCM_Module.h"

#include "GeoModelKernel/GeoFullPhysVol.h"
#include "GeoModelKernel/GeoTransform.h"
#include "GeoModelKernel/GeoNameTag.h"
#include "GeoModelKernel/GeoIdentifierTag.h"

#include "GeoModelInterfaces/StoredMaterialManager.h"
#include "GeoModelUtilities/DecodeVersionKey.h"

#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "RDBAccessSvc/IRDBRecordset.h"
#include "RDBAccessSvc/IRDBRecord.h"

#include "GaudiKernel/SystemOfUnits.h"

//================ Initialisation =================================================

StatusCode InDetDD::BCM_Builder::initialize()
{
  
  StatusCode sc = AthAlgTool::initialize();
  if (sc.isFailure()) return sc;

  ATH_MSG_INFO("BCMBuilder initialize() successful in " << name());
  return StatusCode::SUCCESS;
}

//================ Finalisation =================================================

StatusCode InDetDD::BCM_Builder::finalize()
{
  StatusCode sc = AthAlgTool::finalize();
  return sc;
}

//================ Actual Work =================================================

StatusCode InDetDD::BCM_Builder::build(GeoVPhysVol* pv)
{
  if(!m_bcmon) {
    ATH_MSG_INFO("BCM disabled.");
    return StatusCode::SUCCESS;
  }

  // Unfortunately add is not part of the abstract interface of GeoVPhysVol so we have to dynamic cast
  // to the concrete class.
  GeoFullPhysVol* Phys = dynamic_cast<GeoFullPhysVol *>(pv);
  if (!Phys) {
    ATH_MSG_ERROR("Parent volume is not a GeoFullPhysVol as expected.");  
    return StatusCode::FAILURE;
  }

  StoredMaterialManager * materialManager{nullptr};
  ATH_CHECK(detStore()->retrieve(materialManager, std::string("MATERIALS")));

  //create geometry manager
  std::unique_ptr<BCM_GeometryManager> manager = std::make_unique<BCM_GeometryManager>();

  StatusCode sc;

  if(m_BDparameters)
    {
      DecodeVersionKey versionKey("InnerDetector");
      
      // Issue error if AUTO.
      if (versionKey.tag() == "AUTO") {
	ATH_MSG_ERROR("AUTO Atlas version. Please select a version.");
	return StatusCode::FAILURE;
      }
      
      ATH_MSG_INFO("Building BCM with Version Tag: " << versionKey.tag() << " at Node: " << versionKey.node());
      SmartIF<IRDBAccessSvc> accessSvc{Gaudi::svcLocator()->service("RDBAccessSvc")};
      ATH_CHECK(accessSvc.isValid());
      
      // Print the BCM version tag:
      std::string BCMVersionTag;
      BCMVersionTag = accessSvc->getChildTag("BCM", versionKey.tag(), versionKey.node());
      ATH_MSG_INFO("BCM Version: " << BCMVersionTag);
      
      // Check if version is empty. If so, then the BCM cannot be built. This may or may not be intentional. We just issue an INFO message. 
      if (BCMVersionTag.empty()) {
	ATH_MSG_INFO("No BCM Version. BCM will not be built.");
	return StatusCode::SUCCESS;
      }
      
      IRDBRecordset_ptr DBmodul = accessSvc->getRecordsetPtr("BCMModule", versionKey.tag(), versionKey.node());
      ATH_MSG_DEBUG(" --> Number of records fetched = " << DBmodul->size());
      
      unsigned int ind;
      long moduleNo;

      for(ind = 0; ind < DBmodul->size(); ind++) {
	const IRDBRecord* rec = (*DBmodul)[ind];
	moduleNo = rec->getLong("MODULE_ID");

	//check if this module is suposed to be builded
	unsigned int mask = (1 << moduleNo);
	if((mask & m_moduleon) != mask) {
	  //this module is not sopoused to be built
	  continue;
	}

	std::vector<double> module_property;
	module_property.push_back(rec->getFloat("TRANS_X"));
	module_property.push_back(rec->getFloat("TRANS_Y"));
	module_property.push_back(rec->getFloat("TRANS_Z"));
	module_property.push_back(rec->getFloat("ROT_X"));
	module_property.push_back(rec->getFloat("ROT_Y"));
	module_property.push_back(rec->getFloat("ROT_Z"));
	module_property.push_back(rec->getFloat("DIMESION_Z"));
	module_property.push_back(rec->getFloat("DIAM_TRANS_Y"));
	module_property.push_back(rec->getFloat("DIAM_TRANS_Z"));
	module_property.push_back(rec->getFloat("DIAM_DIAM_X"));
	module_property.push_back(rec->getFloat("DIAM_DAIM_Y"));
	module_property.push_back(rec->getFloat("DIAM_DIAM_Z"));
	module_property.push_back(rec->getFloat("OFF_H"));
	module_property.push_back(rec->getFloat("OFF_I"));
	module_property.push_back(rec->getFloat("OFF_J"));

	//set the BCM_GeometryManeger
	manager->ModuleOn(moduleNo);
	manager->Module(moduleNo)->Set(moduleNo, &module_property);
      }
      ATH_MSG_DEBUG(" --> Number succesfully read from DB");
    } 
  else
    {
      //parameters via jobOptions
      int moduleNo;
      const std::vector<double>* module_property = nullptr;
      for(moduleNo = 0; moduleNo < 8; moduleNo++)
	{
	  unsigned int mask = (1 << moduleNo);
	  if((mask & m_moduleon) != mask)
	    {
	      //this module is not sopoused to be built
	      continue;
	    }
	  if(moduleNo == 0)      module_property = &m_module0.value();
	  else if(moduleNo == 1) module_property = &m_moduleI.value();
	  else if(moduleNo == 2) module_property = &m_moduleII.value();
	  else if(moduleNo == 3) module_property = &m_moduleIII.value();
	  else if(moduleNo == 4) module_property = &m_moduleIV.value();
	  else if(moduleNo == 5) module_property = &m_moduleV.value();
	  else if(moduleNo == 6) module_property = &m_moduleVI.value();
	  else if(moduleNo == 7) module_property = &m_moduleVII.value();
	  	  
	  //set the BCM_GeometryManeger
	  manager->ModuleOn(moduleNo);
	  if (module_property){
	    manager->Module(moduleNo)->Set(moduleNo, module_property);
	  } else {
	    ATH_MSG_ERROR("module_property pointer is null in BCM_Builder.cxx");
	  }
	}
    }
  
  //we are now adding eight BCM modules
  BCM_Module bcm;
  
  for(int i=0; i<8; i++)
    {
      //see if this module is to be built
      if(!(manager->IsModuleOn(i)))
		continue;
      
      //the module should be build
      BCM_ModuleParameters* parameters = manager->Module(i);
      
      //setting transformation
      GeoTrf::Translation3D pos(parameters->Position_X(), parameters->Position_Y(), parameters->Position_Z());
      GeoTrf::Transform3D rm = GeoTrf::RotateZ3D(parameters->Rotation_Z()*Gaudi::Units::deg)
	* GeoTrf::RotateY3D(parameters->Rotation_Y()*Gaudi::Units::deg)
	* GeoTrf::RotateX3D(parameters->Rotation_X()*Gaudi::Units::deg)
	* GeoTrf::RotateZ3D(-90.*Gaudi::Units::deg)
	* GeoTrf::RotateY3D(-90.*Gaudi::Units::deg);
      GeoIntrusivePtr<GeoTransform> xform{new GeoTransform(GeoTrf::Transform3D(pos*rm))};
      ATH_MSG_DEBUG(" --> Module " << i << " build!");

      //building module
      int k=i+951;
      
      GeoIntrusivePtr<GeoNameTag> tag{new GeoNameTag("BCM Module")}; 
      if (materialManager){
        GeoVPhysVol* bcmModPhys = bcm.Build(materialManager, parameters, (msgLvl(MSG::INFO) ? &msg(MSG::INFO) : nullptr));
        Phys->add(tag);
        Phys->add(new GeoIdentifierTag(k));
        Phys->add(xform);
        Phys->add(bcmModPhys);	
      }
    }
  
  ATH_MSG_DEBUG("Registering BCM_GeometryManager."); 
  ATH_CHECK(detStore()->record(std::move(manager), "BCMParameters"));

  return StatusCode::SUCCESS;
}

//============================================================================================
