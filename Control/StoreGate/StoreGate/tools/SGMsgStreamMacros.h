// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file StoreGate/SGMsgStreamMacros.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Aug, 2026
 * @brief Helper macros for writing to MsgStream.
 */


#ifndef STOREGATE_SGMSGSTREAMMACROS_H
#define STOREGATE_SGMSGSTREAMMACROS_H


#include "GaudiKernel/MsgStream.h"


// HACK LIFTED FROM AthenaBaseComps/AthMsgStreamMacros.h to remove dep loop
#define SG_MSG_OPEN (
#define SG_MSG_CLOSE )
#define SG_MSG_LVL(lvl, x, ...)                 \
  do {                                          \
    if (msgLevel(lvl)) {                        \
       msgStream(lvl) << __VA_OPT__(std::format SG_MSG_OPEN ) x  __VA_OPT__ (, __VA_ARGS__ SG_MSG_CLOSE) << endmsg; \
    }                                           \
  } while (0)
 
#define SG_MSG_VERBOSE(x, ...) SG_MSG_LVL(MSG::VERBOSE, x __VA_OPT__(, __VA_ARGS__))
#define SG_MSG_DEBUG(x, ...)   SG_MSG_LVL(MSG::DEBUG, x __VA_OPT__(, __VA_ARGS__))
#define SG_MSG_INFO(x, ...)    SG_MSG_LVL(MSG::INFO, x __VA_OPT__(, __VA_ARGS__))
#define SG_MSG_WARNING(x, ...) SG_MSG_LVL(MSG::WARNING, x __VA_OPT__(, __VA_ARGS__))
#define SG_MSG_ERROR(x, ...)   SG_MSG_LVL(MSG::ERROR, x __VA_OPT__(, __VA_ARGS__))
#define SG_MSG_FATAL(x, ...)   SG_MSG_LVL(MSG::FATAL, x __VA_OPT__(, __VA_ARGS__))

#endif // not STOREGATE_SGMSGSTREAMMACROS_H
