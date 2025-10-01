#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

"""
Plot trigger and reconstruction efficiencies over entire data-periods.
"""

import ROOT as R
import python_tools as pt
import ZLumiScripts.tools.zlumi_mc_cf as dq_cf
from math import sqrt
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
    ymin, ymax = 0.5, 1.1
    xtitle = 'Month / Year'
    date_tag = "Run 3, #sqrt{s} = 13.6 TeV"
    if channel is not None: 
        xval = 0.30
        yval = 0.33
    else:
        xval = 0.43
        yval = 0.33
    set_size = 1
elif len(year.split("_")) > 1: 
    years = year.split("_")
    out_tag = "data"+year
    time_format = "%m/%y"
    ymin, ymax = 0.5, 1.1
    xtitle = 'Month / Year'
    date_tag = "Run 3, #sqrt{s} = 13.6 TeV"
    if channel is not None: 
        xval = 0.30
        yval = 0.33
    else:
        xval = 0.43
        yval = 0.33
    set_size = 1
else: 
    years = [year]
    out_tag = "data"+year
    time_format = "%d/%m"
    ymin, ymax = 0.5, 1.1
    xtitle = 'Date in 20' + year
    date_tag = "Data 20" + year  + ", #sqrt{s} = 13.6 TeV"
    xval = 0.235
    yval = 0.86
    set_size = 0

def main():
    plot_efficiency_comb(channel, years)

def plot_efficiency_comb(channel, years):

    all_graphs = []

    for year in years:
        dict_comb = {}
        dict_comb_err = {}
        dict_mu = {}    
        
        vec_comb = array('d')
        vec_comb_err = array('d')
        vec_mu = array('d')

        grl = pt.get_grl(year)

        for run in grl:
            livetime, zlumi, zerr, olumi, timestamp, dfz_small = pt.get_dfz(args.indir, year, run, channel)

            # Cut out short runs
            if livetime < pt.runlivetimecut:
                if livetime >= 0.: print(f"Skip Run {run} because of live time {livetime/60:.1f} min")
                continue

            dfz_small['CombEff'] = dfz_small[channel + 'EffComb']
            dfz_small['CombErr'] = dfz_small[channel + 'ErrComb']
            # safety to avoid divide by zero - maybe can be removed again later
            dfz_small.loc[dfz_small['CombErr'] == 0., 'CombErr'] = 1.

            # Scale event-level efficiency with FMC
            campaign = "mc23a"
            dfz_small['CombEff'] *= dq_cf.correction(dfz_small['OffMu'], channel, campaign, int(run))
            dfz_small['CombErr'] *= dq_cf.correction(dfz_small['OffMu'], channel, campaign, int(run))

            for index, event in dfz_small.iterrows():
                pileup = int(event.OffMu)

                weight_comb = 1/pow(event.CombErr, 2)
                if pileup not in dict_mu: 
                    dict_mu[pileup] = pileup
                    dict_comb[pileup] = weight_comb * event.CombEff
                    dict_comb_err[pileup] = weight_comb 
                else:
                    dict_comb[pileup] += weight_comb * event.CombEff
                    dict_comb_err[pileup] += weight_comb

                if not dict_mu:
                    print("File has no filled lumi blocks!")
                    return

        for pileup in dict_mu:
            comb_weighted_average = dict_comb[pileup]/dict_comb_err[pileup]
            comb_error = sqrt(1/dict_comb_err[pileup])
            vec_comb.append(comb_weighted_average)
            vec_comb_err.append(comb_error)        
            
            vec_mu.append(pileup)

        all_graphs.append((year, R.TGraphErrors(len(vec_comb), vec_mu, vec_comb, R.nullptr, vec_comb_err)))

    # now draw all
    c1 = R.TCanvas()
    leg = R.TLegend(0.645, 0.7, 0.805, 0.9)
    if channel == "Zee":
        ymin, ymax = 0.52, 0.74
    elif channel == "Zmumu":
        ymin, ymax = 0.74, 0.84
    xmin, xmax = 0, 80

    for igraph in range(len(all_graphs)):
        comb_graph = all_graphs[igraph][1]
        comb_graph.SetMarkerSize(1)
        comb_graph.SetMarkerColor(R.kBlack+igraph)
        comb_graph.SetMarkerStyle(R.kFullCircle+igraph)
        if igraph == 0:
            comb_graph.Draw("ap")
            comb_graph.GetHistogram().SetYTitle("#varepsilon_{event}^{"+pt.plotlabel[channel]+"}#times F^{MC}")
            comb_graph.GetHistogram().GetYaxis().SetRangeUser(ymin, ymax)
            comb_graph.GetHistogram().GetXaxis().SetLimits(xmin, xmax)
            comb_graph.GetHistogram().SetXTitle("Pileup (#mu)")
        else:
            comb_graph.Draw("p same")
        leg.SetBorderSize(0)
        leg.SetTextSize(0.07)
        leg.AddEntry(comb_graph, "Data 20"+all_graphs[igraph][0], "ep")
    leg.Draw()

    if channel == "Zee":
        pt.drawAtlasLabel(0.6, ymax-0.46, "Internal")
        pt.drawText(0.2, ymax-0.46, date_tag)
        pt.drawText(0.2, ymax-0.52, pt.plotlabel[channel] + " counting")
    elif channel == "Zmumu":
        pt.drawAtlasLabel(0.6, ymax-0.56, "Internal")
        pt.drawText(0.2, ymax-0.62, pt.plotlabel[channel] + " counting")
        
    c1.SaveAs(outdir + channel + "_eventeff_vs_mu_"+out_tag+".pdf")

if __name__ == "__main__":
    pt.setAtlasStyle()
    R.gROOT.SetBatch(R.kTRUE)
    main()
