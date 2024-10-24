/*
Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EGAMMAVALIDATION_EFFICIENCYPLOTS_H
#define EGAMMAVALIDATION_EFFICIENCYPLOTS_H

#include "IHistograms.h"

#include "GaudiKernel/ITHistSvc.h"
#include "GaudiKernel/SmartIF.h"


namespace egammaMonitoring {
  
  class EfficiencyPlot {
  public:

    EfficiencyPlot(std::string name, std::string folder, SmartIF<ITHistSvc> rootHistSvc);
    ~EfficiencyPlot() { };
    StatusCode divide(IHistograms *pass, IHistograms *total);

  private:
    std::string m_name;
    std::string m_folder;
    SmartIF<ITHistSvc> m_rootHistSvc;

  };
  
}

#endif


