# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from PixelCalibAlgs.EvoMonitoring import ReadCSV
from matplotlib.backends.backend_agg import FigureCanvasAgg
from matplotlib.figure import Figure
import numpy as np

MaxThreshold = 10

def CalculateTOT(Q,params):
    num = params[1] + Q
    den = params[2] + Q
    if den == 0:
        return 0
    return params[0]*(num/den)

def CheckThresholds(calib,iov,_file):
    
    import os
    os.makedirs("plots/parameters", exist_ok=True)
    
    mapping = ReadCSV()
    
    expectedTOTint = { "IBL": {"normal":[],"long":[]},
                    "Blayer": {"normal":[],"long":[],"ganged":[]},
                    "L1"    : {"normal":[],"long":[],"ganged":[]},
                    "L2"    : {"normal":[],"long":[],"ganged":[]},
                    "Disk"  : {"normal":[],"long":[],"ganged":[]}}
    CalibThreshold = { "IBL": {"normal":[],"long":[]},
                    "Blayer": {"normal":[],"long":[],"ganged":[]},
                    "L1"    : {"normal":[],"long":[],"ganged":[]},
                    "L2"    : {"normal":[],"long":[],"ganged":[]},
                    "Disk"  : {"normal":[],"long":[],"ganged":[]}}
    CalibRMS       = { "IBL": {"normal":[],"long":[]},
                    "Blayer": {"normal":[],"long":[],"ganged":[]},
                    "L1"    : {"normal":[],"long":[],"ganged":[]},
                    "L2"    : {"normal":[],"long":[],"ganged":[]},
                    "Disk"  : {"normal":[],"long":[],"ganged":[]}}
    CalibNoise     = { "IBL": {"normal":[],"long":[]},
                    "Blayer": {"normal":[],"long":[],"ganged":[]},
                    "L1"    : {"normal":[],"long":[],"ganged":[]},
                    "L2"    : {"normal":[],"long":[],"ganged":[]},
                    "Disk"  : {"normal":[],"long":[],"ganged":[]}}
    CalibIntime    = { "IBL": {"normal":[],"long":[]},
                    "Blayer": {"normal":[],"long":[],"ganged":[]},
                    "L1"    : {"normal":[],"long":[],"ganged":[]},
                    "L2"    : {"normal":[],"long":[],"ganged":[]},
                    "Disk"  : {"normal":[],"long":[],"ganged":[]}}
    
    report = {}
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
            if mod_name.startswith("LI_S15"): 
                continue  
        
        key = "%-16s - %s" % (mod_name,mod)
        if key not in report:
            report[key] = ""
            
        for ife in range(len(FEs)):
            fe = FEs[ife]
            if mod_layer == "IBL":
                
                bool1, str1 = ValThreshold(mod_layer,"normal",fe[0],MaxThreshold)
                bool2, str2 = ValThreshold(mod_layer,"long"  ,fe[2],MaxThreshold)
                if bool1:
                    report[key] += ("\tFE%02u: "% ife) + str1
                if bool2:
                    report[key] += ("\tFE%02u: "% ife) + str2 
                
                CalibThreshold[mod_layer]["normal"].append(fe[0])
                CalibThreshold[mod_layer]["long"].append(fe[2]) 
                
                CalibRMS[mod_layer]["normal"].append(fe[1])
                CalibRMS[mod_layer]["long"].append(fe[3])               
            else:
                
                bool1, str1 = ValThreshold(mod_layer,"normal",fe[0],MaxThreshold)
                bool2, str2 = ValThreshold(mod_layer,"long"  ,fe[4],MaxThreshold)
                bool3, str3 = ValThreshold(mod_layer,"ganged",fe[8],MaxThreshold)
                if bool1:
                    report[key] += ("\tFE%02u: "% ife) + str1
                if bool2:
                    report[key] += ("\tFE%02u: "% ife) + str2                 
                if bool3:
                    report[key] += ("\tFE%02u: "% ife) + str3                 
                
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
                
    # writing log files
    fout = open("CheckValues_log.txt", "w") 
    
    fout.write("_______________ LOG _______________\n\n" )
    fout.write("File chosen:\n" )
    fout.write("Calibration  : '%s'\n" % (_file) )
    fout.write("Data base IOV: %s\n\n" % (iov) )   
        
    fout.write("\n Threshold FE values validation:\n")
    fout.write("-"*40+"\n")
    for key, val in report.items():
        if val == "":
            continue
        fout.write(key+"\n")
        fout.write(val)
    fout.write("-"*40)
    
    fout.write("\n\n\n\n Threshold MEAN values validation:\n")
    fout.write("-"*40+"\n")
    for i in ["IBL","Blayer","L1","L2","Disk"]:
        
        for j in CalibThreshold[i]:
            if len(CalibThreshold[i][j]) == 0:
                continue 
            _bool_, _str_ = ValThreshold(i,j,np.average(CalibThreshold[i][j]))
            fout.write("\t"+_str_)
            
        if i == "IBL":
            figurIBL(i,CalibThreshold, "Threshold","CalibThreshold_"+i+".png")
            figurIBL(i,CalibRMS      , "RMS"      ,"CalibRMS_"+i+".png")
        else:
            figur(i,expectedTOTint, "ToT for intime threshold", "totIntime_"+i+".png")
            figur(i,CalibThreshold, "Threshold","CalibThreshold_"+i+".png")
            figur(i,CalibRMS      , "RMS"      ,"CalibRMS_"+i+".png")
            figur(i,CalibNoise    , "Noise"    ,"CalibNoise_"+i+".png")
            figur(i,CalibIntime   , "Intime"   ,"CalibIntime_"+i+".png")
        
    fout.write("-"*40+"\n\n")
    
    totalFE = 0
    totalParams = 0
    fe_indiv     = {"IBL":0,"Blayer":0,"L1":0,"L2":0,"Disk":0}
    params_indiv = {"IBL":0,"Blayer":0,"L1":0,"L2":0,"Disk":0}
    for key, FEs in calib.items():
        
        mod_name = mapping[str(key)]
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
        
        totalFE += len(FEs)
        fe_indiv[mod_layer] += len(FEs)
        for fe in FEs:
            totalParams += len(fe)
            params_indiv[mod_layer] += len(fe)
        
    
    fout.write("Modules checked   : %6u\n" % (len(calib)))
    fout.write("FrontEnds checked : %6u\n" % (totalFE))
    for key, value in fe_indiv.items():
        fout.write("  - %-6s:%6u \n" % (key,value))
    fout.write("Parameters checked: %6u\n" % (totalParams))
    for key, value in params_indiv.items():
        fout.write("  - %-6s:%6u \n" % (key,value))
    
    
    fout.close()

    #open and read the file for terminal print:
    fin = open("CheckValues_log.txt", "r")
    print(fin.read())    
    

def ValThreshold (layer, pix, listavg, perc=1):
    
    # Those values are coming from online crew - Analog thresholds
    # https://twiki.cern.ch/twiki/bin/viewauth/Atlas/PixelConditionsRUN3
    realThresholds = { "IBL": 1500, "Blayer": 4700 , "L1": 4300, "L2": 4300, "Disk": 4300} 
       
    dev = abs((realThresholds[layer]-listavg)/listavg*100)
    status = "OK"
    # If it deviates more than 1%
    if dev > perc:
        status = "NEEDS CHECKING!!!"
    if perc != 1 and status == "OK":
        return False, "OK"
    
    _str_ = "%-25s: %6.1fe (exp.: %4ue), dev.: %6.2f%% - status (>%i%%): %s\n" % (layer+" avg. thr. ["+pix+"]", listavg, realThresholds[layer], dev, perc, status)
    return True, _str_

def figurIBL(title,hist,xlabel,namef):
    fig = Figure(figsize=(13,10))
    fig.suptitle(title)
    plot(fig.add_subplot(2,2,1),hist[title]["normal"]if len(hist[title]["normal"]) else [-1], xlabel+" - normal")
    plot(fig.add_subplot(2,2,2),hist[title]["long"]  if len(hist[title]["long"])   else [-1], xlabel+" - normal")
    plot(fig.add_subplot(2,2,3),hist[title]["normal"]if len(hist[title]["normal"]) else [-1], xlabel+" - normal",True)
    plot(fig.add_subplot(2,2,4),hist[title]["long"]  if len(hist[title]["long"])   else [-1], xlabel+" - normal",True)
    FigureCanvasAgg(fig).print_figure("plots/parameters/"+namef, dpi=150)

def figur(title,hist,xlabel,namef):
    fig = Figure(figsize=(13,10))
    fig.suptitle(title)
    plot(fig.add_subplot(2,3,1),hist[title]["normal"] if len(hist[title]["normal"]) else [-1], xlabel+" - normal")
    plot(fig.add_subplot(2,3,2),hist[title]["long"]   if len(hist[title]["long"])   else [-1], xlabel+" - long")
    plot(fig.add_subplot(2,3,3),hist[title]["ganged"] if len(hist[title]["ganged"]) else [-1], xlabel+" - ganged")
    plot(fig.add_subplot(2,3,4),hist[title]["normal"] if len(hist[title]["normal"]) else [-1], xlabel+" - normal" ,True)
    plot(fig.add_subplot(2,3,5),hist[title]["long"]   if len(hist[title]["long"])   else [-1], xlabel+" - long"   ,True)
    plot(fig.add_subplot(2,3,6),hist[title]["ganged"] if len(hist[title]["ganged"]) else [-1], xlabel+" - ganged" ,True)
    FigureCanvasAgg(fig).print_figure("plots/parameters/"+namef, dpi=150)

def plot(axs,arr, xlabel, islog = False):
    axs.hist(arr, bins=60)
    axs.set_xlabel(xlabel)
    if islog:
        axs.set_yscale("log")
    axs.set_ylabel("Counts [log scale]" if islog else "Counts")  


def  CheckThresholdsIBL(_file):
    from PixelCalibAlgs.EvoMonitoring import ReadCalibOutput
    new_calib, new_iov = ReadCalibOutput(_file)
    
    CheckThresholds(new_calib,new_iov,_file)    
    
    
if __name__ == "__main__":

    import argparse
    parser = argparse.ArgumentParser(prog='python -m PixelCalibAlgs.CheckValues',
                            description="""Checks the Threshold values.\n\n
                            Example: python -m PixelCalibAlgs.CheckValues -f "path/to/file" """)
    
    parser.add_argument('-f'   , required=True, help="New calibration file (output format from the Recovery.py)")
    parser.add_argument('--isDB' , action='store_true', help="File with a DB format (from MakeReferenceFile)")
    args = parser.parse_args()
    
    # Used for testing and checking performance
    if(args.isDB):
        # Used to check the the calibration from the central DB format (using MakeReferenceFile)
        from PixelCalibAlgs.Recovery import ReadDbFile
        new_calib, new_iov = ReadDbFile(args.f)    
    else:
        # Used to check the the calibration from PixelCalibration or CalibrateIBL (removing comments)
        from PixelCalibAlgs.EvoMonitoring import ReadCalibOutput
        new_calib, new_iov = ReadCalibOutput(args.f)
    
    CheckThresholds(new_calib,new_iov,args.f)