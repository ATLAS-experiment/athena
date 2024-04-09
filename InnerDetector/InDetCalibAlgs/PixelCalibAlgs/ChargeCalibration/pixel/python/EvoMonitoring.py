# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
#   This script is used to make plots comparing the lastes IOV in the DB with the new calibration.
#   In order to run it standalone you need to setup Athena and call setupRunning(path_newCalib, path_oldCalib)
#   oldCalib file has the structure of the Recobery.py (after running the calibration)
#   newCalib file has the structure of the MakeReferenceFile (It is on IOV from the central DB)
#

from PixelCalibAlgs.Recovery import ReadDbFile
from PathResolver import PathResolver
import array as ar
import numpy as np
from matplotlib.backends.backend_agg import FigureCanvasAgg
from matplotlib.figure import Figure


def arrayCharge(parameters):
    m_array = [charge(parameters, 5*(1+i)) for i in range(10)]
    return ar.array('d',m_array)

def charge(parameters, tot):
    num = tot*parameters[2] - parameters[0]*parameters[1]
    den = parameters[0] - tot
    return (num/den) if den != 0 else 0

def percent( a,b):
    return a/(a+b)*100 if (a+b) != 0 else 999.99
 

def EvoMon(old_calib, new_calib, mapping, old_iov, new_iov):
    
    save = 1
    log_info = {}
    plot_range = [0,75000]
    
    slopes = []
    information = {"Total_mods": 0, 
                   "Total_FE"  : 0, 
                   "IBL"   :{"bad":0, "ok":0}, 
                   "Blayer":{"bad":0, "ok":0}, 
                   "L1"    :{"bad":0, "ok":0}, 
                   "L2"    :{"bad":0, "ok":0}, 
                   "Disk"  :{"bad":0, "ok":0} 
                  }
    
    for mod in range(len(new_calib)):

        mod_str = mapping[str(mod)]
        mod_layer = ""
        print( "%-18s - %4i" % (mod_str, mod), end='\r')
        
        if mod_str.startswith("L0"): 
            mod_layer = "Blayer"
        elif mod_str.startswith("L1"): 
            mod_layer = "L1"
        elif mod_str.startswith("L2"): 
            mod_layer = "L2"
        elif mod_str.startswith("D"): 
            mod_layer = "Disk"
        else:
            mod_layer = "IBL"
            if mod_str.startswith("LI_S15"): 
                continue
        
        information["Total_mods"] += 1
        fig = Figure(figsize=(13,10))
        axs = fig.add_subplot(1,1,1)
        status = ""
        
        for fe in range(len(new_calib[str(mod)])):
            information["Total_FE"] += 1
            newQ = []
            oldQ = []
            
            if mod_layer != "IBL":
                newCal_normal_pix = new_calib[str(mod)][fe][12:15]
                oldCal_normal_pix = old_calib[str(mod)][fe][12:15]
            
                # We just fet the first point since we loose linearity afetrwards
                newQ = arrayCharge(newCal_normal_pix)[:8]
                oldQ = arrayCharge(oldCal_normal_pix)[:8]
            else:
                # For IBL we dont need to convert TOT into charge, DB already in charge
                newQ = new_calib[str(mod)][fe][4:20]
                oldQ = old_calib[str(mod)][fe][4:20]
                plot_range = [0,35000]
            
            
            m,b = np.polyfit(newQ ,oldQ,1)
            slopes.append(m)
            
            if (abs((1-m)/m)*100) > 5:
                key = "%-18s - %i" % (mod_str, mod)
                if key not in log_info:
                    log_info[key] = "\tFE%02i ---> slope: %5.2f -  deviation: %5.1f%%\n" % (fe,m, abs((1-m)/m)*100)
                else:
                    log_info[key] += "\tFE%02i ---> slope: %5.2f -  deviation: %5.1f%%\n" % (fe,m, abs((1-m)/m)*100)
                    
                information[mod_layer]["bad"] += 1
                status = "_BAD"
            else:
                information[mod_layer]["ok"] += 1
                status = "_OK"
                
            
            if save:
                lstyle = "dotted" if fe < 8 else "solid"                                    
                axs.plot(newQ ,oldQ, marker='o', linestyle=lstyle, label = ("FE%02d" % (fe)))
                axs.set_xlim(plot_range)
                axs.set_ylim(plot_range)
            
        if save:
            fig.suptitle("Hash ID %d - %s" % (mod, mod_str))
            axs.legend(loc='upper left', ncol=4) 
            axs.set_xlabel(new_iov+" - Charge[e]")
            axs.set_ylabel(old_iov+" - Charge[e]")
            axs.plot(plot_range,plot_range,"k:",lw=1)
            
            if mod_layer == 'IBL':
                axs.plot([16000,16000],plot_range,"k:",lw=1)
                axs.text(17000,1000,"TOT@10 = 16 ke")
            elif mod_layer == 'Blayer':
                axs.plot([20000,20000],plot_range,"k:",lw=1)
                axs.text(21000,1000,"TOT@18 = 20 ke")
            else:
                axs.plot([20000,20000],plot_range,"k:",lw=1)
                axs.text(21000,1000,"TOT@30 = 20 ke")
            
            storage = "plots/" + mod_layer + "/"
            
            canvas = FigureCanvasAgg(fig)
            canvas.print_figure(storage+mod_str+"_id"+str(mod)+status+".png", dpi=150)            
    
    if(save):
        fig = Figure(figsize=(13,10))
        fig.suptitle("All modules")
        axs = fig.add_subplot(1,1,1)
        axs.hist(np.clip(slopes, -1, 1.49), bins=100)
        axs.set_yscale("log")
        axs.set_xlabel("Fit slope")
        axs.set_ylabel("Counts")
        FigureCanvasAgg(fig).print_figure("plots/slopes.png", dpi=150)
        
    
    print("-"*20+" SUMMARY "+"-"*20 )
    print("%-20s: %5i"   % ("Total mods in det.", information["Total_mods"]))
    print("%-20s: %5i"   % ("Total FE in det."  , information["Total_FE"]  ))
    print("%-20s: %5i"   % ("Total FE IBL"      , information["IBL"]["ok"]   +information["IBL"]["bad"]   ))
    print("%-20s: %5i"   % ("Total FE Blayer"   , information["Blayer"]["ok"]+information["Blayer"]["bad"]))
    print("%-20s: %5i"   % ("Total FE L1"       , information["L1"]["ok"]    +information["L1"]["bad"]    ))
    print("%-20s: %5i"   % ("Total FE L2"       , information["L2"]["ok"]    +information["L2"]["bad"]    ))
    print("%-20s: %5i\n" % ("Total FE Disk"     , information["Disk"]["ok"]  +information["Disk"]["bad"]  ))
    
    
    print('FrontEnds deviating more than 5% of TOT vs charge gradient between new and previous calibration:')
    print("%-11s: %-4i (%6.2f%%)"   % ("IBL FEs"   , information["IBL"]["bad"]   , percent(information["IBL"]["bad"]   ,information["IBL"]["ok"])   ))    
    print("%-11s: %-4i (%6.2f%%)"   % ("Blayer FEs", information["Blayer"]["bad"], percent(information["Blayer"]["bad"],information["Blayer"]["ok"])))    
    print("%-11s: %-4i (%6.2f%%)"   % ("L1 FEs"    , information["L1"]["bad"]    , percent(information["L1"]["bad"]    ,information["L1"]["ok"])    ))    
    print("%-11s: %-4i (%6.2f%%)"   % ("L2 FEs"    , information["L2"]["bad"]    , percent(information["L2"]["bad"]    ,information["L2"]["ok"])    ))    
    print("%-11s: %-4i (%6.2f%%)\n" % ("Disk FEs"  , information["Disk"]["bad"]  , percent(information["Disk"]["bad"]  ,information["Disk"]["ok"])  ))    
    
    
    print("+"*20+" List of bad FE "+"+"*20 )
    for key, val in log_info.items():
        print(key)
        print(val)
    
        
def ReadCSV():
    mydict = {}
    with open(PathResolver.FindCalibFile("PixelCalibAlgs/mapping.csv")) as fp:
        lines = fp.readlines()
        for line in lines:
            arrayline = line.rstrip("\n").split(', ')
            mydict[arrayline[1]] = arrayline[0]
    return mydict

def ReadCalibOutput(file):
    mydict = {}
    with open(file) as fp:
        lines = fp.readlines()
        for line in lines:
            arrayline = line.rstrip("\n").split(' ')
            
            if arrayline[0] not in mydict:
                mydict[arrayline[0]] = []
            
            # Removing empty values. This is usually used for IBL calib file
            while("" in arrayline):
                arrayline.remove("")
            
            mydict[arrayline[0]].append([ int(arrayline[i]) if i<13 else float(arrayline[i]) for i in range(1,len(arrayline)) ])

    return mydict, "[latest,0]"

def setupRunEvo(path_newCalib, path_oldCalib):
    new_iov = "Latest IOV"
    old_iov = "Older IOV"
    
    print("Files chosen for the comparison:")
    
    print("New calibration: '%s'" % path_newCalib)
    new_calib, new_iov = ReadCalibOutput(path_newCalib)
    print("Old calibration: '%s'" % path_oldCalib)
    old_calib, old_iov = ReadDbFile(path_oldCalib)

    mapping = ReadCSV()
    
    import os
    os.makedirs("plots/Blayer", exist_ok=True)
    os.makedirs("plots/L1"    , exist_ok=True)
    os.makedirs("plots/L2"    , exist_ok=True)
    os.makedirs("plots/Disk"  , exist_ok=True)
    os.makedirs("plots/IBL"   , exist_ok=True)    
    
    EvoMon(old_calib, new_calib, mapping, old_iov, new_iov )    



if __name__ == "__main__":

    import argparse
    parser = argparse.ArgumentParser(prog='python -m PixelCalibAlgs.EvoMonitoring',
                            description="""Compares two IOV and plots the results.\n\n
                            Example: python -m PixelCalibAlgs.EvoMonitoring --new "path/to/file" --old "path/to/file" """)
    
    parser.add_argument('--new', required=True, default="FINAL_calibration_candidate.txt", help="New calibration file (output format from the Recovery.py)")
    parser.add_argument('--old', required=True, default="PixelChargeCalibration-DATA-RUN2-UPD4-26.log", help="Old DB IOV calibration")
    
    args = parser.parse_args()
    setupRunEvo(args.new, args.old)
    exit(0) 
