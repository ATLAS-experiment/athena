/** emacs: this is -*- c++ -*- 
 **
 **   @file    MsgMacros.h        
 **
 **            proper macros that you can use like a proper ostream
 **                   
 **   @author  sutt
 **   @date    Wed  8 Jul 2026 12:08:44 BST
 **
 **   $Id: MsgMacros.h, v0.0   Wed  8 Jul 2026 12:08:44 BST sutt $
 **
 **   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **/

#ifndef  TRIGIDR4MONITORING_MSGMACROS_H
#define  TRIGIDR4MONITORING_MSGMACROS_H

/// if you want to compile these out, just replace by if (false) and the compiler
/// optimisation will just exclude the entire line

#define MY_MSG_FATAL   if ( !this->msgLvl(MSG::FATAL)   ) ; else this->msg(MSG::FATAL) 
#define MY_MSG_ERROR   if ( !this->msgLvl(MSG::ERROR)   ) ; else this->msg(MSG::ERROR) 
#define MY_MSG_WARNING if ( !this->msgLvl(MSG::WARNING) ) ; else this->msg(MSG::WARNING) 
#define MY_MSG_INFO    if ( !this->msgLvl(MSG::INFO)    ) ; else this->msg(MSG::INFO) 
#define MY_MSG_DEBUG   if ( !this->msgLvl(MSG::DEBUG)   ) ; else this->msg(MSG::DEBUG) 
#define MY_MSG_VERBOSE if ( !this->msgLvl(MSG::VERBOSE) ) ; else this->msg(MSG::VERBOSE)  


template<class T>
T* msg_ptr(T* p) { return p; }

template<class T>
T* msg_ptr(T& r) { return &r; }


#define IDTPM2_MSG_INFO0() \
    if (!this->msgLvl(MSG::INFO)) ; else this->msg(MSG::INFO)

#define IDTPM2_MSG_INFO1(x) \
    if (!msg_ptr(x)->msgLvl(MSG::INFO)) ; else msg_ptr(x)->msg(MSG::INFO)

#define FA_MSG_INFO(...) \
    IDTPM2_MACRO(__VA_ARGS__, IDTPM2_MSG_INFO1, IDTPM2_MSG_INFO0)(__VA_ARGS__)

#endif  // TRIGIDR4MONITORING_MSGMACROS_H









