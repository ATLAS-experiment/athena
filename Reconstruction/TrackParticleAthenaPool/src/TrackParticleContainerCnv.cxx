/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

//-----------------------------------------------------------------------------
//
// file:   TrackParticleContainerCnv.cxx
// author: Andreas Wildauer
//
//-----------------------------------------------------------------------------
#include "TrackParticleContainerCnv.h"
#include "AthenaBaseComps/AthCheckMacros.h"
#include <iostream>
//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------

TrackParticleContainerCnv::TrackParticleContainerCnv( ISvcLocator *svcloc ):
                           TrackParticleContainerCnvBase(svcloc),
                           m_msgSvc( msgSvc() )
 {}
 
//-----------------------------------------------------------------------------
// Initializer
//-----------------------------------------------------------------------------
StatusCode TrackParticleContainerCnv::initialize()
{
  ATH_CHECK( TrackParticleContainerCnvBase::initialize() );
  return StatusCode::SUCCESS;
}//end of initialize method

TrackParticleContainer_PERS *
TrackParticleContainerCnv::createPersistent( Rec::TrackParticleContainer *transCont)
{
  TrackParticleContainer_PERS * tpBaseCont = m_TPConverter_tlp3.createPersistent( transCont, msg() );

  return tpBaseCont;
}//end of create persistent method

Rec::TrackParticleContainer * TrackParticleContainerCnv::createTransient(const Token* token)
{
  std::string logname = "TrackParticleContainerCnv";
  MsgStream log (m_msgSvc, logname);
  static const Guid p3_guid( "8C84D957-1899-4C98-A49D-D6AC0F85C5EC" );
  static const Guid p2_guid( "170211F9-C4E1-4173-B0FB-71322899C8B9" );
  static const Guid p1_guid( "4A8CEB2C-0833-4C83-8514-A69928DE4672" );
  static const Guid p0_guid( "35BE1E01-658C-438A-AC86-F951006ECC6B" );

  Rec::TrackParticleContainer *p_collection = 0;
  if( compareClassGuid(token,  p3_guid ) ) {
    // std::cout<<"TrackParticleContainerCnv::createTransient p3_guid"<<std::endl;
    
    poolReadObject< Rec::TrackParticleContainer_tlp3 >(m_TPConverter_tlp3, token);
    p_collection = m_TPConverter_tlp3.createTransient(log);
  } 
  else if( compareClassGuid(token,  p2_guid ) ) {
    // std::cout<<"TrackParticleContainerCnv::createTransient p2_guid"<<std::endl;
    
    poolReadObject< Rec::TrackParticleContainer_tlp2 >(m_TPConverter_tlp2, token);
    p_collection = m_TPConverter_tlp2.createTransient(log);
  } 
  else if( compareClassGuid(token,  p1_guid ) ) {
   poolReadObject< Rec::TrackParticleContainer_tlp1 >(m_TPConverter_tlp1, token);
   p_collection = m_TPConverter_tlp1.createTransient(log);
  }
  else if( compareClassGuid(token,  p0_guid ) ) {
   p_collection = poolReadObject< Rec::TrackParticleContainer >(token);
  }
  else {
    throw std::runtime_error( "Unsupported persistent version of Rec::TrackParticleContainer" );
  }
   
  return p_collection;
  
}//end of create transient method


