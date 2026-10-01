/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 minimal gaudi initialization for AthenaServices unit testing
 ------------------------------------------------------------
 ATLAS Collaboration
 ***************************************************************************/

#include "TestTools/initGaudi.h"

#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IAppMgrUI.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/ModuleIncident.h"
#include "GaudiKernel/PathResolver.h"
#include <print>


namespace Athena_test {

  bool initGaudi(ISvcLocator*& pSvcLoc) {
    return initGaudi({}, pSvcLoc);
  }


  bool initGaudi(const std::string& jobOptsFile, ISvcLocator*& pSvcLoc) {

    std::string jobOptsPath;
    if (!jobOptsFile.empty()) {
      jobOptsPath = System::PathResolver::find_file(jobOptsFile, "JOBOPTSEARCHPATH");
      if (jobOptsPath.empty()) {
        std::println ("\n\nCannot find job opts {}", jobOptsFile);
      }
      else {
        std::println ("\n\nInitializing Gaudi ApplicationMgr using job opts {}", jobOptsPath);
      }
    }

    // Create an instance of ApplicationMgr
    SmartIF<IAppMgrUI> appMgr = Gaudi::createApplicationMgr();
    if(!appMgr.isValid()) {
      std::println ("Fatal error while creating the ApplicationMgr ");
      return false;
    }

    SmartIF<IProperty> propMgr(appMgr);
    SmartIF<ISvcLocator> svcLoc(appMgr);
    if(!svcLoc.isValid() || !propMgr.isValid()) {
      std::println ("Fatal error while retrieving AppMgr interfaces ");
      return false;
    }

    // Return pointer to ISvcLocator
    pSvcLoc = svcLoc.get();

    propMgr->setProperty( "EvtSel", "NONE" ).
      orThrow("Cannnot set EvtSel property", "initGaudi");
    if (jobOptsFile.empty()) {
      propMgr->setProperty( "JobOptionsType", "NONE" ).
        orThrow("Cannnot set JobOptionsType property", "initGaudi");
    } else {
      propMgr->setProperty( "JobOptionsType", "FILE" ).
        orThrow("Cannnot set JobOptionsType property", "initGaudi");
      propMgr->setProperty( "JobOptionsPath", jobOptsPath ).
        orThrow("Cannnot set JobOptionsPath property", "initGaudi");
    }

    if (appMgr->configure().isSuccess() && appMgr->initialize().isSuccess()) {
      std::println ("ApplicationMgr Ready");
      return true;
    } else {
      std::println ("Fatal error while initializing the AppMgr");
      return false;
    }
  }


  // This is for the benefit of the tests in CLIDComps.
  // By default, a dynamic symbol is not exported from a binary unless
  // it's referenced by another library with which we link.
  // A test in CLIDComps creates a ModuleLoadedIncident object in the binary
  // and then passes it to code in CLIDComps.  If ubsan is running,
  // then it will try to verify that this object derives from ModuleIncident.
  // But if the ModuleIncident type_info is not exported from the binary,
  // then the type_info used by the library will be different from that
  // in the binary, causing an ubsan warning.
  // But this test gets linked against TestTools, so we can avoid the warning
  // by referencing the type information here.
  // (An alternative would be to link the binary with --export-dynamic,
  // but that's not portable, so we don't want to put it in a package cmake.)
  void referenceGaudiSymbols()
  {
    ModuleLoadedIncident inc2 ("", "");
  }


  InitGaudi::InitGaudi(const std::string& jobOptsFile) {
    ISvcLocator* pSvcLoc{};
    if (!Athena_test::initGaudi(jobOptsFile, pSvcLoc)) {
      throw std::runtime_error("Cannot initialize Gaudi");
      }
    svcLoc = pSvcLoc;
  }

  InitGaudi::~InitGaudi() {
    auto appMgr = svcLoc.as<IAppMgrUI>();
    appMgr->finalize().ignore();
    appMgr->terminate().ignore();
  }

}
