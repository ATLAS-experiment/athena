// -*- C++ -*-

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHENAPYTHON_ATHENAPYTHONDICT_H
#define ATHENAPYTHON_ATHENAPYTHONDICT_H

#include <string>
#include <typeinfo>
#include "GaudiKernel/IEvtSelector.h"
#include "GaudiKernel/IClassIDSvc.h"
#include "GaudiKernel/ITHistSvc.h"
#include "AthenaKernel/IThinningHdlr.h"
#include "AthenaKernel/ISlimmingHdlr.h"
#include "AthenaKernel/IValgrindSvc.h"
#include "AthenaKernel/IDictLoaderSvc.h"
#include "AthenaKernel/IEvtIdModifierSvc.h"
#include "GaudiKernel/IIoComponent.h"
#include "GaudiKernel/IIoComponentMgr.h"

#include "AthenaBaseComps/AthAlgorithm.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaBaseComps/AthService.h"

#include "AthenaPython/PyAthenaAlg.h"
#include "AthenaPython/PyAthenaSvc.h"
#include "AthenaPython/PyAthenaTool.h"
#include "AthenaPython/PyAthenaAud.h"

namespace AthenaInternal {

  class ROOT6_AthenaPython_WorkAround_Dummy {};
  
  CLID getClid( IClassIDSvc* self, const std::string& typeName ) {
    CLID clid = CLID_NULL;
    self->getIDOfTypeName(typeName, clid).ignore();
    return clid;
  }

  std::pair<StatusCode, TH1*> getHist (ITHistSvc& svc,
                                       const std::string& name,
                                       size_t index = 0)
  {
    TH1* o = nullptr;
    StatusCode sc = svc.getHist (name, o, index);
    return std::make_pair (sc, o);
  }

  std::pair<StatusCode, TGraph*> getGraph (ITHistSvc& svc,
                                           const std::string& name)
  {
    TGraph* o = nullptr;
    StatusCode sc = svc.getGraph (name, o);
    return std::make_pair (sc, o);
  }

  std::pair<StatusCode, TEfficiency*> getEfficiency (ITHistSvc& svc,
                                                     const std::string& name)
  {
    TEfficiency* o = nullptr;
    StatusCode sc = svc.getEfficiency (name, o);
    return std::make_pair (sc, o);
  }

  std::pair<StatusCode, TTree*> getTree (ITHistSvc& svc,
                                         const std::string& name)
  {
    TTree* o = nullptr;
    StatusCode sc = svc.getTree (name, o);
    return std::make_pair (sc, o);
  }

} // namespace AthenaInternal

#endif // ATHENAPYTHON_ATHENAPYTHONDICT_H
