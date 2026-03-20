/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TRT_DriftCircleContainerCnv.h"
#include "InDetIdentifier/TRT_ID.h"

#include <memory>

#include <iostream>

  TRT_DriftCircleContainerCnv::TRT_DriftCircleContainerCnv (ISvcLocator* svcloc)
    : TRT_DriftCircleContainerCnvBase(svcloc, "TRT_DriftCircleContainerCnv"),
      m_converter_p0()
  {}


StatusCode TRT_DriftCircleContainerCnv::initialize() {
   ATH_MSG_INFO("TRT_DriftCircleContainerCnv::initialize()");

   ATH_CHECK( TRT_DriftCircleContainerCnvBase::initialize() );

   // Get the trt helper from the detector store
   const TRT_ID* idhelper(nullptr);
   ATH_CHECK( detStore()->retrieve(idhelper, "TRT_ID") );

   ATH_CHECK( m_converter_p0.initialize(msg()) );

   ATH_MSG_DEBUG("Converter initialized");

   return StatusCode::SUCCESS;
}


InDet::TRT_DriftCircleContainer* TRT_DriftCircleContainerCnv::createTransient(const Token* token) {

  static const pool::Guid   p0_guid("A99630C5-3D7C-4DB4-9E6C-FC3CEF981895"); // before t/p split
  static const pool::Guid   p1_guid("42B48D79-AF4E-4D45-AAA9-A2BA5A033534"); // with TRT_DriftCircle_tlp1
  static const pool::Guid   p2_guid("36195EDE-941C-424B-81A1-E04C867C35D8"); // with TRT_DriftCircle_p2
  ATH_MSG_DEBUG("createTransient(const Token* token): main converter");
  InDet::TRT_DriftCircleContainer* p_collection(nullptr);
  if( compareClassGuid(token, p2_guid) ) {
    ATH_MSG_DEBUG("createTransient(const Token* token): T/P version 2 detected");  
    std::unique_ptr< InDet::TRT_DriftCircleContainer_p2 >   col_vect( poolReadObject< InDet::TRT_DriftCircleContainer_p2 >() );
    ATH_MSG_DEBUG("Delegate TP converter ");
    p_collection = m_TPConverter2.createTransient( col_vect.get(), msg() );
  }
  else if( compareClassGuid(token, p1_guid) ) {
    ATH_MSG_DEBUG("createTransient(const Token* token): T/P version 1 detected");
    std::unique_ptr< InDet::TRT_DriftCircleContainer_tlp1 >  p_coll( poolReadObject< InDet::TRT_DriftCircleContainer_tlp1 >() );
    p_collection = m_TPConverter.createTransient( p_coll.get(), msg() );
  }
  //----------------------------------------------------------------
  else if( compareClassGuid(token, p0_guid) ) {
    ATH_MSG_DEBUG("createTransient(const Token* token): Old input file");
    std::unique_ptr< TRT_DriftCircleContainer_p0 >   col_vect( poolReadObject< TRT_DriftCircleContainer_p0 >() );
    p_collection = m_converter_p0.createTransient( col_vect.get(), msg() );
  }
  else {
     throw std::runtime_error("Unsupported persistent version of TRT_DriftCircleContainer");

  }
  return p_collection;
}


TRT_DriftCircleContainer_PERS*    TRT_DriftCircleContainerCnv::createPersistent (InDet::TRT_DriftCircleContainer* transCont) {
   TRT_DriftCircleContainer_PERS *trtdc_p= m_TPConverter2.createPersistent( transCont, msg() );
   return trtdc_p;
}


