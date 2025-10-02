/* -*- C++ -*- */

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/** @file SGWPtr.h
 *  return type of StoreGateSvc::create. 
 *  Currently a plain pointer, may become a handle informing the scheduler
 *  when we are done "writing"
 */
#ifndef STOREGATE_SGWPTR_H
#define STOREGATE_SGWPTR_H
namespace SG {
  template<class T> using WPtr = T*;
}
#endif // STOREGATE_SGWPTR_H
