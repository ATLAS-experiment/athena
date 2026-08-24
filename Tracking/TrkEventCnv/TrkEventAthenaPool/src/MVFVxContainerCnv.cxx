/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

//-----------------------------------------------------------------------------
//
// file:   MVFVxContainerCnv.cxx
// author: Kirill Prokofiev <Kirill.Prokofiev@cern.ch>
//
//-----------------------------------------------------------------------------


#include "MVFVxContainerCnv.h"


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------

MVFVxContainerCnv::MVFVxContainerCnv( ISvcLocator *svcloc ):
                           MVFVxContainerCnvBase(svcloc)
 {      

 }

//-----------------------------------------------------------------------------
// Initializer
//-----------------------------------------------------------------------------
StatusCode MVFVxContainerCnv::initialize()
{
    MsgStream log(msgSvc());
    
 StatusCode sc = MVFVxContainerCnvBase::initialize();
  if( sc.isFailure() ) 
  {
    log << MSG::FATAL << "Could not initialize MVFVxContainerCnvBase" << endmsg;
    return sc;
  }
  
//-------------------------------------------------------------------------
// Set up the message stream
//-------------------------------------------------------------------------
  // log.setLevel( m_msgSvc->outputLevel() );
  log << MSG::INFO << "MVFVxContainerCnv::initialize()" << endmsg; 
  return StatusCode::SUCCESS;
}


MVFVxContainer_PERS * MVFVxContainerCnv::createPersistent( MVFVxContainer* )
{ 
   MsgStream log(msgSvc(), "MVFVxContainerCnv" );
   log << MSG::ERROR << "createPersistent() is obsolete" << endmsg;
   return nullptr;
}


MVFVxContainer * MVFVxContainerCnv::createTransient(const Token* token)
{
    MsgStream log(msgSvc());
 static const Guid p1_guid( "D7BAA7AD-1A46-4DA3-9CA7-350A1A3F0656" );
 static const Guid p0_guid( "6C6999B7-F961-4B72-B6D9-DF71CB2364CC" );

 MVFVxContainer *p_collection = nullptr;
 
 if( compareClassGuid(token,  p1_guid ) ) {
    // std::cout << "MVFVxContainerCnv::createTransient(const Token* token)" << std::endl;
    poolReadObject< MVFVxContainer_PERS >( m_TPConverter, token );
    p_collection = m_TPConverter.createTransient( log );
  
 } else if( compareClassGuid(token,  p0_guid ) ) {
    // std::cout << "MVFVxContainerCnv::createTransient: use old converter" << std::endl;  
    p_collection = poolReadObject< MVFVxContainer >(token); 
 } else
    throw std::runtime_error( "Unsupported persistent version of MVFVxContainer (unknown GUID)" );
    
 return p_collection; 
}


void        MVFVxContainerCnv::readObjectFromPool( const Token* token )
{
  static const Guid p1_guid( "D7BAA7AD-1A46-4DA3-9CA7-350A1A3F0656" );
  
   // select the object type based on its GUID 
   if( compareClassGuid(token,  p1_guid ) ) {
      // read the object using the main TP converter
      poolReadObject< MVFVxContainer_PERS >( m_TPConverter, token );
   }
   else
      throw std::runtime_error( "Unsupported version of MVFVxContainer (unknown GUID)" );
}
