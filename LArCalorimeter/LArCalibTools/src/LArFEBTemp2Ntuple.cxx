/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "LArCalibTools/LArFEBTemp2Ntuple.h"
#include "LArElecCalib/ILArFEBTempTool.h"
#include "LArIdentifier/LArOnlineID.h"
#include "StoreGate/StoreGateSvc.h"
#include "GaudiKernel/IToolSvc.h"

LArFEBTemp2Ntuple::LArFEBTemp2Ntuple(const std::string& name, ISvcLocator* pSvcLocator)
  : LArCond2NtupleBase(name, pSvcLocator)
{
  m_ntTitle = "FEB Temperatures";
  m_ntpath = "/NTUPLES/FILE1/FEBTEMP";
}

LArFEBTemp2Ntuple::~LArFEBTemp2Ntuple() 
= default;

StatusCode LArFEBTemp2Ntuple::stop() {
  StatusCode sc;
  NTuple::Item<float> temp1;
  NTuple::Item<float> temp2;
 
   sc=m_nt->addItem("temp1",temp1,-1000.,5000.);
   if (sc!=StatusCode::SUCCESS)
     {ATH_MSG_ERROR( "addItem 'temp1' failed" );
	return StatusCode::FAILURE;
     }
   
   sc=m_nt->addItem("temp2",temp2,-1000.,5000.);
   if (sc!=StatusCode::SUCCESS)
     {ATH_MSG_ERROR( "addItem 'temp2' failed" );
	return StatusCode::FAILURE;
     }
   
   SmartIF<IToolSvc> toolSvc{Gaudi::svcLocator()->service("ToolSvc")};
   if(!toolSvc.isValid()) {
     ATH_MSG_ERROR( "Unable to retrieve IToolSvc");
     return StatusCode::FAILURE;
   }   

   ILArFEBTempTool *larFEBTempTool;
   sc = toolSvc->retrieveTool("LArFEBTempTool", larFEBTempTool);
   if (sc!=StatusCode::SUCCESS) {
      ATH_MSG_ERROR( "Unable to retrieve LArFEBTempTool from ToolSvc" );
      return StatusCode::FAILURE;
   } 

   for (const HWIdentifier hwid: m_onlineId->channel_range()) {
	FEBTemp tv = larFEBTempTool->getFebTemp(hwid);
        ATH_MSG_DEBUG( hwid << " " << tv.size() );
	
	if( !tv.empty() )
	  {	     	 
	     FEBTemp::const_iterator itb = tv.begin();
	     FEBTemp::const_iterator ite = tv.end();
	     
	     for(;itb!=ite;++itb)
	       {		  
		  temp1 = (*itb).first;
		  temp2 = (*itb).second;
		  
		  fillFromIdentifier(hwid);
	     
		  sc=ntupleSvc()->writeRecord(m_nt);
	      
		  if (sc!=StatusCode::SUCCESS) 
		    {
		       ATH_MSG_ERROR( "writeRecord failed" );
		       return StatusCode::FAILURE;
		    }
	       }	     
	  }	
     }
   
   ATH_MSG_INFO( "LArFEBTemp2Ntuple has finished." );
   
   return StatusCode::SUCCESS;
   
}// end finalize-method.
   
