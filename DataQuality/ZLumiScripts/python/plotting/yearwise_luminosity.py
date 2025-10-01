#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

"""
Plot comparisons of Zee/Zmumu and Z/ATLAS over entire data-periods. 
This can be done as a function of time or pileup
"""

import numpy as np
import ROOT as R
import python_tools as pt
import math
from array import array
import argparse
import csv
    
parser = argparse.ArgumentParser()
parser.add_argument('--year', type=str, help='15,16,17,18,22,23,24,25 or run3 or year1_year2_...')
parser.add_argument('--channel', type=str, help='Zee or Zmumu')
parser.add_argument('--comp', action='store_true', help='Compare Zee and Zmumu?')
parser.add_argument('--absolute', action='store_true', help='Compare absolute luminosity')
parser.add_argument('--indir', type=str, help='Input directory for CSV files')
parser.add_argument('--outdir', type=str, help='Output directory for plots')
parser.add_argument('--outcsv', action='store_true', help='Create short CSV with plot content')

args    = parser.parse_args()
year    = args.year
channel = args.channel
absolute = args.absolute
indir = args.indir
outdir = args.outdir
outcsv = args.outcsv

# Do all of the ugly plot stlying here
yval = 0.85
xval = 0.2
if args.absolute: ymin, ymax = 0.91, 1.09
else: ymin, ymax = 0.93, 1.07

if year == "run3": 
    years = ["22", "23", "24", "25"]
    out_tag = "run3"
    time_format = "%m/%y"
    xtitle = 'Month / Year'
    date_tag = "Run 3,#kern[-0.5]{ }#sqrt{s} = 13.6 TeV"
    labelsize = 44
    norm_type = "Run3"
    multiyear = True
elif len(year.split("_")) > 1: 
    years = year.split("_")
    out_tag = "data"+year
    time_format = "%m/%y"
    xtitle = 'Month / Year'
    date_tag = "Run 3,#kern[-0.5]{ }#sqrt{s} = 13.6 TeV"
    labelsize = 44
    norm_type = "Run3"
    multiyear = True
else: 
    years = [year]
    out_tag = "data"+year
    time_format = "%d/%m"
    xtitle = 'Date in 20' + year
    date_tag = "Data 20" + year  + ",#kern[-0.5]{ }#sqrt{s} = 13.6 TeV"
    norm_type = "year"
    labelsize = 22
    multiyear = False

def main():
    if args.comp: 
        channel_comparison(years)
    else: 
        zcounting_vs_atlas(channel, years)

def channel_comparison(years):

    print("Lumi Channel Comparison vs Time for years: ", years)

    dict_zlumi = {}
    for year in years:  

        grl = pt.get_grl(year)

        for channel in ["Zee", "Zmumu"]:

            for run in grl: 
                livetime, zlumi, zerr, olumi, timestamp, dfz_small = pt.get_dfz(args.indir, year, run, channel)
                # Cut out short runs
                if livetime < pt.runlivetimecut:
                    if livetime >= 0.: print(f"Skip Run {run} because of live time {livetime/60:.1f} min")
                    continue

                dict_zlumi[channel, run] = (zlumi, zerr, timestamp)

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
                print("WARNING: Run", key, "has Zee/Zmumu ratio %.2f +- %.2f, outside of y-axis range" % (ratio, error))
            else:
                vec_times.append(date)
                vec_ratio.append(ratio)
                vec_ratio_err.append(error)
        except KeyError:
            print("Cannot do ratio for", key)

    tg = R.TGraphErrors(len(vec_times), vec_times, vec_ratio, R.nullptr, vec_ratio_err)
    leg = R.TLegend(0.645, 0.72, 0.805, 0.91)
    leg.SetFillStyle(0)

    # Depending if we're plotting over whole Run-3, change canvas size
    if multiyear:
        c1 = R.TCanvas("c1", "c1", 2000, 1000)
    else:
        c1 = R.TCanvas()

    tg.Draw('ap')
    tg.GetYaxis().SetTitle(pt.Leemumuratiolabel)
    tg.Fit('pol0', '0q')
    tg.GetFunction('pol0').SetLineColor(R.kRed)

    mean = tg.GetFunction('pol0').GetParameter(0)

    # Plot 68% percentile band
    stdev = np.percentile(abs(vec_ratio - np.median(vec_ratio)), 68)
    line1 = pt.make_bands(vec_times, stdev, mean)
    line1.Draw("same 3")
    tg.GetFunction('pol0').Draw("same l")
    tg.Draw('same e0p')

    print("Pol0 fit mean +- 68% percentile = ", round(mean,3), " +- ", round(stdev, 3))
   
    leg.SetBorderSize(0)
    leg.SetTextSize(0.05)
    leg.AddEntry(tg, pt.Leemumuratiolabel, "ep")
    leg.AddEntry(tg.GetFunction("pol0"), "Mean = %.3f" % mean, "l")
    leg.AddEntry(line1, "68%% band (#pm %.3f)" % stdev, "f")
    leg.Draw()

    pt.drawAtlasLabel(xval, 0.88, "Internal")
    pt.drawText(xval, 0.82, date_tag, size=labelsize)

    R.gPad.Update()
    
    tg.GetYaxis().SetRangeUser(ymin, ymax)
    tg.GetXaxis().SetTitle(xtitle)
    tg.GetXaxis().SetTimeDisplay(2)
    tg.GetXaxis().SetNdivisions(9,R.kFALSE)
    tg.GetXaxis().SetTimeFormat(time_format)
    tg.GetXaxis().SetTimeOffset(0,"gmt")

    if years == ["22", "23", "24", "25"]:
        plot_title = "Ratio of Electron and Muon channel Z-counting Luminosities across Run 3"
    else:
        plot_title = "Ratio of Electron and Muon channel Z-counting Luminosities across 20" + years[0]

    tg.SetTitle(plot_title)
    c1.Update()
    c1.SaveAs(outdir + "ZeeZmm_ratio_vs_time_"+out_tag+".pdf")


def zcounting_vs_atlas(channel, years):
    """
    Plot normalised comparison of Z-counting luminosity to ATLAS luminosity.
    This can be done as a function of time and pileup.
    """

    zstring   = pt.plotlabel[channel]+" counting"
    ytitle    = "L_{"+pt.plotlabel[channel]+"}/L_{ATLAS}"
    if args.absolute:
        leg_entry = "L_{"+pt.plotlabel[channel]+"}"
    else:
        leg_entry = "L_{"+pt.plotlabel[channel]+"}^{"+norm_type+"-normalised}/L_{ATLAS}"
    
    print("Channel", channel, "comparison to ATLAS vs time for years: ", years)

    arr_date  = []
    arr_olumi = []
    arr_zlumi = []
    arr_zerr  = []
    run_num   = []
    fill_num = []

    for year in years:
        
        grl = pt.get_grl(year)

        for run in grl:
            livetime, zlumi, zerr, olumi, timestamp, dfz_small = pt.get_dfz(args.indir, year, run, channel)
            
            # Cut out short runs
            if livetime < pt.runlivetimecut:
                if livetime >= 0.: print(f"Skip Run {run} because of live time {livetime/60:.1f} min")
                continue

            prelratio = zlumi/olumi
            if prelratio < ymin or prelratio > ymax:
                print("WARNING: Run", run, "has", channel, "/ATLAS ratio %.2f +- %.2f, outside of y-axis range" % (prelratio, zerr/olumi) )
            
            # If plotting vs. date simply fill the arrays here
            arr_date.append(timestamp)
            arr_olumi.append(olumi)
            arr_zlumi.append(zlumi)
            arr_zerr.append(zerr)
            run_num.append(run)
            fill = dfz_small['FillNum'].median()
            fill_num.append(int(fill))

    
    # for ROOT plotting we need Python arrays
    arr_date = array('d', arr_date)
    # convert lists to numpy arrays
    arr_olumi = np.array(arr_olumi)
    arr_zlumi = np.array(arr_zlumi)
    arr_zerr = np.array(arr_zerr)
    total_lumi = arr_olumi.sum()/1000000
    total_lumi_string = "Official DQ "
    if year == "25": total_lumi_string = "Preliminary DQ "
    total_lumi_string += str(round(total_lumi, 1)) + " fb^{-1}"

    #-----------Normalisation------------

    # Calculate and apply overall normalisation
    if args.absolute:
        normalisation = 1.0
    else:
        normalisation = np.sum(arr_zlumi) / np.sum(arr_olumi)

    # do normalisation to period integral
    arr_zlumi /= normalisation
    arr_zerr  /= normalisation

    # calculate ratio to ATLAS preferred lumi
    arr_zlumi_ratio = arr_zlumi/arr_olumi
    arr_zerr_ratio  = arr_zerr/arr_olumi

#-----------Normalisation------------

    tg = R.TGraphErrors(len(arr_date), arr_date, array('d',arr_zlumi_ratio), R.nullptr, array('d',arr_zerr_ratio))

    # Depending if we're plotting over whole Run-3, change canvas size
    if multiyear:
        c1 = R.TCanvas("c1", "c1", 2000, 1000)
        leg = R.TLegend(0.5, 0.18, 0.65, 0.4)
    else:
        c1 = R.TCanvas()
        leg = R.TLegend(0.6, 0.18, 0.75, 0.4)

    tg.Draw('ap e0')
    tg.GetYaxis().SetRangeUser(ymin, ymax)
    if args.absolute:
        plot_title = "Absolute L_{"+ zstring +"} to ATLAS across " + norm_type
    else:
        plot_title = "Normalised L_{"+ zstring +"} to ATLAS across " + norm_type
    
    # Plot 68% percentile band
    stdev = np.percentile(abs(arr_zlumi_ratio - np.median(arr_zlumi_ratio)), 68)
    print("68% band =", stdev)
    tg.Fit('pol0', '0q')
    mean = tg.GetFunction('pol0').GetParameter(0)
    print("const of pol0 fit", mean) 
    print("median", np.median(arr_zlumi_ratio)) 
    print("mean", np.mean(arr_zlumi_ratio)) 
    
    line1 = pt.make_bands(arr_date, stdev, np.median(arr_zlumi_ratio))
    line1.Draw("same 3")
    tg.Draw('same ep')

    leg.SetFillStyle(0)
    leg.SetBorderSize(0)
    leg.SetTextSize(0.05)
    leg.AddEntry(tg, leg_entry, "ep")
    leg.AddEntry(line1, "68%% band (#pm %.3f)" % stdev, "f")
    leg.Draw()

    pt.drawAtlasLabel(xval, yval-0.47, "Internal")
    pt.drawText(xval, yval-0.53, date_tag, size=labelsize)
    pt.drawText(xval, yval-0.59, zstring, size=labelsize)
    pt.drawText(xval, yval-0.65, "OflLumi-Run3-006", size=labelsize)
    pt.drawText(xval, yval-0.04, total_lumi_string, size=labelsize)

    pt.drawText(xval, 0.88, plot_title, size=labelsize)

    tg.GetXaxis().SetTitle(xtitle)
    tg.GetYaxis().SetTitle(ytitle)
    tg.GetYaxis().SetRangeUser(ymin, ymax)
    tg.GetXaxis().SetTimeDisplay(2)
    tg.GetXaxis().SetNdivisions(9,R.kFALSE)
    tg.GetXaxis().SetTimeFormat(time_format)
    tg.GetXaxis().SetTimeOffset(0,"gmt")
    
    c1.Update()
    c1.Modified()

    filename = outdir + channel + "ATLAS_ratio_vs_time_"+out_tag
    if args.absolute:
        c1.SaveAs(filename+"_abs.pdf")
    else:
        c1.SaveAs(filename+".pdf")

    if outcsv:
        if args.absolute:
            csvfile = open(filename+"_abs.csv", 'w')
        else:
            csvfile = open(filename+".csv", 'w')
        csvwriter = csv.writer(csvfile, delimiter=',')
        csvwriter.writerow(['FillNum','RunNum','Time','OffLumi','ZLumi','ZLumiErr','OffZlumi','OffZlumiErr'])
        for i in range(len(run_num)):
            csvwriter.writerow([fill_num[i], run_num[i], arr_date[i], arr_olumi[i], arr_zlumi[i], arr_zerr[i], arr_zlumi_ratio[i], arr_zerr_ratio[i]])
        csvfile.close()



if __name__ == "__main__":
    pt.setAtlasStyle()
    R.gROOT.SetBatch(R.kTRUE)
    main()
