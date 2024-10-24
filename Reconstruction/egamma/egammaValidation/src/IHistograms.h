/*
Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EGAMMAVALIDATION_BASICHISTOGRAMS_H
#define EGAMMAVALIDATION_BASICHISTOGRAMS_H

#include "GaudiKernel/ITHistSvc.h"
#include "GaudiKernel/SmartIF.h"

#include "xAODBase/IParticle.h"

#include <map>
#include <string>

class StatusCode;
class TH1D;

namespace egammaMonitoring {

  class IHistograms {

  public:

    IHistograms();
    IHistograms(std::string name,
		std::string title,
		std::string folder,
		SmartIF<ITHistSvc> rootHistSvc
		) :
      m_name(std::move(std::move(name))),
      m_title(std::move(std::move(title))),
      m_folder(std::move(std::move(folder))),
      m_rootHistSvc(std::move(rootHistSvc)) {}

    virtual StatusCode initializePlots() = 0;

    virtual void fill(const xAOD::IParticle&) = 0;
    virtual void fill(const xAOD::IParticle&, float mu) = 0;
    virtual ~IHistograms(){};

    std::map<std::string, TH1D*> histoMap;

  protected:
    std::string m_name;
    std::string m_title;
    std::string m_folder;
    SmartIF<ITHistSvc> m_rootHistSvc;

  };

}


#endif //EGAMMAVALIDATION_BASICHISTOGRAM_H
