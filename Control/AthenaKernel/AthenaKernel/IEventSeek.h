// This file's extension implies that it's C, but it's really -*- C++ -*-.

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// $Id: IEventSeek.h,v 1.1 2005-05-05 00:15:02 calaf Exp $
/**
 * @file  IEventSeek.h
 * @author scott snyder <snyder@bnl.gov>
 * @date May, 2005
 * @brief Abstract interface for seeking within an event stream.
 */

#ifndef ATHENAKERNEL_IEVENTSEEK_H
#define ATHENAKERNEL_IEVENTSEEK_H 1

#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/INamedInterface.h"

/**
 * @class IEventSeek
 * @brief Abstract interface for seeking within an event stream.
 */

class IEventSeek : virtual public INamedInterface
{
public:
  DeclareInterfaceID(IEventSeek, 1, 0);


  /**
   * @brief Seek to a given event number.
   * @param evtnum  The event number to which to seek.
   */
  virtual StatusCode seek (int evtnum) = 0;

  /**
   * @brief return the current event number.
   * @return The current event number.
   */
  virtual int curEvent () const = 0;

  /**
   * @brief Supply the absolute (skip-relative) input event index for the
   *        next event, for run-dependent EventID modification.
   *
   * In serial mode AthenaEventLoopMgr derives this from its own counter;
   * in AthenaMP the per-worker counter does not match the input position,
   * so the MP event consumer must supply it. Default no-op. ATEAM-1154.
   */
  virtual void setNextEventModifierIndex (long /*idx*/) {}
};


#endif // not ATHENAKERNEL_IEVENTSEEK_H
