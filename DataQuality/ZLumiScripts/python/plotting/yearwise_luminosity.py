#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

"""
Plot comparisons of Zee/Zmumu and Z/ATLAS over entire data-periods. 
This can be done as a function of time (validated, working perfectly), 
and pileup (not yet fully validated).
"""

import numpy as np
import pandas as pd
import ROOT as R
import python_tools as pt
import math
from array import array
import time
import argparse
import glob
    
parser = argparse.ArgumentParser()
parser.add_argument('--year', type=str, help='15-18, all for full Run-2')
parser.add_argument('--channel', type=str, help='Zee or Zmumu')
parser.add_argument('--comp', action='store_true', help='Compare Zee and Zmumu?')
parser.add_argument('--absolute', action='store_true', help='Compare absolute luminosity')
parser.add_argument('--indir', type=str, help='Input directory for CSV files')
parser.add_argument('--outdir', type=str, help='Output directory for plots')

args    = parser.parse_args()
year    = args.year
channel = args.channel
absolute = args.absolute
indir = args.indir
outdir = args.outdir

print("------------------------------------------")
print("Begin Yearwise Lumi vs Time")
print("------------------------------------------")

# Do all of the ugly plot stlying here
if year == "run3": 
    years = ["22", "23"]
    out_tag = "_run3"
    time_format = "%m/%y"
    if args.absolute:
        ymin, ymax = 1.01, 1.11
    else:
        ymin, ymax = 0.95, 1.05
    xtitle = 'Month / Year'
    date_tag = "Run 3, #sqrt{s} = 13.6 TeV"
    norm_type = "Run3"
    if channel is not None: 
        xval = 0.25
        yval = 0.85
    else:
        xval = 0.25
        yval = 0.85
    set_size = 1
else: 
    years = [year]
    out_tag = year
    time_format = "%d/%m"
    ymin, ymax = 0.94, 1.06
    xtitle = 'Date in 20' + year
    date_tag = "Data 20" + year  + ", #sqrt{s} = 13.6 TeV"
    norm_type = "year"
    xval = 0.20
    yval = 0.86
    set_size = 0


if channel == "Zee": 
    zstring   = "Z #rightarrow ee counting"
    ytitle    = 'L_{Z #rightarrow ee}/L_{ATLAS}'
    if args.absolute:
        leg_entry = "L_{Z #rightarrow ee}"
    else:
        leg_entry = "L_{Z #rightarrow ee}^{"+norm_type+"-normalised}/L_{ATLAS}"
elif channel == "Zmumu":
    zstring   = "Z #rightarrow #mu#mu counting"
    ytitle    = 'L_{Z #rightarrow #mu#mu}/L_{ATLAS}'
    if args.absolute:
        leg_entry = "L_{Z #rightarrow #mu#mu}"
    else:
        leg_entry = "L_{Z #rightarrow #mu#mu}^{"+norm_type+"-normalised}/L_{ATLAS}"
elif channel == "Zll":
    zstring   = "Z #rightarrow ll counting"
    ytitle    = 'L_{Z #rightarrow ll}/L_{ATLAS}'
    if args.absolute:
        leg_entry = "L_{Z #rightarrow ll}"
    else:
        leg_entry = "L_{Z #rightarrow ll}^{"+norm_type+"-normalised}/L_{ATLAS}"

def main():
    if args.comp: 
        channel_comparison(years)
    else: 
        zcounting_vs_atlas(channel, years)
    #CountZ(years)

def channel_comparison(years):

    print("------------------------------------------")
    print("Begin Yearwise Lumi Channel Comparison vs Time")
    print("------------------------------------------")

    dict_zlumi = {}
    for year in years:  

        #maindir  = indir #+ "data"+year+"_13p6TeV/"
        print("year = ", year)

        if year == "23":

            grl = []
            grl = pt.get_grl(year)
            print("2023 grl = ", grl)

            maindir = args.indir

        elif year == "22":
            maindir = "/eos/atlas/atlascerngroupdisk/perf-lumi/Zcounting/Run3/CSVOutputs/HighMu/data22_13p6TeV/temp/physics_Main_MC23a/"
            grl = pt.get_grl(year)
            print("2022 grl = ", grl) 
                
        else:
            grl = pt.get_grl(year)
            print("other grl = ", grl)

        for channel in ["Zee", "Zmumu"]:

            for run in grl: 
                dfz = pd.read_csv(maindir + "run_" + run + ".csv")
                dfz_small = dfz
                dfz_small['ZLumi'] = dfz_small[channel + 'Lumi']
                dfz_small['ZLumiErr'] = dfz_small[channel + 'LumiErr']
                dfz_small = dfz_small.drop(dfz_small[dfz_small.ZLumi == 0].index)   
                # Cut out all runs shorter than 40 minutes
                if dfz_small['LBLive'].sum()/60 < 40: 
                    continue
 
                dfz_small['ZLumi'] *= dfz_small['LBLive']
                dfz_small['ZLumiErr'] *= dfz_small['LBLive']
                # If plotting vs. date simply fill the arrays here
                zlumi = dfz_small['ZLumi'].sum()
                
                dfz_small['ZLumiErr'] *= dfz_small['ZLumiErr']
                zerr = math.sqrt(dfz_small['ZLumiErr'].sum())
            
                # Grab start of the run for plotting later on
                run_start = dfz_small['LBStart'].iloc[0]
                timestamp = time.gmtime(run_start)
                timestamp = R.TDatime(timestamp[0], timestamp[1], timestamp[2], timestamp[3], timestamp[4], timestamp[5])
                timestamp = timestamp.Convert()
                dict_zlumi[channel, run] = (zlumi, zerr, timestamp)
    
    print("grl = ", grl)
    vec_times     = array('d')
    vec_ratio     = array('d')
    vec_ratio_err = array('d')
    keys = [key[1] for key in dict_zlumi if "Zee" in key]

    # If plotting vs. date simply calculate integrated lumi per run and fill array
    for key in sorted(keys):
        try:
            ratio = dict_zlumi["Zee", key][0]/dict_zlumi["Zmumu", key][0]
            error = ratio * math.sqrt( pow(dict_zlumi["Zee", key][1]/dict_zlumi["Zee", key][0], 2) + pow(dict_zlumi["Zmumu", key][1]/dict_zlumi["Zmumu", key][0], 2) )
            date  = dict_zlumi["Zee", key][2]
        
            if ratio < ymin or ratio > ymax:
                print("Run", key, "has ratio", ratio)
                print("Outside of y-axis range")
            else:
                vec_times.append(date)
                vec_ratio.append(ratio)
                vec_ratio_err.append(error)
        except KeyError:
            print("Cannot do ratio for", key)

    tg = R.TGraphErrors(len(vec_times), vec_times, vec_ratio, R.nullptr, vec_ratio_err)
    leg = R.TLegend(0.645, 0.72, 0.805, 0.91)

    # Depending if we're plotting over whole Run-3, change canvas size
    if out_tag == "_run3":
        c1 = R.TCanvas("c1", "c1", 2000, 1000)
    else:
        c1 = R.TCanvas()

    tg.Draw('ap')
    tg.GetYaxis().SetTitle('L_{Z #rightarrow ee} / L_{Z #rightarrow #mu#mu}')
    tg.Fit('pol0', '0q')
    tg.GetFunction('pol0').SetLineColor(R.kRed)

    mean =  round(tg.GetFunction('pol0').GetParameter(0), 4)

    # Plot 68% percentile band
    stdev    = np.percentile(abs(vec_ratio - np.median(vec_ratio)), 68)
    line1 = pt.make_bands(vec_times, stdev, mean)
    line1.Draw("same 3")
    tg.GetFunction('pol0').Draw("same l")
    tg.Draw('same ep')

    print("#### STDEV =", round(stdev, 3))
   
    leg.SetBorderSize(0)
    leg.SetTextSize(0.045)
    leg.AddEntry(tg, "L_{Z #rightarrow ee}/L_{Z #rightarrow #mu#mu}", "ep")
    leg.AddEntry(tg.GetFunction("pol0"), "Mean = " + str(round(mean, 3)), "l")
    leg.AddEntry(line1, "68% band", "f")
    leg.Draw()

    #pt.drawAtlasLabel(xval, 0.86, "Internal")
    pt.drawAtlasLabel(xval, 0.86, "Work In Progress")
    pt.drawText(xval, 0.80, date_tag, set_size)

    new_trig_line = R.TLine(1683743066.0, 0.95, 1683743066.0, 1.05)
        
    new_trig_line.SetLineColor(R.kBlue)
    new_trig_line.SetLineWidth(3)
    new_trig_line.SetLineStyle(2)
    new_trig_line.Draw("same")
    R.gPad.Update()
    
    tg.GetYaxis().SetRangeUser(ymin, ymax)
    tg.GetXaxis().SetTitle(xtitle)
    tg.GetXaxis().SetTimeDisplay(2)
    tg.GetXaxis().SetNdivisions(9,R.kFALSE)
    tg.GetXaxis().SetTimeFormat(time_format)
    tg.GetXaxis().SetTimeOffset(0,"gmt")

    if years == ["22", "23"]:
        plot_title = "Ratio of Electron and Muon channel Z-counting Luminosities across Run 3"
    else:
        plot_title = "Ratio of Electron and Muon channel Z-counting Luminosities across 20" + years[0]

    tg.SetTitle(plot_title)
    c1.Update()
    c1.SaveAs(outdir + "channel_comp_data"+out_tag+".pdf")


def zcounting_vs_atlas(channel, years):
    """
    Plot normalised comparison of Z-counting luminosity to ATLAS luminosity.
    This can be done as a function of time and pileup.
    """
    
    print("------------------------------------------")
    print("Begin Yearwise ", channel, " Lumi ATLAS comparison vs Time")
    print("------------------------------------------")

    arr_date  = []
    arr_olumi = []
    arr_zlumi = []
    arr_zerr  = []
    run_num   = []

    for year in years:
        print("year = ", year)

        if year == "23":

            grl = []
            grl = pt.get_grl(year)
            print("2023 grl = ", grl)

            maindir = args.indir

        elif year == "22":
            maindir = "/eos/atlas/atlascerngroupdisk/perf-lumi/Zcounting/Run3/CSVOutputs/HighMu/data22_13p6TeV/temp/physics_Main_MC23a/"
            grl = pt.get_grl(year)
            print("other grl = ", grl)
                
        else:
            grl = pt.get_grl(year)
            print("other grl = ", grl)

        for run in grl:

            dfz = pd.read_csv(maindir + "run_" + run + ".csv")
            dfz_small = dfz
            dfz_small['ZLumi'] = dfz_small[channel + 'Lumi']
            dfz_small['ZLumiErr'] = dfz_small[channel + 'LumiErr']
            dfz_small['LBLive'] = dfz_small['LBLive']
            dfz_small = dfz_small.drop(dfz_small[dfz_small.ZLumi == 0].index)

            # Cut out all runs shorter than 40 minutes
            if dfz_small['LBLive'].sum()/60 < 40:
                print("Skip Run", run, "because of live time", dfz_small['LBLive'].sum()/60, "min")
                continue
                
            # Grab start of the run for plotting later on
            run_start = dfz_small['LBStart'].iloc[0]
            timestamp = time.gmtime(run_start)
            timestamp = R.TDatime(timestamp[0], timestamp[1], timestamp[2], timestamp[3], timestamp[4], timestamp[5])
            timestamp = timestamp.Convert()
           
            # Calculate integrated ATLAS luminosity
            dfz_small['OffLumi'] *= dfz_small['LBLive']
            olumi = dfz_small['OffLumi'].sum()

            # Calculate integrated Z-counting luminosity
            dfz_small['ZLumi'] *= dfz_small['LBLive']
            zlumi = dfz_small['ZLumi'].sum()
            
            # Calculate uncertainty on Z-counting
            dfz_small['ZLumiErr'] *= dfz_small['LBLive']
            dfz_small['ZLumiErr'] *= dfz_small['ZLumiErr']
            zerr = math.sqrt(dfz_small['ZLumiErr'].sum())

            # If plotting vs. date simply fill the arrays here
            arr_date.append(timestamp)
            arr_olumi.append(olumi)
            arr_zlumi.append(zlumi)
            arr_zerr.append(zerr)
            run_num.append(run)

    # for ROOT plotting we need Python arrays
    arr_date = array('d', arr_date)
    # convert lists to numpy arrays
    arr_olumi = np.array(arr_olumi)
    arr_zlumi = np.array(arr_zlumi)
    arr_zerr = np.array(arr_zerr)
    print("arr_zlumi = ", arr_zlumi)
    total_zlumi = arr_zlumi.sum()/1000000
    total_zlumi_string = "Official Data Quality, " + str(round(total_zlumi, 2)) + " fb^-1"

    # calculate ratio to ATLAS preferred lumi
    arr_zlumi_ratio = arr_zlumi/arr_olumi
    arr_zerr_ratio  = arr_zerr/arr_olumi
    print("Lumi array = ", arr_zlumi_ratio)

#-----------Normalisation------------

    print("Official lumi", np.sum(arr_olumi))
    # Calculate and apply overall normalisation
    if args.absolute:
        normalisation = 1.0
    else:
        normalisation = np.sum(arr_zlumi) / np.sum(arr_olumi)

    print("norm = ", normalisation)
    # do normalisation to period integral
    arr_zlumi_ratio /= normalisation
    arr_zerr_ratio  /= normalisation
    
    print("Lumi array = ", arr_zlumi_ratio)

#-----------Normalisation------------

    tg = R.TGraphErrors(len(arr_date), arr_date, array('d',arr_zlumi_ratio), R.nullptr, array('d',arr_zerr_ratio))

    if args.absolute:
        if channel == "Zee":
            if years == ["22", "23"]:
                plot_title = "Ratio of absolute Z->ee counting Luminosity to ATLAS Luminosity across Run 3"
            else:
                plot_title = "Ratio of absolute Z->ee counting Luminosity to ATLAS Luminosity across 20" + years[0]
        if channel == "Zmumu":
            if years == ["22", "23"]:
                plot_title = "Ratio of absolute Z->mumu counting Luminosity to ATLAS Luminosity across Run 3"
            else:
                plot_title = "Ratio of absolute Z->mumu counting Luminosity to ATLAS Luminosity across 20" + years[0]
        if channel == "Zll":
            if years == ["22", "23"]:
                plot_title = "Ratio of absolute Z->ll counting Luminosity to ATLAS Luminosity across Run 3"
            else:
                plot_title = "Ratio of absolute Z->ll counting Luminosity to ATLAS Luminosity across 20" + years[0]
    else:
        if channel == "Zee":
            if years == ["22", "23"]:
                plot_title = "Ratio of normalised Z->ee counting Luminosity to ATLAS Luminosity across Run 3"
            else:
                plot_title = "Ratio of normalised Z->ee counting Luminosity to ATLAS Luminosity across 20" + years[0]
        if channel == "Zmumu":
            if years == ["22", "23"]:
                plot_title = "Ratio of normalised Z->mumu counting Luminosity to ATLAS Luminosity across Run 3"
            else:
                plot_title = "Ratio of normalised Z->mumu counting Luminosity to ATLAS Luminosity across 20" + years[0]
        if channel == "Zll":
            if years == ["22", "23"]:
                plot_title = "Ratio of normalised Z->ll counting Luminosity to ATLAS Luminosity across Run 3"
            else:
                plot_title = "Ratio of normalised Z->ll counting Luminosity to ATLAS Luminosity across 20" + years[0]

    tg.SetTitle(plot_title+";"+xtitle+";"+ytitle)

    # Depending if we're plotting over whole Run-3, change canvas size
    if out_tag == "_run3":
        c1 = R.TCanvas("c1", "c1", 2000, 1200)
        c1.SetTopMargin(0.1)
    else:
        c1 = R.TCanvas("c1", "c1", 1000, 750)
        c1.SetTopMargin(0.1)

    tg.Draw('ap')
    tg.GetYaxis().SetRangeUser(ymin, ymax)
    
    # Plot 68% percentile band
    stdev = np.percentile(abs(arr_zlumi_ratio - np.median(arr_zlumi_ratio)), 68)
    print("68% band =", stdev)
    tg.Fit('pol0', '0q')
    mean = tg.GetFunction('pol0').GetParameter(0)
    print("const of pol0 fit", mean) 
    print("median", np.median(arr_zlumi_ratio)) 
    print("mean", np.mean(arr_zlumi_ratio)) 
    
    line1 = pt.make_bands(arr_date, stdev, mean)
    line1.Draw("same 3")
    tg.Draw('same ep')

    leg = R.TLegend(0.55, 0.20, 0.69, 0.45)
    leg.SetBorderSize(0)
    leg.SetTextSize(0.045)
    leg.AddEntry(tg, leg_entry, "ep")
    leg.AddEntry(line1, "68% band", "f")
    leg.Draw()
    
    if args.absolute:
        #pt.drawAtlasLabel(xval, yval-0.47, "Internal")
        pt.drawAtlasLabel(xval, yval-0.47, "Work In Progress")        
        pt.drawText(xval, yval-0.53, date_tag, set_size)
        pt.drawText(xval, yval-0.59, zstring, set_size)
        pt.drawText(xval, yval-0.65, "OflLumi-Run3-003", set_size)
    else:
        #pt.drawAtlasLabel(xval, yval-0.47, "Internal")
        pt.drawAtlasLabel(xval, yval-0.47, "Work In Progress")
        pt.drawText(xval, yval-0.53, date_tag, set_size)
        pt.drawText(xval, yval-0.59, zstring, set_size)
        pt.drawText(xval, yval-0.65, "OflLumi-Run3-003", set_size)
        pt.drawText(xval, yval-0.02, total_zlumi_string, set_size)

    pt.drawText(xval-0.12, 0.95, plot_title, set_size)

    tg.GetYaxis().SetRangeUser(ymin, ymax)
    #tg.GetXaxis().SetTitle(xtitle)
    tg.GetXaxis().SetTimeDisplay(2)
    tg.GetXaxis().SetLabelSize(0.04)
    tg.GetYaxis().SetLabelSize(0.04)
    tg.GetXaxis().SetNdivisions(9,R.kFALSE)
    tg.GetXaxis().SetTimeFormat(time_format)
    tg.GetXaxis().SetTimeOffset(0,"gmt")
    
    c1.Update()
    c1.Modified()

    if args.absolute:
        c1.SaveAs(outdir + channel + "_counting_data"+out_tag+"_abs.pdf")
        outfile = R.TFile(outdir + channel + "_counting_data"+out_tag+"_abs.root", "RECREATE")
    else:
        c1.SaveAs(outdir + channel + "_counting_data"+out_tag+".pdf")
        outfile = R.TFile(outdir + channel + "_counting_data"+out_tag+".root", "RECREATE")

    tg.Write()
    line1.SetName("Line")
    line1.Write()
    outfile.Close()

def local_fit(tg, start, end, year):
    """
    Fit over a sub-range of the data and print the mean and chi^2/NDF. 
    Useful to test the remaining trends after the global Run-3 normalisation.
    """

    tg.Fit('pol0', 'Rq0','0', start, end)
    mean = round(tg.GetFunction('pol0').GetParameter(0), 3)
    chi2 = tg.GetFunction('pol0').GetChisquare()
    ndf  = tg.GetFunction('pol0').GetNDF()
    print("|", year, "|", mean, "|", round(chi2/ndf, 3), "|")


if __name__ == "__main__":
    pt.setAtlasStyle()
    R.gROOT.SetBatch(R.kTRUE)
    main()
