///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// TruthParticleContainerCnv.cxx 
// Implementation file for class TruthParticleContainerCnv
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 

// STL includes

// Framework includes
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/IToolSvc.h"
#include "GaudiKernel/IAlgTool.h"
#include "StoreGate/StoreGateSvc.h"

// McParticleKernel includes
#include "McParticleKernel/ITruthParticleCnvTool.h"


// McParticleEventTPCnv includes
#include "McParticleEventTPCnv/TruthParticleContainerCnv_p5.h"
#include "McParticleEventTPCnv/TruthParticleContainerCnv_p6.h"

// McParticleEventAthenaPool includes
#include "TruthParticleContainerCnv.h"

TruthParticleContainerCnv::TruthParticleContainerCnv(ISvcLocator* svcLocator) :
  T_AthenaPoolCustomCnv<TruthParticleContainer, 
                        TruthParticleContainer_PERS>(svcLocator)
{
  if ( m_cnvTool.retrieve().isFailure() ) {
    throw std::runtime_error("Could not fetch TruthParticleCnvTool !!");
  }
}


TruthParticleContainer_PERS* 
TruthParticleContainerCnv::createPersistent(TruthParticleContainer* trans)
{
  MsgStream log( msgSvc(), "TruthParticleContainerCnv" );

  TruthParticleContainerCnv_p6 cnv( m_cnvTool.get() );
  TruthParticleContainer_PERS *pers = cnv.createPersistent(trans, log);

  log << MSG::DEBUG << "::createPersistent [Success]" << endmsg;
  return pers; 
}

TruthParticleContainer* TruthParticleContainerCnv::createTransient(const Token* token) 
{
   MsgStream msg( msgSvc(), "TruthParticleContainerCnv" );

   TruthParticleContainer *trans = 0;

   static const Guid p5_guid("2D25E3D9-950B-49E0-A51F-2B6EC93D1A23");
   static const Guid p6_guid("97AC2CEE-7E8A-4E2E-B6B5-FD8545D77FC4");

   if ( compareClassGuid(token, p6_guid) ) {
     
     std::unique_ptr<TruthParticleContainer_p6> pers( poolReadObject<TruthParticleContainer_p6>(token) );
     TruthParticleContainerCnv_p6 cnv( m_cnvTool.get() );
     trans = cnv.createTransient( pers.get(), msg );

   } else if ( compareClassGuid(token, p5_guid) ) {
     
     std::unique_ptr<TruthParticleContainer_p5> pers( poolReadObject<TruthParticleContainer_p5>(token) );
     TruthParticleContainerCnv_p5 cnv( m_cnvTool.get() );
     trans = cnv.createTransient( pers.get(), msg );
     
   } else {
     throw std::runtime_error("Unsupported persistent version of TruthParticleContainer");
   }

   return trans;
}
