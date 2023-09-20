#!/usr/bin/env python3
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

import numpy as np
import pandas as pd
import ROOT as R
import python_tools as pt
import os
import math
from array import array
import argparse
import re

def main():
    channel = "Zee"
    #channel = "Zmumu"
    
    parser = argparse.ArgumentParser()
    parser.add_argument('--infile', type=str, help='Input root file')
    parser.add_argument('--outfile', type=str, help='Output pdf file')

    args    = parser.parse_args()
    infile    = args.infile
    outfile = args.outfile

    Z_Peak = R.TFile.Open(infile, "READ")
    if channel == "Zee":
        Z_Peak_Hist = Z_Peak.Get("run_452202/GLOBAL/DQTGlobalWZFinder/m_Z_mass_opsele")
    if channel == "Zmumu":
        Z_Peak_Hist = Z_Peak.Get("run_452202/GLOBAL/DQTGlobalWZFinder/m_Z_mass_opsmu")

    c1 = R.TCanvas()
    c1.cd()
    
    Z_Peak_Hist.SetMarkerStyle(20)
    Z_Peak_Hist.SetMarkerSize(0.7)
    Z_Peak_Hist.GetYaxis().SetTitle("Events")
    Z_Peak_Hist.GetYaxis().SetRangeUser(0, 46000)
    Z_Peak_Hist.GetYaxis().SetTitleOffset(1.5)
    if channel == "Zee":
        Z_Peak_Hist.GetXaxis().SetTitle("m_{ee} [GeV]")
    if channel == "Zmumu":
        Z_Peak_Hist.GetXaxis().SetTitle("m_{#mu#mu} [GeV]")
    R.gStyle.SetOptStat(0)
    Z_Peak_Hist.Draw("P")

    l = R.TLatex()
    l.SetNDC()
    l.SetTextFont(72)
    l.DrawLatex(0.16,0.86,"#scale[0.8]{ATLAS}")
     
    p = R.TLatex()
    p.SetNDC()
    p.SetTextFont(42)
    p.DrawLatex(0.26,0.86,"#scale[0.8]{Work In Progress}")
    p.DrawLatex(0.16, 0.80, "#scale[0.8]{2023, #sqrt{s} = 13.6 TeV}")
    p.DrawLatex(0.16, 0.74, "LHC Fill 8773")
    if channel == "Zee":
        p.DrawLatex(0.16, 0.68, "Z #rightarrow ee")
    if channel == "Zmumu":
        p.DrawLatex(0.16, 0.68, "Z #rightarrow #mu#mu")

    c1.Modified()
    c1.SaveAs(outfile+".pdf")

if __name__ == "__main__":
    main()
