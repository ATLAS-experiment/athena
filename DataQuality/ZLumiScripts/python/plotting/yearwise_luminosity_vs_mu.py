#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

import numpy as np
import pandas as pd
import ROOT as R
import python_tools as pt
import argparse

pd.set_option('display.max_rows', None)

parser = argparse.ArgumentParser()
parser.add_argument('--year', type=str, help='15,16,17,18,22,23,24,25 or run2,run3 or year1_year2_...')
parser.add_argument('--channel', type=str, help='Zee or Zmumu or Zll')
parser.add_argument('--comp', action='store_true', help='Compare Zee and Zmumu?')
parser.add_argument('--indir', type=str, help='Input CSV file directory')
parser.add_argument('--outdir', type=str, help='Output plot directory')

args    = parser.parse_args()
year    = args.year
channel = args.channel

comp = args.comp
indir = args.indir
outdir = args.outdir
print("Begin Yearwise Lumi vs Mu")
    
if year == "15":
    xmin = 0.2
    bins = np.concatenate((np.array([0, 10]), np.linspace(11, 17, 7), np.array([18, 22])))
elif year == "16":
    xmin = 0.4
    bins = np.concatenate((np.array([0, 10]), np.linspace(11, 39, 29), np.array([40, 45])))
elif year == "17":
    xmin = 0.4
    bins = np.concatenate((np.array([0, 15]), np.linspace(16, 59, 44), np.array([60, 70])))
elif year == "18":
    xmin = 0.2
    bins = np.concatenate((np.array([0, 15]), np.linspace(16, 55, 40), np.array([56, 70])))
elif year == "22":
    xmin = 0.5
    bins = np.concatenate((np.array([0, 20]), np.linspace(21, 53, 33), np.array([54, 70])))
elif year == "run3" or len(year.split("_")) > 1:
    xmin = 0.5
    bins = np.concatenate((np.array([0, 20]), np.linspace(21, 61, 41), np.array([62, 80])))
else:
    xmin = 0.2
    bins = np.concatenate((np.array([0, 26]), np.linspace(27, 61, 35), np.array([62, 80])))

if year == "run2":
    date_string = "Run 2,#kern[-0.5]{ }#sqrt{s} = 13 TeV"
    grl = pt.get_grl("15")
    grl.extend(pt.get_grl("16"))
    grl.extend(pt.get_grl("17"))
    grl.extend(pt.get_grl("18"))
    out_tag = "run2"
elif year == "run3":  
    date_string = "Run 3,#kern[-0.5]{ }#sqrt{s} = 13.6 TeV"
    grl = pt.get_grl("22")
    grl.extend(pt.get_grl("23"))
    grl.extend(pt.get_grl("24"))
    grl.extend(pt.get_grl("25"))
    out_tag = "run3"
elif len(year.split("_")) > 1: 
    date_string = "Run 3,#kern[-0.5]{ }#sqrt{s} = 13.6 TeV"
    grl = pt.get_grl(year.split("_")[0])
    for i in year.split("_")[1:]:
        grl.extend(pt.get_grl(i))
    out_tag = "data"+year
else:
    out_tag = "data"+year
    date_string = "Data 20"+year+",#kern[-0.5]{ }#sqrt{s} = 13 TeV"
    grl = pt.get_grl(year)
    if int(year) >= 22: date_string = date_string.replace("13 TeV", "13.6 TeV")

outfile = "ZeeZmm_ratio_vs_mu_"+out_tag+".pdf"

ymin, ymax = 0.94, 1.06

def main():
    dflist = []
    for run in grl: 
        livetime, zlumi, zerr, olumi, timestamp, dfz_small = pt.get_dfz(args.indir, year, run, channel)

        # Cut out short runs
        if livetime < pt.runlivetimecut:
            if livetime >= 0.: print(f"Skip Run {run} because of live time {livetime/60:.1f} min")
            continue
            
        # Cut out early 2016 runs with "strange" bunch structure
        if (year == "16" and dfz_small['LBStart'].iloc[0] < 1463184000) or run == "310247": 
            continue
        
        dflist.append(dfz_small)
   
    df = pd.concat(dflist)
    df['OffMu'] = df['OffMu'].round(0)
    df = df.groupby(pd.cut(df.OffMu, bins, right=False), observed=False).sum()
    df.reset_index(drop=True, inplace=True)
    if comp:
        df['ZeeLumiErr'] = np.sqrt(df['ZeeLumiErr'])
        df['ZmumuLumiErr'] = np.sqrt(df['ZmumuLumiErr'])
        df['Ratio']    = df['ZeeLumi'] / df['ZmumuLumi']
        df['RatioErr'] = df['Ratio'] * np.sqrt(pow(df['ZeeLumiErr'] / df['ZeeLumi'], 2) + pow(df['ZmumuLumiErr'] / df['ZmumuLumi'], 2))
    else:
        df['ZLumiErr'] = np.sqrt(df['ZLumiErr'])
        df['Bin'] = pd.Series(bins)
        
        norm = df['ZLumi'].sum() / df['OffLumi'].sum()
        df['Ratio']    = df['ZLumi'] / df['OffLumi'] / norm
        df['RatioErr'] = df['ZLumiErr'] / df['OffLumi'] / norm

    h_total = R.TH1F("h_total", "", len(bins)-1, bins)

    nan_list = df[df['Ratio'].isnull()].index.tolist()
    
    arr_ratio = []
    
    for xbin in range(0, h_total.GetNbinsX()):
        
        if xbin in nan_list:            
            continue
            
        try:
            h_total.SetBinContent(xbin+1, df['Ratio'][xbin])
            h_total.SetBinError(xbin+1, df['RatioErr'][xbin])
            arr_ratio.append(df['Ratio'][xbin])
            
        except KeyError:
            print("Cannot do ratio for", xbin)

    arr_ratio = np.array(arr_ratio)

    median = np.median(arr_ratio)
    stdev = np.percentile(abs(arr_ratio - median), 68)
    
    c1 = R.TCanvas()
    h_total.GetXaxis().SetTitle("<#mu>")
    h_total.Draw("E0")
    R.gStyle.SetErrorX()
    line = R.TLine(h_total.GetXaxis().GetXmin(), median, h_total.GetXaxis().GetXmax(), median)
    line.SetLineColor(R.kRed)
    line.Draw()
    
    if comp:
        h_total.GetYaxis().SetRangeUser(ymin, ymax)
        leg = R.TLegend(0.54, 0.72, 0.805, 0.92)
        leg.SetTextSize(18)
        leg.AddEntry(h_total, pt.Leemumuratiolabel, "ep")
        leg.AddEntry(line, f"Median = {median:.3f}#pm{stdev:.3f}", "l")
    else: 
        h_total.GetYaxis().SetRangeUser(0.95, 1.05)
        leg = R.TLegend(0.20, 0.18, 0.45, 0.35)

    print(f"Year = {year} channel = {channel}: median +- 68% percentile = {median:.3f} +- {stdev:.3f}")

    line1 = pt.make_bands(bins, stdev, median)
    line1.Draw("same 3")
    if comp: line.Draw()
    h_total.Draw('same E0')
    
    leg.SetBorderSize(0)
    leg.SetTextSize(0.05)
    if comp: 
        h_total.GetYaxis().SetTitle(pt.Leemumuratiolabel)
        zstring = ""
    else:
        h_total.GetYaxis().SetTitle("L_{"+pt.plotlabel[channel]+"} / L_{ATLAS}")
        leg.AddEntry(h_total, "L_{"+pt.plotlabel[channel]+"}^{year-normalised}/L_{ATLAS}", "ep")
        zstring = pt.plotlabel[channel]+" counting"

    if comp:
        pt.drawAtlasLabel(0.2, 0.88, "Internal")
        pt.drawText(0.2, 0.83, date_string, size=22)
        pt.drawText(0.2, 0.78, zstring, size=22)
    else:
        pt.drawAtlasLabel(xmin, 0.88, "Internal")
        pt.drawText(xmin, 0.83, date_string, size=22)
        pt.drawText(xmin, 0.78, zstring, size=22)
        pt.drawText(xmin, 0.71, "OflLumi-Run3-006", size=22)
        
    leg.AddEntry(line1, "68% band (#pm" + str(round(stdev, 3)) + ")", "f")
    leg.Draw()

    if comp:
        c1.SaveAs(outdir + outfile)
    else: 
        c1.SaveAs(outdir + channel + "ATLAS_ratio_vs_mu_"+out_tag+".pdf")

if __name__ == "__main__":
    pt.setAtlasStyle()
    R.gROOT.SetBatch(R.kTRUE)
    main()
