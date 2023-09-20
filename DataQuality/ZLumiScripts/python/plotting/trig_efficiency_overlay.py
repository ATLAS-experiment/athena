#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

import numpy as np
import pandas as pd
import ROOT as R
import argparse 
from array import array
from math import sqrt
import python_tools as pt
import re

parser = argparse.ArgumentParser()
#parser.add_argument('--infile', type=str, help='input file')
parser.add_argument('--outdir', type=str, help='output directory')
parser.add_argument('--usemu', action='store_true', help='Plot vs. mu. Default == LB')

args = parser.parse_args()
#infilename = args.infile
outdir = args.outdir

#substring = "data"
#pattern = re.compile(substring + r"(\d{2})")
#match = pattern.search(infilename)
infilelist = ["/eos/home-j/jnewell/Trig_Changes/data23_13p6TeV/run_451569_old_trig/run_451569.csv", "/eos/home-j/jnewell/Trig_Changes/data23_13p6TeV/run_451569_new_trig/run_451569.csv"]
trignamelist = ["Legacy L1", "Upgrade L1"]

#if match:
#    year = match.group(1)
#else:
#    print("Year not found")

year = "23"

def main():

    Zee_plotlist = []
    Zmumu_plotlist = []
    
    for infile in infilelist:
        Zee_plotlist.append(plot_trig('Zee', infile))
        Zmumu_plotlist.append(plot_trig('Zmumu', infile))

    draw_plots('Zee', Zee_plotlist)

def plot_trig(channel, infilename):

    dfz = pd.read_csv(infilename, delimiter=",")
    dfz_small = dfz
    dfz_small['ZLumi']    = dfz_small[channel + 'Lumi']
    dfz_small['ZLumiErr'] = dfz_small[channel + 'LumiErr']
    dfz_small['EffTrig']  = dfz_small[channel + 'EffTrig']
    dfz_small['ErrTrig']  = dfz_small[channel + 'ErrTrig']

    dfz_small = dfz_small.drop(dfz_small[dfz_small.ZLumi == 0].index)
    if channel == "Zee": 
        channel_string = "Z #rightarrow ee"
        lep = "e"
        ymin = 0.65
        if year == "15": 
            ymax = 0.55
            leg = R.TLegend(0.7, 0.37, 0.8, 0.6)
        else: 
            ymax = 0.88
            leg = R.TLegend(0.7, 0.7, 0.8, 0.93)
    elif channel == "Zmumu": 
        channel_string = "Z #rightarrow #mu#mu"
        lep = "#mu"
        leg = R.TLegend(0.7, 0.47, 0.8, 0.7)
        ymax = 0.65
        ymin = 0.65

    dict_mu       = {}
    dict_trig     = {}
    dict_trig_err = {}

    for index, event in dfz_small.iterrows():
        run_number = str(int(event.RunNum))
        lhc_fill   = str(int(event.FillNum))
        if args.usemu: 
            pileup = int(event.OffMu)
        else: 
            pileup = (event.LBNum // 20)*20

        if event.ErrTrig == 0.0: 
            continue

        weight_trig = 1/pow(event.ErrTrig, 2)

        if pileup not in dict_mu: 
            dict_mu[pileup] = pileup
            dict_trig[pileup] = weight_trig * event.EffTrig
            dict_trig_err[pileup] = weight_trig 
        else:
            dict_trig[pileup] += weight_trig * event.EffTrig
            dict_trig_err[pileup] += weight_trig

    vec_trig     = array('d')
    vec_trig_err = array('d')
    vec_mu       = array('d')

    if not dict_mu:
        print("File "+infilename+ " has no filled lumi blocks!")
        return

    for pileup in dict_mu:
        trig_weighted_average = dict_trig[pileup]/dict_trig_err[pileup]
        trig_error = sqrt(1/dict_trig_err[pileup])
        vec_trig.append(trig_weighted_average)
        vec_trig_err.append(trig_error)

        vec_mu.append(pileup)

    if infilelist.index(infilename) == 0:

        trig_graph= R.TGraphErrors(len(vec_trig), vec_mu, vec_trig, R.nullptr, vec_trig_err);
        trig_graph.GetHistogram().SetYTitle("Efficiency")
        trig_graph.GetHistogram().GetYaxis().SetRangeUser(ymin, 1.0)
        trig_graph.SetMarkerSize(1)

    else:

        trig_graph= R.TGraphErrors(len(vec_trig), vec_mu, vec_trig, R.nullptr, vec_trig_err);
        trig_graph.GetHistogram().GetYaxis().SetRangeUser(ymin, 1.0)
        trig_graph.SetMarkerSize(1)
        trig_graph.SetMarkerStyle(21)
        trig_graph.SetMarkerColor(R.kRed)
        trig_graph.SetLineColor(R.kRed)

    return trig_graph

def draw_plots(channel, plot_list):
    
    c1 = R.TCanvas()

    year = "23"

    if channel == "Zee": 
        channel_string = "Z #rightarrow ee"
        lep = "e"
        ymin = 0.65
        if year == "15": 
            ymax = 0.55
            leg = R.TLegend(0.7, 0.30, 0.8, 0.5)
        else: 
            ymax = 0.88
            leg = R.TLegend(0.65, 0.7, 0.8, 0.93)
    elif channel == "Zmumu": 
        channel_string = "Z #rightarrow #mu#mu"
        lep = "#mu"
        leg = R.TLegend(0.7, 0.47, 0.8, 0.7)
        ymax = 0.65
        ymin = 0.65

    leg.SetBorderSize(0)
    leg.SetTextSize(0.03)

    for plot in plot_list:

        if plot_list.index(plot) == 0:
            
            plot.Draw("ap")

        else: 
            
            plot.Draw("p")

        trigname = trignamelist[plot_list.index(plot)]
        leg.AddEntry(plot, trigname + " #varepsilon_{trig}^{single-"+lep+"}", "ep")

        if args.usemu: 
            plot.GetHistogram().SetXTitle("<#mu>")
        else: 
            plot.GetHistogram().SetXTitle("Luminosity Block Number")
    year = "23"
    lhc_fill = "8725"
    run_number = "451569"

    pt.drawAtlasLabel(0.2, ymax, "Work In Progress")
    pt.drawText(0.2, ymax-0.06, "Data 20" + year + ", #sqrt{s} = 13.6 TeV")
    pt.drawText(0.2, ymax-0.12, "LHC Fill " + lhc_fill)
    pt.drawText(0.2, ymax-0.18, channel_string + " counting")
    leg.Draw()

    if args.usemu:
        c1.SaveAs(outdir +run_number+ "/eff_v_mu_"+channel+"_data"+year+"_"+run_number+".eps")
        c1.SaveAs(outdir +run_number+ "/eff_v_mu_"+channel+"_data"+year+"_"+run_number+".pdf")
    else:
        c1.SaveAs(outdir +run_number+ "/eff_v_lb_"+channel+"_data"+year+"_"+run_number+".eps")
        c1.SaveAs(outdir +run_number+ "/eff_v_lb_"+channel+"_data"+year+"_"+run_number+".pdf")

if __name__ == "__main__":
    R.gROOT.SetBatch(R.kTRUE)
    pt.setAtlasStyle()
    main()
