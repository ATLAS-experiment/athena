/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "ALFA_Geometry/ALFA_GeometryReader.h"
#include "ALFA_Geometry/ALFA_RDBAccess.h"


ALFA_RDBAccess::ALFA_RDBAccess()
{
}

ALFA_RDBAccess::~ALFA_RDBAccess()
{
}

bool ALFA_RDBAccess::ReadGeometry(const eRPotName eRPName, eFiberType eFType,
                                  const std::string& element,
                                  const std::string& tag,
                                  const std::string& node)
{
	bool bRes = false;
	
	MsgStream LogStream(Athena::getMessageSvc(), "ALFA_RDBAaccess::ReadGeometry");
	
	SmartIF<IRDBAccessSvc> iAccessSvc(Gaudi::svcLocator()->service("RDBAccessSvc"));

	if (!iAccessSvc)
	{
		LogStream << MSG::FATAL << " Could not initialize RDBAccessSvc! ((RP no."<<eRPName<<", Fiber type "<<eFType<<")"<< endmsg;

		throw GaudiException(" Could not initalize RDBAccessSvc ", "ALFA_RDBAccess::ReadGeometry", StatusCode::FAILURE);
	}

	

	bRes=ReadFiberGeometry(iAccessSvc, element, tag, node);

	return bRes;
}

bool ALFA_RDBAccess::ReadFiberGeometry(IRDBAccessSvc* iAccessSvc, const std::string& element, const std::string& tag, const std::string& node)
{
	bool bRes = false;
	
	MsgStream LogStream(Athena::getMessageSvc(), "ALFA_RDBAaccess::ReadFiberGeometry");

	std::list<FIBERDATA> listFData;
	listFData.clear();

	m_fiberGeometry = iAccessSvc->getRecordsetPtr(element, tag, node);
	
	if (!m_fiberGeometry->size())
	{
		bRes = false;
		LogStream << MSG::FATAL << " ERROR: Unable to retrieve " << element <<" data from database" << endmsg;
	}
	else
	{
		FIBERDATA FiberData;
		IRDBRecordset::const_iterator AccessSvc_iter;
		
		for(AccessSvc_iter = m_fiberGeometry->begin(); AccessSvc_iter != m_fiberGeometry->end(); ++AccessSvc_iter)
		{
			FiberData.nPotID   = (*AccessSvc_iter)->getInt("POTID");
			FiberData.nLayerID = (*AccessSvc_iter)->getInt("COORDID");
			FiberData.nFiberID = (*AccessSvc_iter)->getInt("FIBREID");
			FiberData.fSlope   = (*AccessSvc_iter)->getDouble("SLOPE");
			FiberData.fOffset  = (*AccessSvc_iter)->getDouble("OFFSET");
			FiberData.fZPos    = (*AccessSvc_iter)->getDouble("ZL");
			
			listFData.push_back(FiberData);
		}
		
		if (!listFData.empty())
		{
			m_ListFiberData=listFData;
			bRes = true;
		}
	}

	listFData.clear();
	
	return bRes;
}






