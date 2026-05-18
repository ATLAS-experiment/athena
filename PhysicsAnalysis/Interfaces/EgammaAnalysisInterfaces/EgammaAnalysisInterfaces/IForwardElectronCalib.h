/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/
#ifndef EGAMMAANALYSISINTERFACES_IFORWARDELECTRONCALIBTOOL_H
#define EGAMMAANALYSISINTERFACES_IFORWARDELECTRONCALIBTOOL_H

#include "AsgTools/IAsgTool.h"

// EDM includes
#include "xAODEgamma/EgammaFwd.h"
#include "xAODCaloEvent/CaloClusterFwd.h"

#include "GlobalEventInfo.h"

class EventContext;

/**
 * @class IForwardElectronCalib
 * @brief A tool used to Compute the pT Calibration.
 **/
class IForwardElectronCalib : virtual public asg::IAsgTool{
  ASG_TOOL_INTERFACE(IForwardElectronCalib)
public:
  virtual ~IForwardElectronCalib() override {};

  ///Return Calibration pT for the forward electron
  virtual double calibrate(const EventContext& ctx,
			  const xAOD::Electron* eg) const = 0;

};

#endif
