/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/*
* Author: Ondra Kovanda, ondrej.kovanda at cern.ch
* Date: 03/2025
* Description: The BS converter for ITk pixels
*/

#include "ITkPixelRawContBytestreamCnv.h"
#include "ByteStreamCnvSvcBase/ByteStreamAddress.h"
#include "ITkPixelRDO_Container.h"
#include "AthenaBaseComps/AthCheckMacros.h"
#include "ByteStreamCnvSvcBase/ByteStreamCnvSvcBase.h"
#include "AthenaKernel/StorableConversions.h"

ITkPixelRawContByteStreamCnv::ITkPixelRawContByteStreamCnv(ISvcLocator* svcloc) : 
    AthConstConverter(ByteStreamAddress::storageType(), 1328667962, svcloc, "ITkPixelRawContByteStreamCnv"),
    m_ByteStreamEventAccess("ByteStreamCnvSvc", "ITkPixelRawContByteStreamCnv"),
    m_cnvTool("ITkPixelCnvTool")
{}

StatusCode ITkPixelRawContByteStreamCnv::initialize(){

    ATH_CHECK(AthConstConverter::initialize());

    ATH_CHECK(m_ByteStreamEventAccess.retrieve());

    ATH_CHECK(m_cnvTool.retrieve());

    ATH_MSG_INFO("ITkPixelRawContByteStreamCnv initialized!!!");

    return StatusCode::SUCCESS;
}

/**
* @brief This function is called for the RDO -> BS conversion
* and orchestrates the necessary steps, mainly through
* the ITkPixelCnvTool
*/

StatusCode ITkPixelRawContByteStreamCnv::createRepConst(DataObject* pObj, IOpaqueAddress*& pAddr) const{

    ITkPixelRDO_Container* cont = 0;

    //retrieve & cast the container from SG
    ATH_CHECK(SG::fromStorable(pObj, cont));

    //create BS address
    std::string name = pObj->registry()->name();
    std::unique_ptr<ByteStreamAddress> addr(new ByteStreamAddress(classID(), name, ""));
    pAddr = addr.release();

    //Segfault without this line. No clue why.
    pAddr->addRef();
    ATH_MSG_DEBUG("BS address object created for " << classID() << " name '" << name << "'");

    //Run all the conversion steps scheduled in the ITkPixelCnvTool
    ATH_CHECK(m_cnvTool->convertToByteStream(cont));

    return StatusCode::SUCCESS;
}

const CLID& ITkPixelRawContByteStreamCnv::classID(){
    
    return ClassID_traits<ITkPixelRDO_Container>::ID(); 
}

long ITkPixelRawContByteStreamCnv::storageType(){

    return ByteStreamAddress::storageType();
}