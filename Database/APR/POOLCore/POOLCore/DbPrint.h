/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//====================================================================
//  APR Printer object
//--------------------------------------------------------------------

#ifndef POOL_DBPRINT_H
#define POOL_DBPRINT_H 1

#include <atomic>
#include "AthenaKernel/getMessageSvc.h"
#include "AthenaBaseComps/AthMessaging.h"
#include "SystemTools.h"

namespace pool {

   struct  DbPrintLvl {
      typedef   MSG::Level      MsgLevel;
      static const MsgLevel None        = MsgLevel::NIL;
      static const MsgLevel Verbose     = MsgLevel::VERBOSE;
      static const MsgLevel Debug       = MsgLevel::DEBUG;
      static const MsgLevel Info        = MsgLevel::INFO;
      static const MsgLevel Warning     = MsgLevel::WARNING;
      static const MsgLevel Error       = MsgLevel::ERROR;
      static const MsgLevel Fatal       = MsgLevel::FATAL;
      static const MsgLevel Always      = MsgLevel::ALWAYS;

      static std::atomic<MsgLevel>   outputLvl;
      inline static void         setLevel( MsgLevel l )  { outputLvl.store(l); }
      inline static MSG::Level   getLevel( const std::string& name );
   };


   class DbPrint : public MsgStream {
   public:
     DbPrint( const std::string& name );

     static MsgStream& endmsg( MsgStream& s ) { return ::endmsg(s); }
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
