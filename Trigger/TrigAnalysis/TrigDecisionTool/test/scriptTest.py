#
#    Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#

#
#   Author: Will Buttinger
#
#   This is a demonstration of how to use a TrigDecisionTool in a simple 'standalone' python script.
#   You can run this after setting up AthAnalysis or Athena with:
#      python path/to/scriptTest.py
#
#

import ROOT

evt = ROOT.POOL.TEvent(ROOT.POOL.TEvent.kClassAccess) # fast xAOD read-mode
evt.readFrom("$ASG_TEST_FILE_MC")                     # can replace with a path to an xAOD

tdt = ROOT.ToolHandle[ROOT.Trig.TrigDecisionTool]("Trig::TrigDecisionTool/TrigDecisionTool")
ROOT.AAH.setProperty(tdt,"OutputLevel",2)             # example of setting a property of the tool

tdt.retrieve()                                        # initializes the tool

testPassed = True

for entry in range(10):
    evt.getEntry(entry)

    # can access eventStore objects like this:
    # ei = evt.retrieve("xAOD::EventInfo","EventInfo")
    # print(ei.eventNumber())

    # in this demo we just count the number of triggers passed
    nPassed = 0
    for trig in tdt.getListOfTriggers():
        if tdt.isPassed(trig): nPassed+=1
    print("Passed triggers of evt",entry,":",nPassed)

    if nPassed==0: testPassed=False # fail this test if any events do not pass any triggers



# the lines below are just here because this macro is run as a unittest, they are not necessary to copy
ROOT.gInterpreter.ProcessLine("#include \"xAODRootAccess/tools/TFileAccessTracer.h\"")
ROOT.gInterpreter.ProcessLine("xAOD::TFileAccessTracer::instance().enableDataSubmission(false);") # disable file reporting in unittest
exit(0 if testPassed else 1) # this is just so the macro can be used as a unit test
