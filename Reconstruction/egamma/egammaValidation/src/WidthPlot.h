/*
Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EGAMMAVALIDATION_WIDTHPLOTS_H
#define EGAMMAVALIDATION_WIDTHPLOTS_H

#include "IHistograms.h"

#include "GaudiKernel/ITHistSvc.h"
#include "GaudiKernel/SmartIF.h"

namespace egammaMonitoring {
  
  class WidthPlot {
  public:

    WidthPlot(std::string name, std::string folder, SmartIF<ITHistSvc> rootHistSvc);
    ~WidthPlot(){ };
    StatusCode fill(IHistograms *input);

  private:
    std::string m_name;
    std::string m_folder;
    SmartIF<ITHistSvc> m_rootHistSvc;

  };
  
}

#endif


