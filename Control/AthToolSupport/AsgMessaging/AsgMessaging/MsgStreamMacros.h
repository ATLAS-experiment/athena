/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ASGMESSAGING_MSGSTREAMMACROS_H
#define ASGMESSAGING_MSGSTREAMMACROS_H

// Local include(s):
#include "AsgMessaging/MsgLevel.h"

// Pull in the definition from Athena:
#ifndef XAOD_STANDALONE
#   include "AthenaBaseComps/AthMsgStreamMacros.h"
#else // not XAOD_STANDALONE

#include "CxxUtils/normalizeFunctionName.h"

// Not sure why this is needed...
#undef ERROR

// This is a GCC extension for getting the name of the current function.
#if defined( __GNUC__ )
#   define MSGSTREAM_FNAME CxxUtils::normalizeFunctionName(__PRETTY_FUNCTION__)
#else
#   define MSGSTREAM_FNAME ""
#endif

/// Common prefix for the non-usual messages
///
/// The idea is that a regular user usually only wants to see DEBUG, INFO
/// and some WARNING messages. So those should be reasonably short. On the other
/// hand serious warnings (ERROR, FATAL) messages should be as precise
/// as possible to make debugging the issue easier.
///
#define MSGSTREAM_REPORT_PREFIX \
   __FILE__ << ":" << __LINE__ << " (" << MSGSTREAM_FNAME << "): "

#define ATH_MSG_OPEN (
#define ATH_MSG_CLOSE )

/// Macro used to print "serious" messages
#define ATH_MSG_LVL_SERIOUS( lvl, xmsg, ... )                       \
  this->msg(lvl) << MSGSTREAM_REPORT_PREFIX << __VA_OPT__(std::format ATH_MSG_OPEN ) xmsg  __VA_OPT__ (, __VA_ARGS__ ATH_MSG_CLOSE) << endmsg

/// Macro used to print "regular" messages
#define ATH_MSG_LVL_NOCHK( lvl, xmsg, ... )         \
  this->msg(lvl) << __VA_OPT__(std::format ATH_MSG_OPEN ) xmsg  __VA_OPT__ (, __VA_ARGS__ ATH_MSG_CLOSE) << endmsg

/// Macro used to print "protected" messages
#define ATH_MSG_LVL( lvl, xmsg, ... )           \
   do {                                         \
      if( msg().msgLevel( lvl ) ) {             \
         ATH_MSG_LVL_NOCHK( lvl, xmsg __VA_OPT__(, __VA_ARGS__) );     \
      }                                         \
   } while( 0 )

/// Macro printing verbose messages
#define ATH_MSG_VERBOSE( xmsg, ... )  ATH_MSG_LVL( MSG::VERBOSE, xmsg __VA_OPT__(, __VA_ARGS__) )
/// Macro printing debug messages
#define ATH_MSG_DEBUG( xmsg, ... )    ATH_MSG_LVL( MSG::DEBUG, xmsg __VA_OPT__(, __VA_ARGS__) )
/// Macro printing info messages
#define ATH_MSG_INFO( xmsg, ... )     ATH_MSG_LVL_NOCHK( MSG::INFO,  xmsg __VA_OPT__(, __VA_ARGS__) )
/// Macro printing warning messages
#define ATH_MSG_WARNING( xmsg, ... )  ATH_MSG_LVL_NOCHK( MSG::WARNING, xmsg __VA_OPT__(, __VA_ARGS__) )
/// Macro printing error messages
#define ATH_MSG_ERROR( xmsg , ...)    ATH_MSG_LVL_SERIOUS( MSG::ERROR, xmsg __VA_OPT__(, __VA_ARGS__) )
/// Macro printing fatal messages
#define ATH_MSG_FATAL( xmsg, ... )    ATH_MSG_LVL_SERIOUS( MSG::FATAL, xmsg __VA_OPT__(, __VA_ARGS__) )
/// Macro printing messages that should always appear
#define ATH_MSG_ALWAYS( xmsg, ... )   ATH_MSG_LVL_NOCHK( MSG::ALWAYS, xmsg __VA_OPT__(, __VA_ARGS__) )

/// can be used like so: ATH_MSG(INFO) << "hello" << endmsg;
#define ATH_MSG(lvl) \
  if( msg().msgLevel( MSG::lvl ) ) msg(MSG::lvl)

#endif // not XAOD_STANDALONE
#endif // ASGMESSAGING_MSGSTREAMMACROS_H
