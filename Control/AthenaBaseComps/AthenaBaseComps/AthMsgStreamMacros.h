///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// AthMsgStreamMacros.h 
// Header file for useful macros when comes to using MsgStream
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 
#ifndef ATHENABASECOMPS_ATHMSGSTREAMMACROS_H 
#define ATHENABASECOMPS_ATHMSGSTREAMMACROS_H 1

// CxxUtils
#include <format>

// For the ATH_MSG macros, if a single argument is given, it is streamed
// directly to the MsgStream:
//
//   ATH_MSG_INFO("foo: " << foo) -> msg(lvl) << "foo: " << foo << endmsg
//
// If multiple arguments are given, the arguments are interpreted
// as for std::format:
//
//   ATH_MSG_INFO("foo: {}", foo) -> msg(lvl) << std::format("foo: {}", foo) << endmsg
//

#define ATH_MSG_OPEN (
#define ATH_MSG_CLOSE )

// FIXME: operator precedence ?!
#define ATH_MSG_LVL_NOCHK(lvl, x, ...)          \
  this->msg(lvl) << __VA_OPT__(std::format ATH_MSG_OPEN ) x  __VA_OPT__ (, __VA_ARGS__ ATH_MSG_CLOSE) << endmsg

#define ATH_MSG_LVL(lvl, x, ...)                \
  do {                                          \
    if (this->msgLvl (lvl)) [[unlikely]] {                      \
      ATH_MSG_LVL_NOCHK(lvl, x __VA_OPT__(, __VA_ARGS__));      \
    }                                           \
  } while (0)

#define ATH_MSG_VERBOSE(x, ...) ATH_MSG_LVL(MSG::VERBOSE, x __VA_OPT__(, __VA_ARGS__))
#define ATH_MSG_DEBUG(x, ...)   ATH_MSG_LVL(MSG::DEBUG, x __VA_OPT__(, __VA_ARGS__))
// note that we are using the _NOCHK variant here
#define ATH_MSG_INFO(x, ...)    ATH_MSG_LVL_NOCHK(MSG::INFO, x __VA_OPT__(, __VA_ARGS__))
#define ATH_MSG_WARNING(x, ...) ATH_MSG_LVL_NOCHK(MSG::WARNING, x __VA_OPT__(, __VA_ARGS__))
#define ATH_MSG_ERROR(x, ...)   ATH_MSG_LVL_NOCHK(MSG::ERROR,  x __VA_OPT__(, __VA_ARGS__))
#define ATH_MSG_FATAL(x, ...)   ATH_MSG_LVL_NOCHK(MSG::FATAL,  x __VA_OPT__(, __VA_ARGS__))
#define ATH_MSG_ALWAYS(x, ...)  ATH_MSG_LVL_NOCHK(MSG::ALWAYS, x __VA_OPT__(, __VA_ARGS__))

// can be used like so: ATH_MSG(INFO) << "hello" << endmsg;
#define ATH_MSG(lvl) \
  if (this->msgLvl(MSG::lvl)) this->msg(MSG::lvl)

#endif //> !ATHENABASECOMPS_ATHMSGSTREAMMACROS_H

