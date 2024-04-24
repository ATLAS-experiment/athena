# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from PixelCalibAlgs.Recovery import ReadDbFile
from PixelCalibAlgs.EvoMonitoring import ReadCSV
from matplotlib.backends.backend_agg import FigureCanvasAgg
from matplotlib.figure import Figure
import numpy as np


def CalculateTOT(Q,params):
    num = params[1] + Q
    den = params[2] + Q
    if den == 0:
        return 0
    return params[0]*(num/den)

def CheckThresholds(calib):
    
    import os
    os.makedirs("plots/values", exist_ok=True)
    
    mapping = ReadCSV()
    
    expectedTOTint = { "Blayer": {"normal":[],"long":[],"ganged":[]},
                    "L1"    : {"normal":[],"long":[],"ganged":[]},
                    "L2"    : {"normal":[],"long":[],"ganged":[]},
                    "Disk"  : {"normal":[],"long":[],"ganged":[]}}
    CalibThreshold = { "Blayer": {"normal":[],"long":[],"ganged":[]},
                    "L1"    : {"normal":[],"long":[],"ganged":[]},
                    "L2"    : {"normal":[],"long":[],"ganged":[]},
                    "Disk"  : {"normal":[],"long":[],"ganged":[]}}
    CalibRMS       = { "Blayer": {"normal":[],"long":[],"ganged":[]},
                    "L1"    : {"normal":[],"long":[],"ganged":[]},
                    "L2"    : {"normal":[],"long":[],"ganged":[]},
                    "Disk"  : {"normal":[],"long":[],"ganged":[]}}
    CalibNoise     = { "Blayer": {"normal":[],"long":[],"ganged":[]},
                    "L1"    : {"normal":[],"long":[],"ganged":[]},
                    "L2"    : {"normal":[],"long":[],"ganged":[]},
                    "Disk"  : {"normal":[],"long":[],"ganged":[]}}
    CalibIntime    = { "Blayer": {"normal":[],"long":[],"ganged":[]},
                    "L1"    : {"normal":[],"long":[],"ganged":[]},
                    "L2"    : {"normal":[],"long":[],"ganged":[]},
                    "Disk"  : {"normal":[],"long":[],"ganged":[]}}
    
    
    for mod, FEs in calib.items():
        mod_name = mapping[str(mod)]
        mod_layer = ""
        if mod_name.startswith("L0"): 
            mod_layer = "Blayer"
        elif mod_name.startswith("L1"): 
            mod_layer = "L1"
        elif mod_name.startswith("L2"): 
            mod_layer = "L2"
        elif mod_name.startswith("D"): 
            mod_layer = "Disk"
        else:
            mod_layer = "IBL"
            continue
            if mod_name.startswith("LI_S15"): 
                continue        
        
        for fe in FEs:
            totint_nor = CalculateTOT(fe[3],fe[12:15])
            totint_lon = CalculateTOT(fe[7],fe[15:18])
            totint_gan = CalculateTOT(fe[11],fe[15:18])
            
            expectedTOTint[mod_layer]["normal"].append(totint_nor)
            expectedTOTint[mod_layer]["long"].append(totint_lon)
            expectedTOTint[mod_layer]["ganged"].append(totint_gan)
            
            CalibThreshold[mod_layer]["normal"].append(fe[0])
            CalibThreshold[mod_layer]["long"].append(fe[4])
            CalibThreshold[mod_layer]["ganged"].append(fe[8])
            
            CalibRMS[mod_layer]["normal"].append(fe[1])
            CalibRMS[mod_layer]["long"].append(fe[5])
            CalibRMS[mod_layer]["ganged"].append(fe[9])
            
            CalibNoise[mod_layer]["normal"].append(fe[2])
            CalibNoise[mod_layer]["long"].append(fe[6])
            CalibNoise[mod_layer]["ganged"].append(fe[10])
            
            CalibIntime[mod_layer]["normal"].append(fe[3])
            CalibIntime[mod_layer]["long"].append(fe[7])
            CalibIntime[mod_layer]["ganged"].append(fe[11])
        
    
    
    print("\n Threshold values validation:")
    print("-"*40)
    for i in ["Blayer","L1","L2","Disk"]:
        
        for j in CalibThreshold[i]:
            ValThreshold(i,j,CalibThreshold[i][j])
        figur(i,expectedTOTint, "ToT for intime threshold", "totIntime_"+i+".png")
        figur(i,CalibThreshold, "Threshold","CalibThreshold_"+i+".png")
        figur(i,CalibRMS      , "RMS"      ,"CalibRMS_"+i+".png")
        figur(i,CalibNoise    , "Noise"    ,"CalibNoise_"+i+".png")
        figur(i,CalibIntime   , "Intime"   ,"CalibIntime_"+i+".png")
        
    print("-"*40,"\n")

def ValThreshold (layer, pix, list):
    
    # Those values are coming from online crew - Analog thresholds
    # https://twiki.cern.ch/twiki/bin/viewauth/Atlas/PixelConditionsRUN3
    realThresholds = { "Blayer": 4700 , "L1": 4300, "L2": 4300, "Disk": 4300} 
       
    listavg = np.average(list)
    dev = (realThresholds[layer]-listavg)/listavg*100
    status = "OK"
    
    # If it deviates more than 1%
    if dev > 1:
        status = "NEEDS CHECKING!!!"
    
    print("%-25s: %6.1fe (exp.: %4ue), dev.: %6.2f%% - status (<1%%): %s" % (layer+" avg. thr. ["+pix+"]", listavg, realThresholds[layer], dev, status))

def figur(title,hist,xlabel,namef):
    fig = Figure(figsize=(13,10))
    fig.suptitle(title)
    plot(fig.add_subplot(2,3,1),hist[title]["normal"], xlabel+" - normal")
    plot(fig.add_subplot(2,3,2),hist[title]["long"]  , xlabel+" - long")
    plot(fig.add_subplot(2,3,3),hist[title]["ganged"], xlabel+" - ganged")
    plot(fig.add_subplot(2,3,4),hist[title]["normal"], xlabel+" - normal" ,True)
    plot(fig.add_subplot(2,3,5),hist[title]["long"]  , xlabel+" - long"   ,True)
    plot(fig.add_subplot(2,3,6),hist[title]["ganged"], xlabel+" - ganged" ,True)
    FigureCanvasAgg(fig).print_figure("plots/values/"+namef, dpi=150)

def plot(axs,arr, xlabel, islog = False):
    axs.hist(arr, bins=60)
    axs.set_xlabel(xlabel)
    if islog:
        axs.set_yscale("log")
    axs.set_ylabel("Counts [log scale]" if islog else "Counts")    
    
    
if __name__ == "__main__":
    
    # Used for testing and checking performance
    old_calib, old_iov = ReadDbFile("PixelChargeCalibration-DATA-RUN2-UPD4-26.log")
    CheckThresholds(old_calib)