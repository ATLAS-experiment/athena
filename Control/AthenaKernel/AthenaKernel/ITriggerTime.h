/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHENAKERNEL_ITRIGGERTIME_H
#define ATHENAKERNEL_ITRIGGERTIME_H
/** @file ITriggerTime.h
 * @brief  interface to a tool that returns the
 *         time offset of the current trigger. Used by PileUpMergeSvc
 * @author Paolo Calafiura <pcalafiura@lbl.gov> - ATLAS Collaboration
 */

#include "GaudiKernel/IAlgTool.h"

/** @class ITriggerTime
 * @brief  interface to a tool that returns the
 *         time offset of the current trigger. Used by PileUpMergeSvc
 */
class ITriggerTime : public virtual IAlgTool {
public:
  DeclareInterfaceID(ITriggerTime, 1, 0);

  /// returns the time offset of the current trigger
  virtual double time() = 0;
};
#endif
