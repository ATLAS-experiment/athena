#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

import subprocess
import re
import ROOT as R
from subprocess import Popen, PIPE
from array import array
import pandas as pd
import math
import time

narrowspace="#kern[-0.8]{ }"
plotlabel = {}
plotlabel["Zee"] = "Z"+narrowspace+"#rightarrow"+narrowspace+"ee"
plotlabel["Zmumu"] = "Z"+narrowspace+"#rightarrow"+narrowspace+"#mu#mu"
plotlabel["Zll"] = "Z"+narrowspace+"#rightarrow"+narrowspace+"ll"
Leemumuratiolabel = "L_{"+plotlabel["Zee"]+"} / L_{"+plotlabel["Zmumu"]+"}"

# global run livetime cut in seconds, for paper 40min, reduce to 10min for now
runlivetimecut = 10*60 # in seconds
# global lumiblock livetime cut in seconds, 9sec for now
lblivetimecut = 9 # in seconds

def get_grl(year, verbose=True):
    '''
    Get a list of runs for a year from the baseline GRL
    '''
    CVMFS = "/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/GoodRunsLists"
    grl = {}
    grl["15"] = CVMFS + "/data15_13TeV/20190708/data15_13TeV.periodAllYear_DetStatus-v105-pro22-13_Unknown_PHYS_StandardGRL_All_Good_25ns.xml"
    grl["16"] = CVMFS + "/data16_13TeV/20190708/data16_13TeV.periodAllYear_DetStatus-v105-pro22-13_Unknown_PHYS_StandardGRL_All_Good_25ns_WITH_IGNORES.xml" 
    grl["17"] = CVMFS + "/data17_13TeV/20190708/data17_13TeV.periodAllYear_DetStatus-v105-pro22-13_Unknown_PHYS_StandardGRL_All_Good_25ns_Triggerno17e33prim.xml" 
    grl["18"] = CVMFS + "/data18_13TeV/20190708/data18_13TeV.periodAllYear_DetStatus-v105-pro22-13_Unknown_PHYS_StandardGRL_All_Good_25ns_Triggerno17e33prim.xml" 
    grl["22"] = CVMFS + "/data22_13p6TeV/20230207/data22_13p6TeV.periodAllYear_DetStatus-v109-pro28-04_MERGED_PHYS_StandardGRL_All_Good_25ns.xml"
    grl["23"] = CVMFS + "/data23_13p6TeV/20230828/data23_13p6TeV.periodAllYear_DetStatus-v110-pro31-06_MERGED_PHYS_StandardGRL_All_Good_25ns.xml"
    grl["24"] = CVMFS + "/data24_13p6TeV/20241118/data24_13p6TeV.periodsEtoO_DetStatus-v130-pro36-08_MERGED_PHYS_StandardGRL_All_Good_25ns.xml"
    grl["25"] = "/eos/atlas/atlascerngroupdisk/perf-lumi/Zcounting/Run3/MergedOutputs/data25_13p6TeV/latest_GRL.xml"

    if year != "25":
        pipe = Popen(["grep", "RunList", grl[year]], stdout=PIPE, stderr=PIPE)
        runs = re.sub("[^0-9,]", "", str(pipe.communicate()[0])).split(",")
   
    elif year == "25":
        # preliminary GRL does not include "RunList" field, concatenate <Run> fields
        runs=subprocess.check_output("grep '<Run>' "+grl[year]+" | tr -dc ' [:digit:] '", shell=True).decode('ascii').split()
    if verbose:
        print("20"+year+": list or runs =", runs)

    return runs

def setAtlasStyle():
    R.gROOT.SetStyle("Plain")

    # use plain black on white colors
    icol = 0
    R.gStyle.SetFrameBorderMode(icol)
    R.gStyle.SetFrameFillColor(icol)
    R.gStyle.SetCanvasBorderMode(icol)
    R.gStyle.SetCanvasColor(icol)
    R.gStyle.SetPadBorderMode(icol)
    R.gStyle.SetPadColor(icol)
    R.gStyle.SetStatColor(icol)

    R.gStyle.SetLineColor(R.kBlack)
    
    # set the paper & margin sizes
    R.gStyle.SetPaperSize(20,26)

    # set margin sizes
    R.gStyle.SetPadTopMargin(0.05)
    R.gStyle.SetPadRightMargin(0.05)
    R.gStyle.SetPadBottomMargin(0.16)
    R.gStyle.SetPadLeftMargin(0.16)

    # set title offsets (for axis label)
    R.gStyle.SetTitleXOffset(1.4)
    R.gStyle.SetTitleYOffset(1.4)

    # use large fonts
    font=42 # Helvetica
    tsize=0.05
    R.gStyle.SetTextFont(font)
    R.gStyle.SetTextSize(tsize)
    R.gStyle.SetLegendFont(font)
    R.gStyle.SetLabelFont(font,"xyz")
    R.gStyle.SetTitleFont(font,"xyz")
  
    R.gStyle.SetLabelSize(tsize,"xyz")
    R.gStyle.SetTitleSize(tsize,"xyz")

    # use bold lines and markers
    R.gStyle.SetMarkerStyle(20)
    R.gStyle.SetMarkerSize(1.2)
    R.gStyle.SetLineStyleString(2,"[12 12]") # postscript dashes

    R.gStyle.SetEndErrorSize(0.)

    # do not display any of the standard histogram decorations
    R.gStyle.SetOptTitle(0)
    R.gStyle.SetOptStat(0)
    R.gStyle.SetOptFit(0)

    # put tick marks on top and RHS of plots
    R.gStyle.SetPadTickX(1)
    R.gStyle.SetPadTickY(1)
    R.gROOT.ForceStyle()

def drawAtlasLabel(x, y, text = "", color = R.kBlack):
    l = R.TLatex()
    l.SetNDC()
    l.SetTextFont(72)
    l.SetTextColor(color)
    
    delx = 0.115*696*R.gPad.GetWh()/(472*R.gPad.GetWw())

    l.DrawLatex(x,y,"ATLAS")
    if len(text) > 0:
        p = R.TLatex()
        p.SetNDC()
        p.SetTextFont(42)
        p.SetTextColor(color)
        p.DrawLatex(x+delx,y,text)

def drawText(x, y, text, size=27, color = R.kBlack):
    l = R.TLatex()
    l.SetNDC()
    if size > 0:
        l.SetTextSize(size)
    l.SetTextFont(43)
    l.SetTextColor(color)
    l.DrawLatex(x,y,text)

def make_bands(vec_in, stdev, yval):
    vec_y = array('d', [yval] * (len(vec_in) + 2))
    vec_x = array('d', sorted(vec_in))
    err_y = array('d', [stdev] * (len(vec_in) + 2))

    vec_x.insert(0, 0)
    vec_x.append(9999999999)   
  
    line = R.TGraphErrors(len(vec_x), vec_x, vec_y, R.nullptr, err_y)
    line.SetFillColorAlpha(8, 0.35)
    line.SetFillStyle(4050)

    return line

def get_year(run):
    run=int(run)
    if run >= 495667: return "25"
    elif run >= 472553: return "24"
    elif run >= 450227: return "23"
    elif run >= 427394: return "22"
    elif run >= 348885: return "18"
    elif run >= 325713: return "17"
    elif run >= 297730: return "16"
    elif run >= 276262: return "15"
    else:
        print("ERROR: Cannot classify run", run)
        exit(1)

def get_yearsqrtstxt(run_number):
    yearsqrtstxt = "Data 20" + get_year(run_number) + ",#kern[-0.5]{ }"
    if run_number < 427394:
        yearsqrtstxt += "#sqrt{s} = 13 TeV"
    else:
        yearsqrtstxt += "#sqrt{s} = 13.6 TeV"
    return yearsqrtstxt
    
def get_dfz(basedir, year, run, channel, standardcuts = True):
    '''
    Standard retrieval of Z counting Panda dataframe from CSV
    '''
    if year=="run3" or len(year.split("_")) > 1:
        mydir = basedir + "data"+get_year(run)+"_13p6TeV/physics_Main/"
    elif year=="run2":
        mydir = basedir + "data"+get_year(run)+"_13TeV/physics_Main/"
    elif int(year) >= 22:
        mydir = basedir + "data" + year + "_13p6TeV/physics_Main/"
    else:
        mydir = basedir + "data" + year + "_13TeV/physics_Main/" # untested

    try:
        dfz = pd.read_csv(mydir + "run_" + run + ".csv")
    except FileNotFoundError:
        print("WARNING: CVS for run", run, "not found, will skip.")
        return -1., 0., 0., 0., 0., None
        
    dfz_small = dfz

    if channel is not None:
        # reading a specific channel and calculating some integrated quantities
        dfz_small['ZLumi'] = dfz_small[channel + 'Lumi']
        dfz_small['ZLumiErr'] = dfz_small[channel + 'LumiErr']
        if standardcuts:
            dfz_small = dfz_small.drop(dfz_small[dfz_small.ZLumi == 0].index)
            dfz_small = dfz_small.drop(dfz_small[(dfz_small['LBLive']<lblivetimecut) | (dfz_small['PassGRL']==0)].index)

        dfz_small['ZLumi'] *= dfz_small['LBLive']
        dfz_small['ZLumiErr'] *= dfz_small['LBLive']
        zlumi = dfz_small['ZLumi'].sum()

        dfz_small['ZLumiErr'] *= dfz_small['ZLumiErr']
        zerr = math.sqrt(dfz_small['ZLumiErr'].sum())
    else:
        # reading both Zee and Zll, some preparatory calculations, but nothing final
        if standardcuts:
            dfz_small = dfz_small.drop(dfz_small[(dfz_small.ZeeLumi == 0) | (dfz_small.ZmumuLumi == 0)].index)
            dfz_small = dfz_small.drop(dfz_small[(dfz_small['LBLive']<lblivetimecut) | (dfz_small['PassGRL']==0)].index)
        dfz_small['ZeeLumi']    *= dfz_small['LBLive']
        dfz_small['ZeeLumiErr'] *= dfz_small['LBLive']
        dfz_small['ZeeLumiErr'] *= dfz_small['ZeeLumiErr']
        dfz_small['ZmumuLumi']    *= dfz_small['LBLive']
        dfz_small['ZmumuLumiErr'] *= dfz_small['LBLive']
        dfz_small['ZmumuLumiErr'] *= dfz_small['ZmumuLumiErr']
        zlumi, zerr = 0., 0.
        
    livetime = dfz_small['LBLive'].sum()
        
    # Calculate integrated ATLAS luminosity
    dfz_small['OffLumi'] *= dfz_small['LBLive']
    olumi = dfz_small['OffLumi'].sum()

    # Grab start of the run for plotting later on
    try:
        run_start = dfz_small['LBStart'].iloc[0]
        timestamp = time.gmtime(run_start)
        timestamp = R.TDatime(timestamp[0], timestamp[1], timestamp[2], timestamp[3], timestamp[4], timestamp[5])
        timestamp = timestamp.Convert()
    except IndexError:
        timestamp = 0
    
    return livetime, zlumi, zerr, olumi, timestamp, dfz_small


def local_fit(tg, start, end, year):
    """
    Fit over a sub-range of the data and print the mean and chi^2/NDF. 
    Useful to test the remaining trends after the global Run-3 normalisation.
    """

    tg.Fit('pol0', 'Rq0','0', start, end)
    mean = tg.GetFunction('pol0').GetParameter(0)
    chi2 = tg.GetFunction('pol0').GetChisquare()
    ndf  = tg.GetFunction('pol0').GetNDF()
    print("|", year, "|", round(mean,3), "|", round(chi2/ndf, 2), "|")
