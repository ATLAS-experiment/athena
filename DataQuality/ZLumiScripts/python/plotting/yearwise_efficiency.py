#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

"""
Plot trigger and reconstruction efficiencies over entire data-periods.
"""

import numpy as np
import ROOT as R
import python_tools as pt
import ZLumiScripts.tools.zlumi_mc_cf as dq_cf
import math
from array import array
import argparse
    
parser = argparse.ArgumentParser()
parser.add_argument('--year', type=str, help='A year number (15-25) or run3 or year1_year2_...')
parser.add_argument('--channel', type=str, help='Zee or Zmumu')
parser.add_argument('--indir', type=str, help='Input directory for CSV files')
parser.add_argument('--outdir', type=str, help='Output directory for plots')

args = parser.parse_args()
year = args.year
channel = args.channel
indir = args.indir
outdir = args.outdir

if year == "run3": 
    years = ["22", "23", "24", "25"]
    out_tag = "run3"
    time_format = "%m/%y"
    xtitle = 'Month / Year'
    date_tag = "Run 3,#kern[-0.5]{ }#sqrt{s} = 13.6 TeV"
    labelsize = 55
    multiyear = True
elif len(year.split("_")) > 1: 
    years = year.split("_")
    out_tag = "data"+year
    time_format = "%m/%y"
    xtitle = 'Month / Year'
    date_tag = "Run 3,#kern[-0.5]{ }#sqrt{s} = 13.6 TeV"
    labelsize = 55
    multiyear = True
else: 
    years = [year]
    out_tag = "data"+year
    time_format = "%d/%m"
    xtitle = 'Date in 20' + year
    date_tag = "Data 20" + year  + ",#kern[-0.5]{ }#sqrt{s} = 13.6 TeV"
    labelsize = 22
    multiyear = False

def main():
    plot_efficiency(channel, years)
    plot_efficiency_comb(channel, years)

def plot_efficiency_comb(channel, years):

    arr_date  = []
    arr_combeff = []
    arr_comberr = []
    run_num   = []
    
    for year in years:
        grl = pt.get_grl(year)

        for run in grl:
            livetime, zlumi, zerr, olumi, timestamp, dfz_small = pt.get_dfz(args.indir, year, run, channel)
            # Cut out short runs
            if livetime < pt.lblivetimecut:
                if livetime >= 0.: print(f"Skip Run {run} because of live time {livetime/60:.1f} min")
                continue

            dfz_small['CombEff'] = dfz_small[channel + 'EffComb']
            dfz_small['CombErr'] = dfz_small[channel + 'ErrComb']
            dfz_small['OffMu'] = dfz_small['OffMu']

            # Scale event-level efficiency with FMC
            campaign = "mc23a"
            dfz_small['CombEff'] *= dq_cf.correction(dfz_small['OffMu'], channel, campaign, int(run))
            dfz_small['CombErr'] *= dq_cf.correction(dfz_small['OffMu'], channel, campaign, int(run))
        
            # Calculate average event-level efficiency
            dfz_small['CombEff'] *= dfz_small['LBLive']
            comb_eff_avg = dfz_small['CombEff'].sum()/livetime

            # Calculate average trigger efficiency error
            dfz_small['CombErr'] *= dfz_small['LBLive']
            dfz_small['CombErr'] *= dfz_small['CombErr']
            comb_err_avg = math.sqrt(dfz_small['CombErr'].sum())/livetime
            
            arr_date.append(timestamp)
            arr_combeff.append(comb_eff_avg)
            arr_comberr.append(comb_err_avg)
            run_num.append(run)

    arr_date = array('d', arr_date)

    arr_combeff = np.array(arr_combeff)
    arr_comberr = np.array(arr_comberr)

    if channel == "Zee":
        ymin, ymax = 0.56, 0.74
    elif channel == "Zmumu": 
        ymin, ymax = 0.74, 0.80

    comb_graph = R.TGraphErrors(len(arr_date), arr_date, arr_combeff, R.nullptr,arr_comberr)
    comb_graph.GetHistogram().SetYTitle("Efficiency")
    comb_graph.GetHistogram().GetYaxis().SetRangeUser(ymin, ymax)
    comb_graph.GetXaxis().SetTimeDisplay(2)
    comb_graph.GetXaxis().SetNdivisions(9,R.kFALSE)
    comb_graph.GetXaxis().SetTimeFormat(time_format)
    comb_graph.GetXaxis().SetTimeOffset(0,"gmt")
    comb_graph.SetMarkerSize(1)

    if multiyear:
        c1 = R.TCanvas("c1", "c1", 2000, 1200)
    else:
        c1 = R.TCanvas()

    comb_graph.Draw("ap")

    leg = R.TLegend(0.645, 0.4, 0.805, 0.6)
    if channel == "Zee":
        pt.drawAtlasLabel(0.2, ymax-0.06, "Internal")
    elif channel == "Zmumu":
        pt.drawAtlasLabel(0.2, ymax-0.4, "Internal")

    pt.drawText(0.2, ymax-0.46, date_tag, size=labelsize)
    pt.drawText(0.2, ymax-0.52, pt.plotlabel[channel] + " counting", size=labelsize)

    leg.SetBorderSize(0)
    leg.SetTextSize(0.07)
    leg.AddEntry(comb_graph, "#varepsilon_{event}^{"+pt.plotlabel[channel]+"}", "ep")
    
    leg.Draw()

    comb_graph.GetHistogram().SetXTitle("Date")
    c1.SaveAs(outdir + channel + "_eventeff_vs_time_"+out_tag+".pdf")

def plot_efficiency(channel, years):

    print("Efficiency Plots vs Time for years: ", years)

    arr_date  = []
    arr_trigeff = []
    arr_trigerr = []
    arr_recoeff  = []
    arr_recoerr = []
    run_num   = []

    trigeff_vs_runlength = R.TH2D("trigeff_vs_runlength",\
                                  "Trigger efficiency vs. Run Length;Run Length [h]; Trigger Efficiency;N_{run}",\
                                  48, 0., 24., 80, .6, 1.0)
    recoeff_vs_runlength = R.TH2D("recoeff_vs_runlength",\
                                  "Reconstruction efficiency vs. Run Length;Run Length [h]; Reconstruction Efficiency;N_{run}",\
                                  48, 0., 24., 60, .8, 1.0)
    
    for year in years:
        grl = pt.get_grl(year)

        for run in grl:
            livetime, zlumi, zerr, olumi, timestamp, dfz_small = pt.get_dfz(args.indir, year, run, channel)
            # Cut out short runs
            if livetime < pt.lblivetimecut:
                if livetime >= 0.: print(f"Skip Run {run} because of live time {livetime/60:.1f} min")
                continue

            dfz_small['TrigEff'] = dfz_small[channel + 'EffTrig']
            dfz_small['TrigErr'] = dfz_small[channel + 'ErrTrig']
            dfz_small['RecoEff'] = dfz_small[channel + 'EffReco']
            dfz_small['RecoErr'] = dfz_small[channel + 'ErrReco']

            # Calculate average trigger efficiency
            dfz_small['TrigEff'] *= dfz_small['LBLive']
            trig_eff_avg = dfz_small['TrigEff'].sum()/livetime

            # Calculate average reconstruction efficiency
            dfz_small['RecoEff'] *= dfz_small['LBLive']
            reco_eff_avg = dfz_small['RecoEff'].sum()/livetime

            # Calculate average trigger efficiency error
            dfz_small['TrigErr'] *= dfz_small['LBLive']
            dfz_small['TrigErr'] *= dfz_small['TrigErr']
            trig_err_avg = math.sqrt(dfz_small['TrigErr'].sum())/livetime

            # Calculate average reconstruction efficiency error
            dfz_small['RecoErr'] *= dfz_small['LBLive']
            dfz_small['RecoErr'] *= dfz_small['RecoErr']
            reco_err_avg = math.sqrt(dfz_small['RecoErr'].sum())/livetime
            
            arr_date.append(timestamp)
            arr_trigeff.append(trig_eff_avg)
            arr_trigerr.append(trig_err_avg)
            arr_recoeff.append(reco_eff_avg)
            arr_recoerr.append(reco_err_avg)
            run_num.append(run)

            trigeff_vs_runlength.Fill(min(livetime/3600, 23.99), trig_eff_avg)
            recoeff_vs_runlength.Fill(min(livetime/3600, 23.99), reco_eff_avg)

    arr_date = array('d', arr_date)

    arr_trigeff = np.array(arr_trigeff)
    arr_trigerr = np.array(arr_trigerr)
    arr_recoeff = np.array(arr_recoeff)
    arr_recoerr = np.array(arr_recoerr)

    ymin, ymax = 0.61, 0.96
    if channel == "Zee": 
        lep = "e"
    elif channel == "Zmumu": 
        lep = "#mu"

    trig_graph = R.TGraphErrors(len(arr_date), arr_date, arr_trigeff, R.nullptr,arr_trigerr)
    trig_graph.GetHistogram().SetYTitle("Efficiency")
    trig_graph.GetHistogram().GetYaxis().SetRangeUser(ymin, ymax)
    trig_graph.GetXaxis().SetTimeDisplay(2)
    trig_graph.GetXaxis().SetNdivisions(9,R.kFALSE)
    trig_graph.GetXaxis().SetTimeFormat(time_format)
    trig_graph.GetXaxis().SetTimeOffset(0,"gmt")
    trig_graph.SetMarkerSize(1)
        
    reco_graph = R.TGraphErrors(len(arr_date), arr_date, arr_recoeff, R.nullptr,arr_recoerr)
    reco_graph.GetHistogram().GetYaxis().SetRangeUser(ymin, ymax)
    reco_graph.GetXaxis().SetTimeDisplay(2)
    reco_graph.GetXaxis().SetNdivisions(9,R.kFALSE)
    reco_graph.GetXaxis().SetTimeFormat(time_format)
    reco_graph.GetXaxis().SetTimeOffset(0,"gmt")
    reco_graph.SetMarkerSize(1)
    reco_graph.SetMarkerStyle(21)
    reco_graph.SetMarkerColor(R.kRed)
    reco_graph.SetLineColor(R.kRed)

    if multiyear:
        c1 = R.TCanvas("c1", "c1", 2000, 1200)
    else:
        c1 = R.TCanvas()

    trig_graph.Draw("ap")
    reco_graph.Draw("p")

    if channel == "Zee":
        leg = R.TLegend(0.645, 0.2, 0.805, 0.4)
        pt.drawAtlasLabel(0.2, ymax-0.64, "Internal")
        pt.drawText(0.2, ymax-0.70, date_tag, size=labelsize)
        pt.drawText(0.2, ymax-0.76, pt.plotlabel[channel] + " counting", size=labelsize)
    elif channel == "Zmumu":
        leg = R.TLegend(0.645, 0.45, 0.805, 0.65)
        pt.drawAtlasLabel(0.2, ymax-0.36, "Internal")
        pt.drawText(0.2, ymax-0.42, date_tag, size=labelsize)
        pt.drawText(0.2, ymax-0.48, pt.plotlabel[channel] + " counting", size=labelsize)

    leg.SetBorderSize(0)
    leg.SetTextSize(0.07)
    leg.AddEntry(reco_graph, "#varepsilon_{reco}^{single-"+lep+"}", "ep")
    leg.AddEntry(trig_graph, "#varepsilon_{trig}^{single-"+lep+"}", "ep")
    
    leg.Draw()
    
    trig_graph.GetHistogram().SetXTitle("Date")
    c1.SaveAs(outdir + channel + "_eff_vs_time_"+out_tag+".pdf")

    c1.SetRightMargin(0.15)
    trigeff_vs_runlength.Draw("colz")
    if channel == "Zee":
        ymin, ymax = 0.73, 0.95
    elif channel == "Zmumu":
        ymin, ymax = 0.6, 0.85
    trigeff_vs_runlength.GetYaxis().SetRangeUser(ymin, ymax)
    pt.drawAtlasLabel(0.2, 0.89, "Internal")
    pt.drawText(0.2, 0.83, date_tag, size=labelsize)
    pt.drawText(0.2, 0.77, pt.plotlabel[channel] + " counting", size=labelsize)
    c1.SaveAs(outdir + channel + "_trigeff_vs_runlength_"+out_tag+".pdf")

    recoeff_vs_runlength.Draw("colz")
    if channel == "Zee":
        ymin, ymax = 0.8, 0.95
    elif channel == "Zmumu":
        ymin, ymax = 0.92, 1.0
    recoeff_vs_runlength.GetYaxis().SetRangeUser(ymin, ymax)
    pt.drawAtlasLabel(0.2, 0.89, "Internal")
    pt.drawText(0.2, 0.83, date_tag, size=labelsize)
    pt.drawText(0.2, 0.77, pt.plotlabel[channel] + " counting", size=labelsize)
    c1.SaveAs(outdir + channel + "_recoeff_vs_runlength_"+out_tag+".pdf")
    

if __name__ == "__main__":
    pt.setAtlasStyle()
    R.gROOT.SetBatch(R.kTRUE)
    main()
