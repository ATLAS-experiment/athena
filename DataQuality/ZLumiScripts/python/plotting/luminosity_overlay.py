#!/usr/bin/env python3
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

import numpy as np
import pandas as pd
import python_tools as pt
import ROOT as R
import os
import math
from array import array
import argparse
import re


parser = argparse.ArgumentParser()
parser.add_argument('--infile', type=str, help='input file')
parser.add_argument('--outdir', type=str, help='output directory')
parser.add_argument('--usemu', action='store_true', help='Plot vs. mu. Default == LB')
parser.add_argument('--absolute', action='store_true', help='Use for official lumi absolute comparison')

args = parser.parse_args()
infilename = args.infile
outdir = args.outdir

#substring = "data"
#pattern = re.compile(substring + r"(\d{2})")
#match = pattern.search(infilename)
#if match:
#    year = match.group(1)
#else:
#    print("Year not found")

infilelist = ["/eos/home-j/jnewell/Trig_Changes/data23_13p6TeV/run_451569_old_trig/run_451569.csv", "/eos/home-j/jnewell/Trig_Changes/data23_13p6TeV/run_451569_new_trig/run_451569.csv"]
trignamelist = ["Legacy L1", "Upgrade L1"]

def main():

    Zee_hz_list = []
    Zee_hr_list = []

    Zmumu_hz_list = []
    Zmumu_hr_list = []

    for infile in infilelist: 
        Zee_hz, Zee_hr, Zee_ho, Zee_arr_rat, Zee_arr_bins = plot_channel('Zee', infile)
        Zmumu_hz, Zmumu_hr, Zmumu_ho, Zmumu_arr_rat, Zmumu_arr_bins  = plot_channel('Zmumu', infile)

        Zee_hz_list.append(Zee_hz)
        Zee_hr_list.append(Zee_hr)
    

    draw_plots('Zee', Zee_hz_list, Zee_hr_list, Zee_ho, Zee_arr_rat, Zee_arr_bins)
    #plot_ratio(zeelumi, zeelumierr, zmumulumi, zeelumierr)
    

def plot_channel(channel, infilename):
    dfz = pd.read_csv(infilename, delimiter=',')
    run_number = str(int(dfz.RunNum[0]))
    lhc_fill   = str(int(dfz.FillNum[0]))

    year = "23"

    if channel == 'Zee':
        Lumi = 'ZeeLumi'
        LumiErr = 'ZeeLumiErr'
        LumiString = "L_{Z #rightarrow ee}"
        name = 'e'

        # Drop LBs with no Z-counting information
        dfz = dfz.drop(dfz[(dfz.ZeeLumi == 0)].index)
        print("Zee-dfz")
        print(dfz)

    elif channel == 'Zmumu':
        Lumi = 'ZmumuLumi'
        LumiErr = 'ZmumuLumiErr'
        LumiString = "L_{Z #rightarrow #mu#mu}"
        name = '#mu'

        # Drop LBs with no Z-counting information
        dfz = dfz.drop(dfz[(dfz.ZmumuLumi == 0)].index)
        print("Zmumu-dfz")
        print(dfz)

    #Calculate mean per LB against ATLAS
    dfz = dfz.drop(dfz[(dfz['LBLive']<10) | (dfz['PassGRL']==0)].index)
    #print(dfz[channel+'Lumi'])
    print(dfz['OffLumi'])
    ratio = array('d', dfz[channel+'Lumi']/dfz['OffLumi'])
    print("mean for "+channel+": ", np.mean(ratio))

    # Scale by livetime
    for entry in [Lumi, LumiErr,'OffLumi']:  
        dfz[entry] *= dfz['LBLive']

    # Square uncertainties
    dfz[LumiErr] *= dfz[LumiErr]

    # Merge by groups of 20 LBs or pileup bins
    if args.usemu:
        dfz['OffMu'] = dfz['OffMu'].astype(int)
        dfz = dfz.groupby(['OffMu']).sum()
    else: 
        dfz['LBNum'] = (dfz['LBNum']//20)*20
        dfz = dfz.groupby(['LBNum']).sum()
    
    dfz[LumiErr]   = np.sqrt(dfz[LumiErr])

    print("Making Z-counting vs. ATLAS plots!")
    channel_string = "Z #rightarrow "+name+name
    lep = name
	
    normalisation = dfz[channel+'Lumi'].sum() / dfz['OffLumi'].sum()
    if args.absolute:
        normalisation = 1

    arr_bins      = array('d', dfz.index)
    arr_zlumi     = array('d', dfz[channel+'Lumi'] / dfz['LBLive'] / normalisation)
    arr_zlumi_err = array('d', dfz[channel+'LumiErr'] / dfz['LBLive'] / normalisation)
    arr_rat       = array('d', dfz[channel+'Lumi'] / dfz['OffLumi'] / normalisation)
    arr_rat_err   = array('d', dfz[channel+'LumiErr'] / dfz['OffLumi'] / normalisation)
    arr_olumi     = array('d', dfz['OffLumi'] / dfz['LBLive'])
    print("normalisation = ")
    print(normalisation)

    hz = R.TGraphErrors(len(arr_bins), arr_bins, arr_zlumi, R.nullptr, arr_zlumi_err)
    ho = R.TGraphErrors(len(arr_bins), arr_bins, arr_olumi, R.nullptr, R.nullptr)
    hr = R.TGraphErrors(len(arr_bins), arr_bins, arr_rat, R.nullptr, arr_rat_err)

    ho.SetLineColor(R.kAzure)
    ho.SetLineWidth(3)

    return hz, hr, ho, arr_rat, arr_bins

def draw_plots(channel, hz_list, hr_list, ho, arr_rat, arr_bins):
        
    c1 = R.TCanvas()

    year = "23"
    lhc_fill = "8725"
    run_number = "451569"

    if channel == 'Zee':
        Lumi = 'ZeeLumi'
        LumiErr = 'ZeeLumiErr'
        LumiString = "L_{Z #rightarrow ee}"
        name = 'e'

    elif channel == 'Zmumu':
        Lumi = 'ZmumuLumi'
        LumiErr = 'ZmumuLumiErr'
        LumiString = "L_{Z #rightarrow #mu#mu}"
        name = '#mu'

    channel_string = "Z #rightarrow "+name+name

    if args.usemu: 
        leg = R.TLegend(0.5, 0.28, 0.75, 0.50)
        xtitle = "<#mu>"
        if args.absolute:
            outfile = outdir + run_number + "/" + "data"+year+"_"+run_number+"_"+channel+"_vs_atlas_mu_abs"
        else:
            outfile = outdir + run_number + "/" + "data"+year+"_"+run_number+"_"+channel+"_vs_atlas_mu"
    else: 
        leg = R.TLegend(0.5, 0.15, 0.75, 0.35)
        xtitle = "Luminosity Block Number"
        if args.absolute:
            outfile = outdir + run_number + "/" + "data"+year+"_"+run_number+"_"+channel+"_vs_atlas_abs"
        else:
            outfile = outdir + run_number + "/" + "data"+year+"_"+run_number+"_"+channel+"_vs_atlas"

    for hz in hz_list:

        hz.GetXaxis().SetLabelSize(0)

        xmin = hz.GetXaxis().GetXmin()
        xmax = hz.GetXaxis().GetXmax()

        if hz_list.index(hz) == 0:

            hz.SetMarkerStyle(24)
            hz.SetMarkerColor(1)
            hz.SetLineColor(1)
            hz.Draw('ap')

        else:

            hz.SetMarkerStyle(25)
            hz.SetMarkerColor(2)
            hz.SetLineColor(2)
            hz.Draw('same p')

        hz.GetYaxis().SetTitle("Luminosity [10^{33} cm^{-2}s^{-1}]")
        hz.GetXaxis().SetTitle(xtitle)
        hz.GetXaxis().SetTitleOffset(0.8)
        ymax = hz.GetHistogram().GetMaximum()
        ymin = hz.GetHistogram().GetMinimum()
        if not args.usemu:
            if args.absolute:
                hz.GetYaxis().SetRangeUser(0, 33)
            else: 
                #hz.GetYaxis().SetRangeUser(ymin-2, ymax*1.6)
                hz.GetYaxis().SetRangeUser(4, 12)
        leg.SetBorderSize(0)
        leg.SetTextSize(0.04)
        trigname = trignamelist[hz_list.index(hz)]
        if args.absolute:
            leg.AddEntry(hz, "L_{"+channel_string+"}", "ep")
        else:
            leg.AddEntry(hz, trigname+" - L_{"+channel_string+"}^{normalised to L_{ATLAS}^{fill}}", "ep")
        
    ho.Draw("same L")
    leg.AddEntry(ho, "L_{ATLAS}", "l")
    leg.Draw()

    #pt.drawAtlasLabel(0.2, 0.86, "Internal")
    pt.drawAtlasLabel(0.14, 0.86, "Work in Progress")
    if year in ['15', '16', '17', '18']:
        pt.drawText(0.14, 0.80, "Data 20" + year + ", #sqrt{s} = 13 TeV")
    else:
        pt.drawText(0.14, 0.80, "Data 20" + year + ", #sqrt{s} = 13.6 TeV")
    pt.drawText(0.14, 0.74, "LHC Fill " + lhc_fill)
    pt.drawText(0.14, 0.68,  channel_string + " counting")

    c1.SaveAs(outfile + ".eps")
    c1.SaveAs(outfile + ".pdf")

    # Plot ratio with fit
    c2 = R.TCanvas()

    for hr in hr_list:   

        hr.Draw('ap0')
        hr.Fit('pol0')
        hr.GetFunction('pol0').SetLineColor(R.kRed)

        hr.GetYaxis().SetTitle("Ratio")
        hr.GetXaxis().SetTitle(xtitle)
        hr.GetYaxis().SetRangeUser(0.9, 1.1)
        hr.GetYaxis().SetNdivisions()

        chi2 = hr.GetFunction('pol0').GetChisquare()
        ndf  = hr.GetFunction('pol0').GetNDF()    

        mean = hr.GetFunction("pol0").GetParameter(0)
        stdev = np.percentile(abs(arr_rat - np.median(arr_rat)), 68)
        print("####")
        print("stdev =", stdev)
        print("####")
        line0 = pt.make_bands(arr_bins, stdev, mean)
        line0.Draw("same 3")
        hr.GetFunction("pol0").Draw("same l")
        hr.Draw("same ep")

        leg = R.TLegend(0.17, 0.2, 0.90, 0.3)
        leg.SetBorderSize(0)
        leg.SetTextSize(0.05)
        leg.SetNColumns(3)
        leg.AddEntry(hr, LumiString+"/L_{ATLAS}", "ep")
        leg.AddEntry(hr.GetFunction("pol0"), "Mean = {:.3f}".format(round(mean, 3)), "l")
        leg.AddEntry(line0, "68% band", "f")
        leg.Draw()

    #pt.drawAtlasLabel(0.2, 0.86, "Internal")
    pt.drawAtlasLabel(0.2, 0.86, "Work in Progress")
    if year == "22" or year == "23":
        pt.drawText(0.2, 0.80, "Data 20" + year + ", #sqrt{s} = 13.6 TeV")
    else:
        pt.drawText(0.2, 0.80, "Data 20" + year + ", #sqrt{s} = 13 TeV")
    pt.drawText(0.2, 0.74, "LHC Fill " + lhc_fill)
    pt.drawText(0.2, 0.68,  channel_string + " counting")

    c2.SaveAs(outfile+"_ratio.eps")
    c2.SaveAs(outfile+"_ratio.pdf")
    
    #return arr_zlumi, arr_zlumi_err

    # Zee / Zmumu comparison

"""
def plot_ratio(zeelumi, zeelumierr, zmumulumi, zmumulumierr):
    dfz = pd.read_csv(infilename, delimiter=',')
    run_number = str(int(dfz.RunNum[0]))
    lhc_fill   = str(int(dfz.FillNum[0]))
    

     #Calculate mean per LB against ATLAS
    dfz = dfz.drop(dfz[(dfz['LBLive']<10) | (dfz['PassGRL']==0)].index)

    # Scale by livetime
    for entry in ['ZeeLumi','ZmumuLumi','ZeeLumiErr','ZmumuLumiErr','OffLumi']:  
        dfz[entry] *= dfz['LBLive']

    # Square uncertainties
    dfz['ZeeLumiErr'] *= dfz['ZeeLumiErr']
    dfz['ZmumuLumiErr'] *= dfz['ZmumuLumiErr']

    # Merge by groups of 20 LBs or pileup bins
    dfz['LBNum'] = (dfz['LBNum']//20)*20
    dfz = dfz.groupby(['LBNum']).sum()
    
    dfz['ZeeLumiErr']   = np.sqrt(dfz['ZeeLumiErr'])
    dfz['ZmumuLumiErr'] = np.sqrt(dfz['ZmumuLumiErr'])


    print("Doing channel comparison!")
    dfz = dfz.drop(dfz[(dfz.ZeeLumi == 0) | (dfz.ZmumuLumi == 0)].index)
    arr_bins      = array('d', dfz.index)
    arr_rat       = array('d', dfz['ZeeLumi'] / dfz['ZmumuLumi'])
    arr_rat_err   = array('d', (dfz['ZeeLumi'] / dfz['ZmumuLumi']) * np.sqrt(pow(dfz['ZeeLumiErr']/dfz['ZeeLumi'], 2) + pow(dfz['ZmumuLumiErr']/dfz['ZmumuLumi'], 2)))

    c1 = R.TCanvas()
    gr = R.TGraphErrors(len(arr_rat), arr_bins, arr_rat, R.nullptr, arr_rat_err)
    gr.Draw("ap")
    gr.GetXaxis().SetTitle("Luminosity Block")
    gr.GetYaxis().SetTitle("L_{Z #rightarrow ee} / L_{Z #rightarrow #mu#mu}")
    if year == "15": 
        gr.GetYaxis().SetRangeUser(0.6, 1.4)
    else: 
        gr.GetYaxis().SetRangeUser(0.8, 1.2)
    gr.Fit("pol0", "0")
    gr.GetFunction("pol0").SetLineColor(R.kRed)
   
    mean = gr.GetFunction("pol0").GetParameter(0)
    stdev = np.percentile(abs(arr_rat - np.median(arr_rat)), 68)
    print("####")
    print("stdev =", stdev)
    print("####")
    line1 = pt.make_bands(arr_bins, stdev, mean)
    line1.Draw("same 3")
    gr.GetFunction("pol0").Draw("same l")
    gr.Draw("same ep")

    latex = R.TLatex()
    #R.ATLASLabel(0.2, 0.86, "Internal")
    R.ATLASLabel(0.2, 0.86, "Work in Progress")
    if year in ['15', '16', '17', '18']:
        latex.DrawLatexNDC(0.2, 0.80, "Data 20" +year+ ", #sqrt{s} = 13 TeV")
    else:
        latex.DrawLatexNDC(0.2, 0.80, "Data 20" +year+ ", #sqrt{s} = 13.6 TeV")
    latex.DrawLatexNDC(0.2, 0.74, "LHC Fill " + lhc_fill)

    chi2 = gr.GetFunction('pol0').GetChisquare()
    ndf  = gr.GetFunction('pol0').GetNDF()
    print ("####")
    if ndf == 0:
        print("NDf = 0, so cannot calculate Chi2/NDf")
    else:
        print ("chi2 =", chi2/ndf)
    print ("####")

    leg = R.TLegend(0.17, 0.2, 0.90, 0.3)
    leg.SetBorderSize(0)
    leg.SetTextSize(0.05)
    leg.SetNColumns(3)
    leg.AddEntry(gr, "L_{Z #rightarrow ee}/L_{Z #rightarrow #mu#mu}", "ep")
    leg.AddEntry(gr.GetFunction("pol0"), "Mean = {:.3f}".format(round(mean, 3)), "l")
    leg.AddEntry(line1, "68% band", "f")
    leg.Draw()

    c1.SaveAs(outdir + run_number + "/" + "data"+year+"_"+run_number+"_zeezmmratio.eps")
    c1.SaveAs(outdir + run_number + "/" + "data"+year+"_"+run_number+"_zeezmmratio.pdf")
"""

if __name__ == "__main__":
    main()
