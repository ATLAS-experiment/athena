#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

import pandas as pd
import ROOT as R
import argparse 
from array import array
from math import sqrt
import python_tools as pt

parser = argparse.ArgumentParser()
parser.add_argument('--infile', type=str, help='input file')
parser.add_argument('--outdir', type=str, help='output directory')
parser.add_argument('--usemu', action='store_true', help='Plot vs. mu. Default == LB')

args = parser.parse_args()
infilename = args.infile
outdir = args.outdir

def main():
    plot_channel('Zee')
    plot_channel('Zmumu')

def plot_channel(channel):
    dfz = pd.read_csv(infilename, delimiter=",")
    if dfz.empty:
        print('No data in file', infilename, ', exiting.')
        return

    run_number = dfz.RunNum[0]
    lhc_fill   = dfz.FillNum[0]

    EffTrig = channel + 'EffTrig'
    EffReco = channel + 'EffReco'
    ErrTrig = channel + 'ErrTrig'
    ErrReco = channel + 'ErrReco'
    
    dfz = dfz.drop(dfz[(dfz[channel+'Lumi'] == 0)].index)
    dfz = dfz.drop(dfz[(dfz['LBLive']<pt.lblivetimecut) | (dfz['PassGRL']==0)].index)
    if len(dfz) == 0:
        print("No valid LBs found. Exiting")
        return

    if channel == "Zee": 
        channel_string = "Z#kern[-0.5]{ }#rightarrow#kern[-0.5]{ }ee"
        lep = "e"
        leg = R.TLegend(0.7, 0.7, 0.85, 0.9)
        ymin = 0.75
        ylabel = 0.88
    elif channel == "Zmumu": 
        channel_string = "Z#kern[-0.5]{ }#rightarrow#kern[-0.5]{ }#mu#mu"
        lep = "#mu"
        ymin = 0.55
        ylabel = 0.7
        leg = R.TLegend(0.7, 0.55, 0.85, 0.75)
    else:
        print("Wrong channel, exit.")
        exit(1)

    dict_mu       = {}
    dict_trig     = {}
    dict_trig_err = {}
    dict_reco     = {}
    dict_reco_err = {}
    
    for index, event in dfz.iterrows():
        if args.usemu: 
            pileup = int(event.OffMu)
        else: 
            pileup = (event.LBNum // 20)*20

        if event[ErrTrig] == 0.0 or event[ErrReco] == 0.0: 
            continue

        weight_trig = 1/pow(event[ErrTrig], 2)
        weight_reco = 1/pow(event[ErrReco], 2)
        if pileup not in dict_mu: 
            dict_mu[pileup] = pileup
            dict_trig[pileup] = weight_trig * event[EffTrig]
            dict_reco[pileup] = weight_reco * event[EffReco]
            dict_trig_err[pileup] = weight_trig 
            dict_reco_err[pileup] = weight_reco 
        else:
            dict_trig[pileup] += weight_trig * event[EffTrig]
            dict_reco[pileup] += weight_reco * event[EffReco]
            dict_trig_err[pileup] += weight_trig 
            dict_reco_err[pileup] += weight_reco 

    vec_trig     = array('d')
    vec_trig_err = array('d')
    vec_reco     = array('d')
    vec_reco_err = array('d')
    vec_mu       = array('d')
        
    if not dict_mu:
        print("File "+infilename+ " has no filled lumi blocks!")
        return

    for pileup in dict_mu:
        trig_weighted_average = dict_trig[pileup]/dict_trig_err[pileup]
        trig_error = sqrt(1/dict_trig_err[pileup])
        vec_trig.append(trig_weighted_average)
        vec_trig_err.append(trig_error)
            
        reco_weighted_average = dict_reco[pileup]/dict_reco_err[pileup]
        reco_error = sqrt(1/dict_reco_err[pileup])
        vec_reco.append(reco_weighted_average)
        vec_reco_err.append(reco_error)
        
        vec_mu.append(pileup)
            
    trig_graph = R.TGraphErrors(len(vec_trig), vec_mu, vec_trig, R.nullptr, vec_trig_err)
    trig_graph.GetHistogram().SetYTitle("Efficiency")
    trig_graph.GetHistogram().GetYaxis().SetRangeUser(ymin, 1.0)
    trig_graph.SetMarkerSize(1)

    reco_graph = R.TGraphErrors(len(vec_reco), vec_mu, vec_reco, R.nullptr, vec_reco_err)
    reco_graph.GetHistogram().GetYaxis().SetRangeUser(ymin, 1.0)
    reco_graph.SetMarkerSize(1)
    reco_graph.SetMarkerStyle(21)
    reco_graph.SetMarkerColor(R.kRed)
    reco_graph.SetLineColor(R.kRed)

    leg.SetBorderSize(0)
    leg.SetTextSize(0.07)
    leg.AddEntry(reco_graph, "#varepsilon_{reco}^{single-"+lep+"}", "ep")
    leg.AddEntry(trig_graph, "#varepsilon_{trig}^{single-"+lep+"}", "ep")

    c1 = R.TCanvas()
    trig_graph.Draw("ap")
    reco_graph.Draw("p")
        
    pt.drawAtlasLabel(0.2, ylabel, "Internal")
    pt.drawText(0.2, ylabel-0.05, pt.get_yearsqrtstxt(run_number), size=22)
    pt.drawText(0.2, ylabel-0.1, "LHC Fill " + str(lhc_fill), size=22)
    pt.drawText(0.2, ylabel-0.15, channel_string + " counting", size=22)
    leg.Draw()

    if args.usemu: 
        trig_graph.GetHistogram().SetXTitle("<#mu>")
        c1.SaveAs(outdir+"/"+channel+"_eff_vs_mu.pdf")
    else: 
        trig_graph.GetHistogram().SetXTitle("Luminosity Block Number")
        c1.SaveAs(outdir+"/"+channel+"_eff_vs_lb.pdf")

if __name__ == "__main__":
    R.gROOT.SetBatch(R.kTRUE)
    pt.setAtlasStyle()
    main()
