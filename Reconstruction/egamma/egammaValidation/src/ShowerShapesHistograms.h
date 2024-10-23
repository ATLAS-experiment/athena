/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EGAMMAVALIDATION_SHOWERSHAPESHISTOGRAMS_H
#define EGAMMAVALIDATION_SHOWERSHAPESHISTOGRAMS_H

#include <map>

#include "GaudiKernel/ITHistSvc.h"
#include "GaudiKernel/SmartIF.h"

#include "xAODEgamma/Egamma.h"

class TH1D;
class TH2D;

namespace egammaMonitoring {

  class ShowerShapesHistograms {
  public:

    // Histos
    ShowerShapesHistograms(std::string name,
			   std::string title,
			   std::string folder,
			   SmartIF<ITHistSvc> rootHistSvc) :
      m_name(std::move(std::move(name))),
      m_title(std::move(std::move(title))),
      m_folder(std::move(std::move(folder))),
      m_rootHistSvc(std::move(rootHistSvc)) {}

    std::map<std::string, TH1D* > histoMap;
    std::map<std::string, TH2D* > histo2DMap;

    StatusCode initializePlots();
    void fill(const xAOD::Egamma& egamma);

  protected:
    std::string m_name;
    std::string m_title;
    std::string m_folder;
    SmartIF<ITHistSvc> m_rootHistSvc;

  };

}

#endif
