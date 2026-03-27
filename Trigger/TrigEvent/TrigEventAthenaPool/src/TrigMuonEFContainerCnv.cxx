/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigMuonEFContainerCnv.h"
#include "TrigMuonEventTPCnv/TrigMuonEFContainer_p2.h"

//createPersistent 
TrigMuonEFContainer_PERS * TrigMuonEFContainerCnv::createPersistent( TrigMuonEFContainer *transObj)
{
  MsgStream mlog(msgSvc(), "TrigMuonEFContainerConverter" );
  
  mlog << MSG::DEBUG << "TrigMuonEFContainerCnv::createPersistent called" << endmsg;

  TrigMuonEFContainer_PERS * p_muEFcont = m_converter.createPersistent( transObj, mlog );

  return p_muEFcont;

}//end of create persistent method


//createTransient
TrigMuonEFContainer * TrigMuonEFContainerCnv::createTransient(const Token* token)
{
  MsgStream mlog(msgSvc(), "TrigMuonEFContainerConverter" );
  
  mlog << MSG::DEBUG << "TrigMuonEFContainerCnv::createTransient called" << endmsg;
  
  static const pool::Guid p2_guid( "3BB5C107-2975-4F9E-AACC-9FD948C3D2A8" );
  static const pool::Guid p1_guid( "567E65A0-23E8-469F-94F7-A03F6E9C1C2E" );
  static const pool::Guid p0_guid( "BB866230-C9D8-437A-A11B-A0CC08ACD97B" );
  
  if( compareClassGuid(token,  p2_guid ) ){
         std::unique_ptr< TrigMuonEFContainer_p2 > col_vect( poolReadObject< TrigMuonEFContainer_p2 >(token) );
         //         std::cout << "Reading IMFC p2" << std::endl;
         return m_converter.createTransient( col_vect.get(), mlog ) ;

  } else if( compareClassGuid(token,  p1_guid ) ) {
         std::unique_ptr< TrigMuonEFContainer_tlp1 > col_vect( poolReadObject< TrigMuonEFContainer_tlp1 >(token) );
         //  std::cout << "Reading IMFC tlp1" << std::endl;
         return m_converter_tlp1.createTransient( col_vect.get(), mlog );
      
  } else if( compareClassGuid(token,  p0_guid ) ){
      
         return poolReadObject< TrigMuonEFContainer >(token);
      
    } else  throw std::runtime_error( "Unsupported persistent version of TrigMuonEFContainer" );
  
}//end of create transient method

