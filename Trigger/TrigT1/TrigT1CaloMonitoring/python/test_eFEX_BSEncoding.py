#!/usr/bin/env python

#
#  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#


# this test we will run the main job once to produce a raw file, then 
# run it a second time to process that raw file
# then compare the monitoring histograms


import tempfile
import os
import ROOT

testFile = "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrigP1Test/data24_13p6TeV.00475321.physics_Main.daq.RAW._lb0247._SFO-11._0006.data_150evt"
#testFile = "/eos/atlas/atlascerngroupdisk/det-l1calo/OfflineSoftware/TestFiles/data24_13p6TeV/data24_13p6TeV.00477048.physics_Main.daq.RAW._lb0821._SFO-20._0001.data"

def runMonCmd(inputFile,outputBSFile=None,outputMonFile=None):
    cmdString = f"l1calo-ath-mon --evtMax 10 --filesInput {inputFile}" # -- Trigger.L1.doeFex=True"  # need topo atm for TrigCompositeContainer to trigger bs encoder
    if outputMonFile: cmdString += f" Output.HISTFileName={outputMonFile}"
    if outputBSFile: 
       if os.path.exists(outputBSFile): os.remove(outputBSFile)
       cmdString += f" Output.BSFileName=\"{outputBSFile}\""
    print("Executing: " + cmdString)
    os.system(cmdString)


def histosEqual(h1, h2, tolerance=1e-3):
   for bin in range(h1.GetNcells()):
      if( abs(h1.GetBinContent(bin)-h2.GetBinContent(bin))>tolerance ): 
         print(f"Bin {bin} difference: {h1.GetBinContent(bin)} vs {h2.GetBinContent(bin)}")
         return False
   return True


with tempfile.TemporaryDirectory() as tmp:
  bsFile = "test.efex.raw" # os.path.join(tmp, 'test.efex.raw')
  runMonCmd(inputFile = testFile,
          outputBSFile=bsFile,
          outputMonFile="monitoring.orig.root")
  runMonCmd(inputFile=bsFile,outputMonFile="monitoring.root")


# now c.f. the relevant histograms from the two monitoring files
f1 = ROOT.TFile("monitoring.orig.root")
f2 = ROOT.TFile("monitoring.root")
hists = ["h_L1_eEMRoI_LowPtCut_EtaPhiMap","h_L1_eTauRoI_LowPtCut_EtaPhiMap","h_L1_eEMxRoI_LowPtCut_EtaPhiMap","h_L1_eTauxRoI_LowPtCut_EtaPhiMap"]
for h in hists:
   h1 = f1.FindObjectAny(h)
   if not h1:
      print(f"Missing {h} from {f1.GetName()}")
      exit(1)
   h2 = f2.FindObjectAny(h)
   if not h2:
      print(f"Missing {h} from {f2.GetName()}")
      exit(1)
   if not histosEqual(h1,h2):
      print(f"{h} histograms differ")
      exit(1)