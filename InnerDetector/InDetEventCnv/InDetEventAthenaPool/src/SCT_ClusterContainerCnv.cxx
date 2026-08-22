/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "SCT_ClusterContainerCnv.h"

#include "MsgUtil.h"

#include "InDetIdentifier/SCT_ID.h"
#include "StoreGate/StoreGateSvc.h"

#include <iostream>
#include <memory>

  SCT_ClusterContainerCnv::SCT_ClusterContainerCnv (ISvcLocator* svcloc)
    : SCT_ClusterContainerCnvBase(svcloc, "SCT_ClusterContainerCnv"),
      m_converter_p0()
  {}


StatusCode SCT_ClusterContainerCnv::initialize() {
   ATH_MSG_INFO("SCT_ClusterContainerCnv::initialize()");

   ATH_CHECK( SCT_ClusterContainerCnvBase::initialize() );

   // Get the SCT helper from the detector store
   const SCT_ID* idhelper(nullptr);
   ATH_CHECK( detStore()->retrieve(idhelper, "SCT_ID") );

   ATH_CHECK( m_converter_p0.initialize(msg()) );

   ATH_MSG_DEBUG("Converter initialized");

   return StatusCode::SUCCESS;
}


InDet::SCT_ClusterContainer* SCT_ClusterContainerCnv::createTransient(const Token* token) {
  //  MsgStream log(msgSvc(), "SCT_ClusterContainerCnv" );
  static const Guid   p0_guid("A180F372-0D52-49C3-8AA0-0939CB0B8179"); // before t/p split
  static const Guid   p1_guid("657F6546-F5CD-4166-9567-16AD9C96D286"); // with SCT_Cluster_tlp1
  static const Guid   p2_guid("ECE7D831-0F31-4E6F-A6BE-2ADDE90083BA"); // with SCT_Cluster_p2
  static const Guid   p3_guid("623F5836-369F-4A94-9DD4-DAD728E93C13"); // with SCT_Cluster_p3

  //ATH_MSG_DEBUG("createTransient(const Token* token): main converter");
  InDet::SCT_ClusterContainer* p_collection(nullptr);
  if ( compareClassGuid(token, p3_guid) ) {
    //ATH_MSG_DEBUG("createTransient(const Token* token): T/P version 3 detected");
    std::unique_ptr< SCT_ClusterContainer_PERS >  p_coll( poolReadObject< SCT_ClusterContainer_PERS >(token) );
    p_collection = m_TPConverter_p3.createTransient( p_coll.get(), msg() );
   
  } else if ( compareClassGuid(token, p1_guid) ) {
    //ATH_MSG_DEBUG("createTransient(const Token* token): T/P version 1 detected");
    std::unique_ptr< InDet::SCT_ClusterContainer_tlp1 >  p_coll( poolReadObject< InDet::SCT_ClusterContainer_tlp1 >(token) );
    p_collection = m_TPConverter.createTransient( p_coll.get(), msg() );

  } else if ( compareClassGuid(token, p2_guid) ) {
    //ATH_MSG_DEBUG("createTransient(const Token* token): T/P version 2 detected");
    std::unique_ptr< InDet::SCT_ClusterContainer_p2 >  p_coll( poolReadObject< InDet::SCT_ClusterContainer_p2 >(token) );
    p_collection = m_TPConverter_p2.createTransient( p_coll.get(), msg() );

  } else if ( compareClassGuid(token, p0_guid) ) {
    //ATH_MSG_DEBUG("createTransient(const Token* token): Old input file");
    std::unique_ptr< SCT_ClusterContainer_p0 >  col_vect( poolReadObject< SCT_ClusterContainer_p0 >(token) );
    p_collection = m_converter_p0.createTransient( col_vect.get(), msg() );

  } else {
     throw std::runtime_error("Unsupported persistent version of SCT_ClusterContainer");

  }
  return p_collection;
}


SCT_ClusterContainer_PERS* SCT_ClusterContainerCnv::createPersistent (InDet::SCT_ClusterContainer* transCont) {
   SCT_ClusterContainer_PERS* sctdc_p= m_TPConverter_p3.createPersistent( transCont, msg() );
   return sctdc_p;
}
