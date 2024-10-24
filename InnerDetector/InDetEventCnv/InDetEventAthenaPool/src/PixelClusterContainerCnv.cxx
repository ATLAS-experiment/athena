/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "StoreGate/StoreGateSvc.h"
#include "PixelClusterContainerCnv.h"
#include "InDetIdentifier/PixelID.h"

#include <memory>

#include <iostream>

  PixelClusterContainerCnv::PixelClusterContainerCnv (ISvcLocator* svcloc)
    : PixelClusterContainerCnvBase(svcloc, "PixelClusterContainerCnv"),
      m_converter_p0()
  {}


StatusCode PixelClusterContainerCnv::initialize() {
   ATH_MSG_INFO("PixelClusterContainerCnv::initialize()");

   ATH_CHECK( PixelClusterContainerCnvBase::initialize() );

   // Get the pixel helper from the detector store
   const PixelID* idhelper(nullptr);
   ATH_CHECK( detStore()->retrieve(idhelper, "PixelID") );

   ATH_CHECK( m_converter_p0.initialize(msg()) );

   ATH_MSG_DEBUG("Converter initialized");

   return StatusCode::SUCCESS;
}


InDet::PixelClusterContainer* PixelClusterContainerCnv::createTransient() {
  static const pool::Guid   p0_guid("37B00A31-EA80-45DF-9A3F-2721EC0F0DA6"); // before t/p split
  static const pool::Guid   p1_guid("9DB54746-8C4E-4A56-8B4C-0E5D42905218"); // with PixelCluster_tlp1
  static const pool::Guid   p2_guid("DE48E26B-9E03-4EAD-86B9-351AD88D060E"); // with pixelCluster_p2
  static const pool::Guid   p3_guid("7BF0F163-B227-434C-86A6-16130E005E6C"); // with pixelCluster_p3
  ATH_MSG_DEBUG("createTransient(): main converter");
  InDet::PixelClusterContainer* p_collection(nullptr);
  if( compareClassGuid(p3_guid) ) {
    ATH_MSG_DEBUG("createTransient(): T/P version 3 detected");
    std::unique_ptr< InDet::PixelClusterContainer_p3 >  p_coll( poolReadObject< InDet::PixelClusterContainer_p3 >() );
    p_collection = m_converter_p3.createTransient( p_coll.get(), msg() );
  } else if( compareClassGuid(p2_guid) ) {
    ATH_MSG_DEBUG("createTransient(): T/P version 2 detected");
    std::unique_ptr< InDet::PixelClusterContainer_p2 >  p_coll( poolReadObject< InDet::PixelClusterContainer_p2 >() );
    p_collection = m_converter_p2.createTransient( p_coll.get(), msg() );
  } else if( compareClassGuid(p1_guid) ) {
    ATH_MSG_DEBUG("createTransient(): T/P version 1 detected");
    std::unique_ptr< InDet::PixelClusterContainer_tlp1 >  p_coll( poolReadObject< InDet::PixelClusterContainer_tlp1 >() );
    p_collection = m_TPConverter.createTransient( p_coll.get(), msg() );
  }
  //----------------------------------------------------------------
  else if( compareClassGuid(p0_guid) ) {
    ATH_MSG_DEBUG("createTransient(): Old input file");

    std::unique_ptr< PixelClusterContainer_p0 >   col_vect( poolReadObject< PixelClusterContainer_p0 >() );
    p_collection = m_converter_p0.createTransient( col_vect.get(), msg() );
  }
  else {
     throw std::runtime_error("Unsupported persistent version of PixelClusterContainer");

  }
  return p_collection;
}


PixelClusterContainer_PERS*    PixelClusterContainerCnv::createPersistent (InDet::PixelClusterContainer* transCont) {
   PixelClusterContainer_PERS *pixdc_p= m_converter_p3.createPersistent( transCont, msg() );
   return pixdc_p;
}


