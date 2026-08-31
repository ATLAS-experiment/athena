///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// McEventCollectionCnv.cxx 
// Implementation file for class McEventCollectionCnv
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 

// Framework includes
#include "GaudiKernel/MsgStream.h"

// GeneratorObjectsTPCnv includes
#include "GeneratorObjectsTPCnv/McEventCollectionCnv_p1.h"
#include "GeneratorObjectsTPCnv/McEventCollectionCnv_p2.h"
#include "GeneratorObjectsTPCnv/McEventCollectionCnv_p3.h"
#include "GeneratorObjectsTPCnv/McEventCollectionCnv_p4.h"
#include "GeneratorObjectsTPCnv/McEventCollectionCnv_p5.h"
#include "GeneratorObjectsTPCnv/McEventCollectionCnv_p6.h"
#include "GeneratorObjectsTPCnv/McEventCollectionCnv_p7.h"

// GeneratorObjectsAthenaPool includes
#include "McEventCollectionCnv.h"

McEventCollection_PERS* 
McEventCollectionCnv::createPersistent( McEventCollection* transCont ) 
{
  MsgStream msg( msgSvc(), "McEventCollectionCnv" );

  McEventCollectionCnv_p6 cnv;
  McEventCollection_PERS *persObj = cnv.createPersistent( transCont, msg );

  msg << MSG::DEBUG << "::createPersistent [Success]" << endmsg;
  return persObj; 
}

McEventCollection* McEventCollectionCnv::createTransient(const Token* token) 
{
   MsgStream msg( msgSvc(), "McEventCollectionConverter" );

   McEventCollection *transObj = 0;

   static const Guid tr_guid("6DE62B45-7C72-4539-92F2-3A8E739A4AC3");
   static const Guid p1_guid("BF93438C-D1D3-4F1C-8850-EB690AB7C416");
   static const Guid p2_guid("851BB1D2-1964-4B0A-B83A-6BD596CFB5E2");
   static const Guid p3_guid("6FC41599-64D6-4DB9-973E-9493166F6291");
   static const Guid p4_guid("C517102A-94DE-407C-B07F-09BD81F6172E");
   static const Guid p5_guid("D52391A4-F951-46BF-A0D5-E407698D2917");
   static const Guid p6_guid("6B78A751-B31A-4597-BFB6-DDCE62646CF9");
   static const Guid p7_guid("A3A04C74-A278-4E35-ACF8-4C6B8C32FC56");
   // Hook to disable datapool if we are doing pileup
   bool isPileup(false);
   if(serviceLocator()->existsService("PileUpEventLoopMgr")) {
      isPileup=true;
      msg << MSG::DEBUG << "Pile run, disable datapool for McEventCollection " << endmsg;
   }

   if ( compareClassGuid(token, tr_guid) ) {

     // regular object from before the T/P separation
     return poolReadObject<McEventCollection>(token);

   } else if ( compareClassGuid(token, p1_guid) ) {

      std::unique_ptr<McEventCollection_p1> persObj( poolReadObject<McEventCollection_p1>(token) );
      McEventCollectionCnv_p1 cnv;
      transObj = cnv.createTransient( persObj.get(), msg );
   } else if ( compareClassGuid(token, p2_guid) ) {

      std::unique_ptr<McEventCollection_p2> persObj( poolReadObject<McEventCollection_p2>(token) );
      McEventCollectionCnv_p2 cnv;
      transObj = cnv.createTransient( persObj.get(), msg );
   } else if ( compareClassGuid(token, p3_guid) ) {

      std::unique_ptr<McEventCollection_p3> persObj( poolReadObject<McEventCollection_p3>(token) );
      McEventCollectionCnv_p3 cnv;
      transObj = cnv.createTransient( persObj.get(), msg ); 
   } else if ( compareClassGuid(token, p4_guid) ) {

      std::unique_ptr<McEventCollection_p4> persObj( poolReadObject<McEventCollection_p4>(token) );
      McEventCollectionCnv_p4 cnv;
      if(isPileup) cnv.setPileup();
      transObj = cnv.createTransient( persObj.get(), msg );
   } else if ( compareClassGuid(token, p5_guid) ) {

      std::unique_ptr<McEventCollection_p5> persObj( poolReadObject<McEventCollection_p5>(token) );
      McEventCollectionCnv_p5 cnv;
      if(isPileup) cnv.setPileup();
      transObj = cnv.createTransient( persObj.get(), msg );
   } else if ( compareClassGuid(token, p6_guid) ) {

      std::unique_ptr<McEventCollection_p6> persObj( poolReadObject<McEventCollection_p6>(token) );
      McEventCollectionCnv_p6 cnv;
      if(isPileup) cnv.setPileup();
      transObj = cnv.createTransient( persObj.get(), msg );
   } else if ( compareClassGuid(token, p7_guid) ) {

      std::unique_ptr<McEventCollection_p7> persObj( poolReadObject<McEventCollection_p7>(token) );
      McEventCollectionCnv_p7 cnv;
      if(isPileup) cnv.setPileup();
      transObj = cnv.createTransient( persObj.get(), msg );
  } else {
     throw std::runtime_error("Unsupported persistent version of McEventCollection");
   }

   return transObj;
}
