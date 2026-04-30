/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloTowerContainerCnv.h"

// LArDetDescr includes
#include "CaloDetDescr/CaloDetDescrManager.h"

// Gaudi
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/IToolSvc.h"
#include "GaudiKernel/ThreadLocalContext.h"

// Athena
#include "CaloUtils/CaloTowerBuilderToolBase.h"
#include "CaloUtils/CaloTowerBuilderTool.h"


CaloTowerContainerCnv::CaloTowerContainerCnv(ISvcLocator* svcloc)
    :
    // Base class constructor
    CaloTowerContainerCnvBase(svcloc)
{}

CaloTowerContainer* CaloTowerContainerCnv::createTransient(const Token* token) {
    const EventContext& ctx = Gaudi::Hive::currentContext();
    MsgStream log(msgSvc(), "CaloTowerContainerCnv::createTransient" );
    CaloTowerContainer* Cont = 0;

    if (compareClassGuid(token, p0_guid)) {
     if (log.level() <= MSG::DEBUG) log << MSG::DEBUG << "Read version p0 of CaloTowerContainer. token=" 
	 << token->toString() << endmsg;
     Cont=poolReadObject<CaloTowerContainer>(token);
    }
    else if(compareClassGuid(token, p1_guid)) {
      if (log.level() <= MSG::DEBUG) log << MSG::DEBUG << "Read version p1 of CaloTowerContainer. token=" 
	  << token->toString() << endmsg;
      CaloTowerContainerPERS* pers=poolReadObject<CaloTowerContainer_p1>(token);
      Cont=new CaloTowerContainer();
      m_converter.persToTrans(pers,Cont,log);
      delete pers;
    }
    if (!Cont) {
      log << MSG::FATAL << "Unable to get object from pool" << endmsg;
      return Cont;
    }
    

    Cont->init();

    // rebuild the CaloTowers in the container.

    std::vector<CaloCell_ID::SUBCALO> v; 

    if(Cont->getCalos(v)==0){
 	log<<MSG::WARNING<< " No SUBCALO in CaloTowerContainer"<<endmsg;
        return Cont;
    }

    std::vector<CaloCell_ID::SUBCALO> EmHec;

    std::vector<CaloCell_ID::SUBCALO>::const_iterator it = v.begin();
    std::vector<CaloCell_ID::SUBCALO>::const_iterator it_e = v.end();
    for(; it!=it_e;++it){
	CaloCell_ID::SUBCALO sub = *it;

	if(sub == CaloCell_ID::LAREM) {
	  EmHec.push_back(sub); 
	} 

	if(sub == CaloCell_ID::LARHEC) {
	  EmHec.push_back(sub); 
	} 

	if(sub == CaloCell_ID::LARFCAL) {
	  if(! m_fcalTowerBldr){
	    m_fcalTowerBldr= getTool("LArFCalTowerBuilderTool",
		"LArTowerFCal");
	    if(!m_fcalTowerBldr){
	     log<<MSG::ERROR<< " Failed to create LArFCalTowerBuilder " <<endmsg;
             return 0;
	    }
	  }
         if (log.level() <= MSG::DEBUG)  log<<MSG::DEBUG<<" Towers rebuild for FCAL "<<endmsg; 
          StatusCode scfcal = m_fcalTowerBldr->execute(ctx, Cont); 
	  if (scfcal.isFailure()) {
	    log<<MSG::ERROR<<" Towers rebuild for FCAL failed "<<endmsg; 
	  }
	} 

	if(sub == CaloCell_ID::TILE) {
	  if(! m_tileTowerBldr){
	    m_tileTowerBldr= getTool("TileTowerBuilderTool",
		"TileTower");
	    if(!m_tileTowerBldr){
	     log<<MSG::ERROR<< " Failed to create TileTowerBuilder " <<endmsg;
             return 0;
	    }
	  }
          if (log.level() <= MSG::DEBUG) log<<MSG::DEBUG<<" Towers rebuild for Tile "<<endmsg; 
	  StatusCode sctile=m_tileTowerBldr->execute(ctx, Cont); 
	  if (sctile.isFailure()) {
	    log<<MSG::ERROR<<" Towers rebuild for Tile failed "<<endmsg; 
	  }
	} 

    }

    if(EmHec.size()>0){ 
	  if(! m_emHecTowerBldr){
	    CaloTowerBuilderToolBase * bldr = getTool("LArTowerBuilderTool",
		"LArTowerEMHEC");
	    m_emHecTowerBldr=dynamic_cast<CaloTowerBuilderTool*>(bldr); 
	    if(!m_emHecTowerBldr){
	     log<<MSG::ERROR<< " Failed to create LArTowerBuilder for EM&HEC" <<endmsg;
             return 0;
	    }
	  }
          if (log.level() <= MSG::DEBUG) log<<MSG::DEBUG<<" Towers rebuild for EM and/or HEC "<<endmsg; 
	  m_emHecTowerBldr->setCalos(EmHec); 
	  StatusCode scemHec=m_emHecTowerBldr->execute(ctx, Cont);
	  if (scemHec.isFailure()) {
	    log<<MSG::ERROR<<" Towers rebuild for EM and/or HEC failed "<<endmsg; 
	  }

    }

    return Cont; 
}

CaloTowerContainerPERS* CaloTowerContainerCnv::createPersistent(CaloTowerContainer* trans) {
    MsgStream log(msgSvc(), "CaloTowerContainerCnv::createPersistent");
    if (log.level() <= MSG::DEBUG) log << MSG::DEBUG << "Writing CaloTowerContainer_p1" << endmsg;
    CaloTowerContainerPERS* pers=new CaloTowerContainerPERS();
    m_converter.transToPers(trans,pers,log); 
    return pers;
}


CaloTowerBuilderToolBase* CaloTowerContainerCnv::getTool(
const std::string& type, const std::string& nm)
{
  SmartIF<IToolSvc> myToolSvc{Gaudi::svcLocator()->service("ToolSvc")};
  if(!myToolSvc.isValid()) {
    ATH_MSG_ERROR("Cannot locate ToolSvc");
    return 0;
  }

  ////////////////////
  // Allocate Tools //
  ////////////////////

  IAlgTool* algToolPtr{nullptr};
  StatusCode sc = myToolSvc->retrieveTool(type,nm,algToolPtr);
  // tool not found
  if ( sc.isFailure() )
   {
     ATH_MSG_INFO("Cannot find tool named <"
		  << type << "/" << nm 
		  << ">");
          return 0; 
    }
  return   dynamic_cast<CaloTowerBuilderToolBase*>(algToolPtr);

}
