#!/usr/bin/env python
# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

import sys
import ROOT
import argparse
import subprocess
from array import array

def checkxAODSize(xAODFiles, histfile, addgraphlabel=False):

    # Set output HIST filename
    histfilename = histfile if histfile else "hist.root"

    # Loop over xAODFiles and extract Total size/event in kB with checkxAOD
    sizes = []
    for filename in xAODFiles:
        cmd = ['checkxAOD', filename]
        outlog = subprocess.Popen(cmd, stdout = subprocess.PIPE)
        lines = str(outlog.communicate()).split("\\n")
        # Set some variable to start values
        iline = 0
        for line in lines:
            # Parse CSV lines with domain sizes
            if line.startswith("CSV"):
                domainlist = lines[iline+1].strip().split(",")
                domainsize = lines[iline+2].strip().split(",")
                domaindict = dict(zip(domainlist, domainsize))
            iline = iline + 1
        sizes.append(float(domaindict['Total']))
            
    # Open existing histogram file and add size/event infos
    hfile = ROOT.TFile( histfilename, 'UPDATE', 'ROOT file with histograms' )
    hfile.cd()
    hfile.mkdir("Sizes")
    hfile.cd("Sizes")

    # Histogram with "Total" checkxAOD sizes
    hsize = ROOT.TH1F('hist PHYSLITE PHYS AOD event size', 'hist PHYSLITE PHYS AOD event size', len(sizes), -0.5, len(sizes)-0.5 )
    for i in range(0,len(sizes)):
      hsize.Fill(i, sizes[i])
    hsize.Write()

    # The same "Total" checkxAOD size in a TGraph
    xbins = [*range(len(sizes))]
    x, y = array( 'd' ), array( 'd' )
    for i in range(len(sizes)):
        x.append(xbins[i])
        y.append(sizes[i])

    grsize = ROOT.TGraph(len(sizes),x,y)
    grsize.SetTitle('graph PHYSLITE PHYS AOD file size')
    grsize.SetName('graph PHYSLITE PHYS AOD file size')

    # Add xaxis label names to TGraph
    if addgraphlabel:
        xbinsLabel = [ 'PHYSLITE %3.1f'%sizes[0],
                       'PHYS %3.1f'%sizes[1],
                       'AOD %3.1f'%sizes[2] ]

        graxis = grsize.GetXaxis()
        for ibin in range(0,len(sizes)):
            tmpbin = graxis.FindBin(ibin)
            graxis.SetBinLabel(tmpbin, xbinsLabel[ibin])
        graxis.Draw("AP")
    
    grsize.Write()
    hfile.Close()

    return

def main():
    parser = argparse.ArgumentParser(
        description="Extracts a few basic quantities from the xAOD file and dumps them into a hist ROOT file")
    parser.add_argument("--xAODFiles", help="xAOD filenames (comma separated) for size/event check", 
                        action="store", default=None)
    parser.add_argument("--outputHISTFile", help="histogram output filename",
                        action="store", default=None)
    parser.add_argument("--addgraphlabel", help="Add PHYSLITE PHYS AOD x-Axis graph label and size",
                        action="store_true", default=False)

    args = parser.parse_args()

    if len(sys.argv) < 2:
        parser.print_help()
        sys.exit(1)

    # Create input filelist
    filelist = args.xAODFiles.split(',')

    # Call checkxAOD 
    checkxAODSize(filelist, args.outputHISTFile, args.addgraphlabel)

    return 0

if __name__ == "__main__":

    main()
