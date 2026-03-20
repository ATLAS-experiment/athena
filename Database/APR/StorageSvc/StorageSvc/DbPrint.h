/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//====================================================================
//  APR Printer object
//--------------------------------------------------------------------

#ifndef POOL_DBPRINT_H
#define POOL_DBPRINT_H 1

#include "AthenaBaseComps/AthMessaging.h"

#include <atomic>

namespace pool {

   struct  DbPrintLvl {
      static std::atomic<MSG::Level>   outputLvl;
      inline static void         setLevel( MSG::Level l )  { outputLvl.store(l); }
      inline static MSG::Level   getLevel( const std::string& name );
   };


   class DbPrint : public MsgStream {
   public:
     DbPrint( const std::string& name );

     // return the configured output level
     static MSG::Level getOutputLvl();

     // read the output level from environment
     static MSG::Level getOutputLvlFromEnv();

     static MsgStream& endmsg( MsgStream& s )  { return ::endmsg(s); }
   };


   /// @brief  AthMessaging wrapper to set the output level in APR components
   class APRMessaging : public AthMessaging {
   public:
      APRMessaging(const std::string& name);

      APRMessaging(const APRMessaging&) = delete;
      APRMessaging& operator=(const APRMessaging&) = delete;
      APRMessaging(APRMessaging&&) = delete;
      APRMessaging& operator=(APRMessaging&&) = delete;
   };

}       // End namespace pool
#endif  // POOL_DBPRINT_H
