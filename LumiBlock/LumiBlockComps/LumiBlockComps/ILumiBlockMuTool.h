/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

/**
   @file ILumiBlockMuTool.h
   @class ILumiBlockMuTool
   @brief Provides interface for LumiBlockMuTool
   @author E.Torrence
**/

#ifndef LUMIBLOCKCOMPS_ILumiBlockMuTool_H
#define LUMIBLOCKCOMPS_ILumiBlockMuTool_H

#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/EventContext.h"

class ILumiBlockMuTool: virtual public IAlgTool {

 public:
  DeclareInterfaceID(ILumiBlockMuTool, 1, 0);

  // Interface
  virtual float averageInteractionsPerCrossing(const EventContext& ctx) const = 0;
  virtual float actualInteractionsPerCrossing(const EventContext& ctx) const = 0;

 private:
};

#endif
