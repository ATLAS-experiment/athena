/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigT2JetContainerCnv.h"
#include "TrigCaloEventTPCnv/TrigT2JetContainer_tlp1.h"
#include "TrigCaloEventTPCnv/TrigT2JetContainer_p3.h"

//createPersistent 
TrigT2JetContainer_PERS * TrigT2JetContainerCnv::createPersistent( TrigT2JetContainer *transObj)
{

  MsgStream mlog(msgSvc(), "TrigT2JetContainerConverter" );

  mlog << MSG::DEBUG << "TrigT2JetContainerCnv::createPersistent" << endmsg;

  TrigT2JetContainer_PERS* p_T2JetCont = m_converter.createPersistent( transObj, mlog );
 
  return p_T2JetCont;
 
}//end of create persistent method
 

//createTransient
TrigT2JetContainer * TrigT2JetContainerCnv::createTransient(const Token* token)
{
  
  MsgStream mlog(msgSvc(), "TrigT2JetContainerConverter" );

  mlog << MSG::DEBUG << "TrigT2JetContainerCnv::createTransient called" << endmsg;

  static const Guid tlp1_guid( "3B670168-C5AA-48A1-9813-C94530980EBF" );
  static const Guid p3_guid( "6215BEE2-45E7-4681-9089-9BD470CDAF4D" );


  if( compareClassGuid(token,  p3_guid ) ){
         std::unique_ptr< TrigT2JetContainer_p3 > col_vect( poolReadObject< TrigT2JetContainer_p3 >(token) );
         //std::cout << "Reading TTCC p3" << std::endl;
         return m_converter.createTransient( col_vect.get(), mlog ) ;
  } else if( compareClassGuid(token,  tlp1_guid ) ) {
         std::unique_ptr< TrigT2JetContainer_tlp1 > col_vect( poolReadObject< TrigT2JetContainer_tlp1 >(token) );
         //std::cout << "Reading TTC tlp1" << std::endl;
         return m_converter_tlp1.createTransient( col_vect.get(), mlog );
  } else  throw std::runtime_error( "Unsupported persistent version of TrigT2JetContainer" );
     
}//end of create transient method
 
