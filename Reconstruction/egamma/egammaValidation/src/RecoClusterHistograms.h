/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EGAMMAVALIDATION_RECOCLUSTERSHISTOGRAMS_H
#define EGAMMAVALIDATION_RECOCLUSTERSHISTOGRAMS_H

#include <map>

#include "GaudiKernel/ITHistSvc.h"
#include "GaudiKernel/SmartIF.h"

#include "xAODEgamma/Egamma.h"

class TH3D;

namespace egammaMonitoring{

  class RecoClusterHistograms{
  public:
    
    // Histos
    RecoClusterHistograms(std::string name,
			  std::string title,
			  std::string folder,
			  SmartIF<ITHistSvc> rootHistSvc) :
      m_name(std::move(name)),
      m_title(std::move(title)),
      m_folder(std::move(folder)),
      m_rootHistSvc(std::move(rootHistSvc)) {}

    std::map<std::string, TH3D*> m_histo3DMap;

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
