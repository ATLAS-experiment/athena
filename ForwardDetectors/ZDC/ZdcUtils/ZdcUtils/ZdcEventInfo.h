/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file ZdcUtils/ZdcEventInfo.h
 * @author Brian Cole <bcole@cern.ch>
 * @date August 2023
 * @brief Define enumerations for event-level ZDC data
 */

#ifndef ZDCUTILS__ZdcEventInfo__h_
#define ZDCUTILS__ZdcEventInfo__h_

namespace ZdcEventInfo
{
  enum ZdcEventType {ZdcEventUnknown, ZdcEventPhysics, ZdcEventLED, ZdcSimulation, numEventTypes};
  enum DAQMode {DAQModeUndef = 0, Standalone, PhysicsPEB, CombinedPhysics, MCDigits, numDAQModes};
  enum LEDType {Blue1 = 0, Green = 1, Blue2 = 2, NumLEDs, LEDNone};    
  enum FlagType {DECODINGERROR = 0, UNPACKERROR = 1, ZDCRECOERROR = 2, RPDRECOERROR = 3, ZDCDECODINGERROR = 4, RPDDECODINGERROR = 5, LISRECOERROR = 6, LISDECODINGERROR = 7, numFlagTypes};
  enum LucrodType {LucrodLowGain = 0, LucrodHighGain=1, LucrodRPD1A=2, LucrodRPD2A=3, LucrodRPD1C=4, LucrodRPD2C=5, LucrodLIS=6, numLucrodTypes};
  enum LucrodTotals {nTotalRpdLucrod = 4, nTotalZdcLucrod = 2, nTotalLisLucrod = 1};
};

#endif
