/*
    Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file EFTrackingFPGAIntegration/IEFTrackingIntegrationTool.h
 * @author zhaoyuan.cui@cern.ch
 * @date Apr. 11, 2024
 * @brief Interface class for tools
 */

#ifndef EFTRACKINGFPGAINTEGRATION_IEFTRACKINGFPGAINTEGRATIONTOOL_H
#define EFTRACKINGFPGAINTEGRATION_IEFTRACKINGFPGAINTEGRATIONTOOL_H

#include "GaudiKernel/IAlgTool.h"

/**
 * @class IEFTrackingIntegrationTool
 * @brief Abstrct interface class for EFTrackingFPGAIntergration
 */
class IEFTrackingFPGAIntegrationTool : virtual public IAlgTool {
 public:
  // Interface ID declaration for Gaudi
  DeclareInterfaceID(IEFTrackingFPGAIntegrationTool, 1, 0);
};

#endif  // EFTRACKINGFPGAINTEGRATION_IEFTRACKINGFPGAINTEGRATIONTOOL_H