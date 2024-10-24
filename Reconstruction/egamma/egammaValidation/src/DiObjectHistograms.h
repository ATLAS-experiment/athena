/*
Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EGAMMAVALIDATION_DIOBJECTHISTOGRAMS_H
#define EGAMMAVALIDATION_DIOBJECTHISTOGRAMS_H

#include "AsgMessaging/StatusCode.h"
#include "GaudiKernel/ITHistSvc.h"
#include "GaudiKernel/SmartIF.h"

#include <map>
#include <string>

class TH1;
class ITHistSvc;
namespace xAOD {
  class IParticle; 
}

namespace egammaMonitoring {
  
  class DiObjectHistograms {
  public:

    DiObjectHistograms(std::string name,
		       std::string title,
		       std::string folder,
		       SmartIF<ITHistSvc> rootHistSvc) :
      m_name(std::move(name)),
      m_title(std::move(title)),
      m_folder(std::move(folder)),
      m_rootHistSvc(std::move(rootHistSvc)) {}
    
    StatusCode initializePlots();
    
    void fill(const xAOD::IParticle& eg1, const xAOD::IParticle& eg2);
    void fill(const xAOD::IParticle& eg1, const xAOD::IParticle& eg2, float mu);
    std::map<std::string, TH1*> histoMap;

  protected:
    std::string m_name;
    std::string m_title;
    std::string m_folder;
    SmartIF<ITHistSvc> m_rootHistSvc;
  };
  
}

#endif
