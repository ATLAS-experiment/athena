/*
Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "POOLCore/DbPrint.h"


using namespace pool;

// Global variable for setting APR output level from the outside (Athena)
// otherwise the environment variable POOL_OUTMSG_LEVEL can be used for that
std::atomic<DbPrintLvl::MsgLevel> DbPrintLvl::outputLvl = MSG::NIL;


 MSG::Level  DbPrintLvl::getLevel(const std::string& name) 
 {
   // temporary solution to keep Collections logging at WARNING by default
   // while the rest of APR at the level set by Athena or by the environment if not in Athena
   static const std::set<std::string> collNames = {"RNTCollection","RootCollection","POOLCollFactory",
                                                   "CollectionBase","ImplicitCollection"};
   if( collNames.contains(name) or outputLvl == MSG::NIL ) {
      return SystemTools::GetOutputLvl(); 
   } else {
      return outputLvl;
   }
}


DbPrint::DbPrint( const std::string& name )
   // 'quiet=true' option only works with 'Eager' creation
   : MsgStream( Athena::getMessageSvc(Athena::Options::Eager,true), name )
{
   setLevel( DbPrintLvl::getLevel(name) );
}

APRMessaging::APRMessaging(const std::string& name) : AthMessaging(name)
{
  auto msgSvc = Athena::getMessageSvc(Athena::Options::Eager,true);
  msgSvc->setOutputLevel(name, DbPrintLvl::getLevel(name) );
}


  