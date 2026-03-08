/*
    Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
   Author: Will Buttinger

   This is a demonstration of how to use a TrigDecisionTool in a simple 'standalone' macro.
   You can run this after setting up AthAnalysis or Athena with:
      root path/to/macroTest.cxx

 */

{
  POOL::TEvent evt(POOL::TEvent::kClassAccess); // fast xAOD read-mode
  evt.readFrom("$ASG_TEST_FILE_MC");            // can replace with a path to an xAOD

  ToolHandle<Trig::TrigDecisionTool> tdt("Trig::TrigDecisionTool/TrigDecisionTool");
  AAH::setProperty(tdt,"OutputLevel",2);        // example of setting a property of the tool

  tdt.retrieve();                               // initializes the tool

  bool testPassed = true;

  for(size_t entry = 0; entry < 10; entry++) {
      evt.getEntry(entry);

      // can access eventStore objects like this:
      // const xAOD::EventInfo* ei = nullptr;
      // evt.retrieve(ei,"EventInfo");
      // std::cout << ei->eventNumber() << std::endl;

      // in this demo we just count the number of triggers passed
      size_t nPassed = 0;
      for(auto trig : tdt->getListOfTriggers()) if(tdt->isPassed(trig)) nPassed++;
      std::cout << "Passed triggers of evt " << entry << " : " << nPassed << std::endl;

      if(nPassed==0) testPassed=false; // fail this test if any events do not pass any triggers
  }


  // the lines below are just here because this macro is run as a unittest, they are not necessary to copy
  gInterpreter->ProcessLine("#include \"xAODRootAccess/tools/TFileAccessTracer.h\"");
  gInterpreter->ProcessLine("xAOD::TFileAccessTracer::instance().enableDataSubmission(false);"); // disable file reporting in unittest
  exit(testPassed? 0 : 1); // this is just so the macro can be used as a unit test

}
