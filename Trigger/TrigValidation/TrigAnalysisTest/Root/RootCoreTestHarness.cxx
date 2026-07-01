/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//
// Code to run a test of a given name.
//

#include "TrigAnalysisTest/RootCoreTestHarness.h"

#include "TrigAnalysisTest/TestFactory.h"
#include <iostream>

using namespace TrigAnalysisTest;
using namespace std;


#ifdef ROOTCORE
#		include <xAODRootAccess/Event.h>
#		include <xAODRootAccess/Init.h>
#		include <xAODRootAccess/tools/ReturnCheck.h>
#               include <AsgTools/SgEvent.h>
#endif

#include "TrigConfxAOD/xAODConfigTool.h"
#include "TrigDecisionTool/TrigDecisionTool.h"

#include "TChain.h"
#include "TError.h"
#include "TFile.h"
#include "TH1F.h"
#include "TSystem.h"

using namespace std;
using namespace Trig;
using namespace TrigConf;
using namespace xAOD;

namespace TrigAnalysisTest {

  int runTrigAnalysisTest (const std::string &testName)
  {
    // Fetch the test to run it.
    auto test = GetTrigAnalysisTest(testName);
    if (test == nullptr) {
      cout << "Unable to load test" << endl;
      return 1;
    }

    // Initialize (as done for all xAOD standalone programs!)
    if (!xAOD::Init(testName.c_str()).isSuccess()) {
      return 1;
    }

    // Load up the proper file we should be checking against.
    std::unique_ptr< TFile > file( TFile::Open( gSystem->Getenv("ROOTCORE_TEST_FILE"), "READ" ) );


    // Init data access to the trigger
    auto event = xAOD::Event::createAndReadFrom(*file);
    if (!event) {
      cout << "cannot read from file: " << file << endl;
      return 1;
    }

    xAODConfigTool configTool("xAODConfigTool");
    ToolHandle<TrigConf::ITrigConfigTool> configHandle(&configTool);
    if (!configHandle->initialize().isSuccess()) return 1;
   
    TrigDecisionTool trigDecTool("TrigDecTool");
    if (!trigDecTool.setProperty("ConfigTool",configHandle)) return 1;
    if (!trigDecTool.setProperty("TrigDecisionKey","xTrigDecision")) return 1;
    if (!trigDecTool.initialize()) return 1;

		asg::SgEvent sgevent(event.get());
		test->setEventStore( &sgevent );

    // Run the files
    size_t nEntries = event->getEntries();
    for (size_t entry = 0; entry < nEntries; entry++) {
      event->getEntry(entry);

      test->processEvent (trigDecTool);
    }

    // Clean up
    return test->finalize();
  }
}
