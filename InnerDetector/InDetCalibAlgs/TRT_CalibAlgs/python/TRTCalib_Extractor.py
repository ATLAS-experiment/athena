# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#!/usr/bin/env python
import sys
from ROOT import TFile

class storeinfo:
    def __init__(self):
        self.res=0.
        self.tres=0.
        self.resmean=0.
        self.tresmean=0.

# Script to read and plot rts from a text file:
filenamein = sys.argv[1]
rts={}

# Here we read the text file for rts:
file = TFile(filenamein)
detectortuple = file.Get("Detector_Artuple")

entries = detectortuple.GetEntries()
info={}

#loop over the Argon entries for Barrel A and C
for i in range(entries):
        detectortuple.GetEntry(i)
        if abs(detectortuple.det) == 2:
            continue
        infoprev = storeinfo()
        
        infoprev.res  = detectortuple.res
        infoprev.tres = detectortuple.tres
        infoprev.resmean = detectortuple.resMean
        infoprev.tresmean = detectortuple.tresMean
        infoprev.hits = detectortuple.nhits
        det = detectortuple.det
        
        print ("For detector part %d res = %s\n" %(det,infoprev.res))
        print ("For detector part %d tres = %s\n" %(det,infoprev.tres))
        print ("For detector part %d resmean = %s\n" %(det,infoprev.resmean))
        print ("For detector part %d tresmean = %s\n" %(det,infoprev.tresmean))
        info[detectortuple.det, detectortuple.lay  ] = infoprev

detectortuple = file.Get("Detectortuple")
entries = detectortuple.GetEntries()

#loop over the Xenon entries for Endcap A and C
for i in range(entries):
        detectortuple.GetEntry(i)
        if abs(detectortuple.det) == 1:
            continue
        infoprev = storeinfo()
        
        infoprev.res  = detectortuple.res
        infoprev.tres = detectortuple.tres
        infoprev.resmean = detectortuple.resMean
        infoprev.tresmean = detectortuple.tresMean
        infoprev.hits = detectortuple.nhits
        det = detectortuple.det
        
        print ("For detector part %d res = %s\n" %(det,infoprev.res))
        print ("For detector part %d tres = %s\n" %(det,infoprev.tres))
        print ("For detector part %d resmean = %s\n" %(det,infoprev.resmean))
        print ("For detector part %d tresmean = %s\n" %(det,infoprev.tresmean))
        info[detectortuple.det, detectortuple.lay  ] = infoprev

print("Extracted data\n")

