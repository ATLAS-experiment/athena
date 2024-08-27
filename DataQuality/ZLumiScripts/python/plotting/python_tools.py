#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

import os
import subprocess
import re
import ROOT as R
from subprocess import Popen, PIPE
from array import array
import pandas as pd
import math
import time

plotlabel = {}
plotlabel["Zee"] = "Z #rightarrow ee"
plotlabel["Zmumu"] = "Z #rightarrow #mu#mu"
plotlabel["Zll"] = "Z #rightarrow ll"

# global livetime cut in seconds, typically 40min
livetimecut = 40*60 # in seconds

try:
    from AthenaCommon.Utils import unixtools
    R.gROOT.LoadMacro(unixtools.find_datafile('ZLumiScripts/AtlasStyle/AtlasStyle.C'))
    R.gROOT.LoadMacro(unixtools.find_datafile('ZLumiScripts/AtlasStyle/AtlasLabels.C'))
    R.gROOT.LoadMacro(unixtools.find_datafile('ZLumiScripts/AtlasStyle/AtlasUtils.C'))
except Exception:
    R.gROOT.LoadMacro(os.getcwd() + "/plotting/AtlasStyle/AtlasStyle.C")
    R.gROOT.LoadMacro(os.getcwd() + "/plotting/AtlasStyle/AtlasLabels.C")
    R.gROOT.LoadMacro(os.getcwd() + "/plotting/AtlasStyle/AtlasUtils.C")

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
    grl["24"] = "/eos/atlas/atlascerngroupdisk/perf-lumi/Zcounting/Run3/MergedOutputs/data24_13p6TeV/latest_GRL.xml"

    if year != "24":
        pipe = Popen(["grep", "RunList", grl[year]], stdout=PIPE, stderr=PIPE)
        runs = re.sub("[^0-9,]", "", str(pipe.communicate()[0])).split(",")
   
    elif year == "24":
        # preliminary GRL does not include "RunList" field, concatenate <Run> fields
        runs=subprocess.check_output("grep '<Run>' "+grl[year]+" | tr -dc ' [:digit:] '", shell=True).decode('ascii').split()
    if verbose:
        print("20"+year+": list or runs =", runs)

    return runs

def setAtlasStyle():
    R.SetAtlasStyle()

def drawAtlasLabel(x, y, text):
    R.ATLASLabel(x, y, text, 1)

def drawText(x, y, text, size=False):
    R.myText(x, y, 1, text, size)


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

def get_dfz(basedir, year, run, channel, standardcuts = True):
    '''
    Standard retrieval of Z counting Panda dataframe from CSV
    '''
    if year=="run3":
        if int(run) >= 472553: mydir = basedir + "data24_13p6TeV/physics_Main/"
        elif int(run) >= 450227: mydir = basedir + "data23_13p6TeV/physics_Main/"
        elif int(run) >= 427394: mydir = basedir + "data22_13p6TeV/physics_Main/"
    elif year=="run2":
        if int(run) >= 348885: mydir = basedir + "data18_13TeV/physics_Main/"
        elif int(run) >= 325713: mydir = basedir + "data17_13TeV/physics_Main/"
        elif int(run) >= 297730: mydir = basedir + "data16_13TeV/physics_Main/"
        elif int(run) >= 276262: mydir = basedir + "data15_13TeV/physics_Main/"
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
            dfz_small = dfz_small.drop(dfz_small[(dfz_small['LBLive']<10) | (dfz_small['PassGRL']==0)].index)

        dfz_small['ZLumi'] *= dfz_small['LBLive']
        dfz_small['ZLumiErr'] *= dfz_small['LBLive']
        zlumi = dfz_small['ZLumi'].sum()

        dfz_small['ZLumiErr'] *= dfz_small['ZLumiErr']
        zerr = math.sqrt(dfz_small['ZLumiErr'].sum())
    else:
        # reading both Zee and Zll, some preparatory calculations, but nothing final
        if standardcuts:
            dfz_small = dfz_small.drop(dfz_small[(dfz_small.ZeeLumi == 0) | (dfz_small.ZmumuLumi == 0)].index)
            dfz_small = dfz_small.drop(dfz_small[(dfz_small['LBLive']<10) | (dfz_small['PassGRL']==0)].index)
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
