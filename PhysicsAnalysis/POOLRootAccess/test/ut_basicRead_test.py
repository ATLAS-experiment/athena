# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#This is an example of reading a POOL file in PyROOT

import ROOT
evt = ROOT.POOL.TEvent()
#evt = ROOT.POOL.TEvent(ROOT.POOL.TEvent.kClassAccess) #for faster xAOD reading

evt.readFrom("$ASG_TEST_FILE_MC")

for i in range(0,10): #use evt.getEntries() to read all
    evt.getEntry(i)

    #normal retrieve method types a 'type' and 'key'
    ei = evt.retrieve("xAOD::EventInfo","EventInfo")
    print(ei.eventNumber())

    #possible (but slower) to retrieve just by 'key'
    els = evt.get_item("TruthElectrons")
    for el in els: ROOT.xAOD.dump(el)
