/*
Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "StorageSvc/DbPrint.h"
#include "AthenaKernel/getMessageSvc.h"

using namespace pool;


// Global variable for setting APR output level from the outside (Athena)
// otherwise the environment variable POOL_OUTMSG_LEVEL can be used for that
std::atomic<MSG::Level> DbPrintLvl::outputLvl = MSG::NIL;


 MSG::Level  DbPrintLvl::getLevel(const std::string& name) 
 {
   // temporary solution to keep Collections logging at WARNING by default
   // while the rest of APR at the level set by Athena or by the environment if not in Athena
   static const std::set<std::string> collNames = {"ImplicitCollection","RootCollection","CollectionSvc"};
   if( collNames.contains(name) or outputLvl == MSG::NIL ) {
      return DbPrint::getOutputLvl(); 
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



MSG::Level DbPrint::getOutputLvl()
{
   // Priority to the ENV setting so debugging is easy
   static const MSG::Level ol = getOutputLvlFromEnv();
   if( ol != MSG::NIL )
   return ol;
   // Last the default
   return MSG::WARNING;
}


MSG::Level DbPrint::getOutputLvlFromEnv()
{
   const char *msg_var = getenv( "POOL_OUTMSG_LEVEL" );
   // Check only the first char of the environment variable
   if( msg_var ) switch( *msg_var ) {
      // NOTE:  NIL is used as 'no setting'
      case 0  :
      case '0':
      case 'n':
      case 'N': return MSG::NIL;

      case '1':
      case 'v':
      case 'V': return MSG::VERBOSE;

      case '2':
      case 'd':
      case 'D': return MSG::DEBUG;

      case '3':
      case 'i':
      case 'I': return MSG::INFO;

      case '4':
      case 'w':
      case 'W': return MSG::WARNING;

      case '5':
      case 'e':
      case 'E': return MSG::ERROR;

      case '6':
      case 'f':
      case 'F': return MSG::FATAL;

      case '7':
      case 'a':
      case 'A': return MSG::ALWAYS;

      default: break;
   }
   return MSG::NIL; 
}


APRMessaging::APRMessaging(const std::string& name) : AthMessaging(name)
{
  setLevel( DbPrintLvl::getLevel(name) );
}
