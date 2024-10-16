/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloTopoTowerContainerCnv.h"

// LArDetDescr includes
#include "CaloDetDescr/CaloDetDescrManager.h"

// Gaudi
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/IToolSvc.h"
#include "GaudiKernel/ThreadLocalContext.h"

// Athena
#include "CaloUtils/CaloTopoTowerBuilderToolBase.h"


CaloTopoTowerContainerCnv::CaloTopoTowerContainerCnv(ISvcLocator* svcloc)
    :
    // Base class constructor
    CaloTopoTowerContainerCnvBase(svcloc)
{}

CaloTopoTowerContainer* CaloTopoTowerContainerCnv::createTransient() {
    MsgStream log(msgSvc(), "CaloTopoTowerContainerCnv::createTransient" );
    CaloTopoTowerContainer* Cont = 0;
    if (log.level() <= MSG::DEBUG) log << MSG::DEBUG << "Starting CaloTopoTowerContainerCnv::PoolToDataObject" << endmsg;
    if (compareClassGuid(p0_guid)) {
     if (log.level() <= MSG::DEBUG) log << MSG::DEBUG << "Read version p0 of CaloTopoTowerContainer. GUID=" 
	 << m_classID.toString() << endmsg;
     Cont=poolReadObject<CaloTopoTowerContainer>();
    }
    else if(compareClassGuid(p1_guid)) {
      if (log.level() <= MSG::DEBUG) log << MSG::DEBUG << "Read version p1 of CaloTopoTowerContainer. GUID=" 
	  << m_classID.toString() << endmsg;
      CaloTopoTowerContainerPERS* pers=poolReadObject<CaloTopoTowerContainer_p1>();
      Cont=new CaloTopoTowerContainer();
      m_converter.persToTrans(pers,Cont,log);
      delete pers;
    }
    if (!Cont) {
      log << MSG::FATAL << "Unable to get object from pool" << endmsg;
      return Cont;
    }
    
    Cont->init();

    // rebuild the CaloTopoTowers in the container.
    const EventContext& ctx = Gaudi::Hive::currentContext();

    m_TopoTowerBldr= getTool("CaloTopoTowerBuilderTool","TopoTowerTwrBldr");
    if(!m_TopoTowerBldr){
      log<<MSG::ERROR<< " Failed to create CaloTopoTowerContainer " <<endmsg;
      return 0;
    }
    if (log.level() <= MSG::DEBUG) log << MSG::DEBUG << "creating CaloTopoTowerContainerCnv::PoolToDataObject" << endmsg; 
    StatusCode scfcal = m_TopoTowerBldr->execute(ctx, Cont);
    if (log.level() <= MSG::DEBUG) log<<MSG::DEBUG<<" TopoTowers rebuild m_TopoTowerBldr->execute(Cont); Successful "<<endmsg; 
    if (scfcal.isFailure()) {
      log<<MSG::ERROR<<" TopoTowers rebuild failed "<<endmsg; 
    }
    if (log.level() <= MSG::DEBUG) log<<MSG::DEBUG<<" TopoTowers rebuild worked "<<endmsg;

    return Cont; 
}

CaloTopoTowerContainerPERS* CaloTopoTowerContainerCnv::createPersistent(CaloTopoTowerContainer* trans) {
    MsgStream log(msgSvc(), "CaloTopoTowerContainerCnv::createPersistent");
    if (log.level() <= MSG::DEBUG) log << MSG::DEBUG << "Writing CaloTopoTowerContainer_p1" << endmsg;
    CaloTopoTowerContainerPERS* pers=new CaloTopoTowerContainerPERS();
    m_converter.transToPers(trans,pers,log); 
    return pers;
}


CaloTopoTowerBuilderToolBase* CaloTopoTowerContainerCnv::getTool(
const std::string& type, const std::string& nm)
{
  SmartIF<IToolSvc> myToolSvc{Gaudi::svcLocator()->service("ToolSvc")};

  ////////////////////
  // Allocate Tools //
  ////////////////////

  IAlgTool* algToolPtr;
  StatusCode sc = myToolSvc->retrieveTool(type,nm,algToolPtr);
  // tool not found
  if ( sc.isFailure() )
   {
     ATH_MSG_INFO("Cannot find tool named <"
		  << type << "/" << nm 
		  << ">");
          return 0; 
    }
  return   dynamic_cast<CaloTopoTowerBuilderToolBase*>(algToolPtr);

}
