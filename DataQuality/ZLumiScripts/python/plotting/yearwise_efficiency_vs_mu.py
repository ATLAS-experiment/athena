#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

"""
Plot trigger and reconstruction efficiencies over entire data-periods.
"""

import pandas as pd
import ROOT as R
import python_tools as pt
import ZLumiScripts.tools.zlumi_mc_cf as dq_cf
from math import sqrt
from array import array
import argparse
    
parser = argparse.ArgumentParser()
parser.add_argument('--year', type=str, help='15-18, all for full Run-2')
parser.add_argument('--channel', type=str, help='Zee or Zmumu')
parser.add_argument('--indir', type=str, help='Input directory for CSV files')
parser.add_argument('--outdir', type=str, help='Output directory for plots')
parser.add_argument('--dir_2022', type=str, help='Input directory for 2022 data')
parser.add_argument('--dir_2023', type=str, help='Input directory for 2023 data')

args    = parser.parse_args()
year    = args.year
channel = args.channel
indir = args.indir
outdir = args.outdir
dir_2022 = args.dir_2022
dir_2023 = args.dir_2023

if year == "run3": 
    years = ["22", "23"]
    out_tag = "_run3"
    time_format = "%m/%y"
    ymin, ymax = 0.5, 1.1
    xtitle = 'Month / Year'
    date_tag = "Run 3, #sqrt{s} = 13.6 TeV"
    norm_type = "Run3"
    if channel is not None: 
        xval = 0.30
        yval = 0.33
    else:
        xval = 0.43
        yval = 0.33
    set_size = 1
else: 
    years = [year]
    out_tag = year
    time_format = "%d/%m"
    ymin, ymax = 0.5, 1.1
    xtitle = 'Date in 20' + year
    date_tag = "Data 20" + year  + ", #sqrt{s} = 13.6 TeV"
    norm_type = "year"
    xval = 0.235
    yval = 0.86
    set_size = 0

def main():
    plot_efficiency_comb(channel, years)

def plot_efficiency_comb(channel, years):

    graph_dict = {}
    c1 = R.TCanvas()

    if channel == "Zee":
        
        leg = R.TLegend(0.645, 0.7, 0.805, 0.9)

    elif channel == "Zmumu":
        
        leg = R.TLegend(0.645, 0.7, 0.805, 0.9)
    
    for year in years:

        dict_comb = {}
        dict_comb_err = {}
        dict_mu = {}    
        
        vec_comb = array('d')
        vec_comb_err = array('d')
        vec_mu = array('d')

        print("year = ", year)

        if year == "23":

            grl = []
            maindir = args.indir + dir_2023

            grl = pt.get_grl(year)

            print("grl = ")
            print(grl)

        elif year == "22":

            grl = []
            maindir = args.indir + dir_2022

            grl = pt.get_grl(year)

            print("grl = ")
            print(grl)

        for run in grl:

            print('Begin Run ', run, 'processing')

            dfz = pd.read_csv(maindir + "run_" + run + ".csv")
            dfz_small = dfz
            dfz_small['ZLumi']    = dfz_small[channel + 'Lumi']
            dfz_small['ZLumiErr'] = dfz_small[channel + 'LumiErr']
            dfz_small['CombEff'] = dfz_small[channel + 'EffComb']
            dfz_small['CombErr'] = dfz_small[channel + 'ErrComb']
            dfz_small['LBLive'] = dfz_small['LBLive']
            dfz_small['OffMu'] = dfz_small['OffMu']
            dfz_small = dfz_small.drop(dfz_small[dfz_small.ZLumi == 0].index)
            dfz_small = dfz_small.drop(dfz_small[(dfz_small['LBLive']<10) | (dfz_small['PassGRL']==0)].index)

            #Cut out all runs shorter than 40 minutes
            if dfz_small['LBLive'].sum()/60 < 40:
                print("Skip Run", run, "because of live time", dfz_small['LBLive'].sum()/60, "min")
                continue

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

        if channel == "Zee":
            channel_string = "Z #rightarrow ee"
            ymin, ymax = 0.52, 0.74
            xmin, xmax = 0, 80
        elif channel == "Zmumu":
            channel_string = "Z #rightarrow #mu#mu"
            ymin, ymax = 0.74, 0.84
            xmin, xmax = 0, 80

        if year == "22":
            
            comb_graph_22 = R.TGraphErrors(len(vec_comb), vec_mu, vec_comb, R.nullptr, vec_comb_err)
            comb_graph_22.GetHistogram().SetYTitle("#varepsilon_{event}^{"+channel_string+"}#times F^{MC}")
            comb_graph_22.GetHistogram().GetYaxis().SetRangeUser(ymin, ymax)
            comb_graph_22.GetHistogram().GetXaxis().SetLimits(xmin, xmax)
            comb_graph_22.SetMarkerSize(1)
            comb_graph_22.GetHistogram().SetXTitle("Pileup (#mu)")
            comb_graph_22.Draw("ap")
            leg.SetBorderSize(0)
            leg.SetTextSize(0.07)
            leg.AddEntry(comb_graph_22, "Data 20"+year, "ep")


        if year == "23":
            
            comb_graph_23 = R.TGraphErrors(len(vec_comb), vec_mu, vec_comb, R.nullptr, vec_comb_err)
            comb_graph_23.GetHistogram().SetYTitle("#varepsilon_{event}^{"+channel_string+"}#times F^{MC}")
            comb_graph_23.GetHistogram().GetYaxis().SetRangeUser(ymin, ymax)
            comb_graph_23.GetHistogram().GetXaxis().SetLimits(xmin, xmax)
            comb_graph_23.SetMarkerSize(1)
            comb_graph_23.Draw("samep")
            comb_graph_23.SetMarkerColor(R.kRed)
            leg.SetBorderSize(0)
            leg.SetTextSize(0.07)
            leg.AddEntry(comb_graph_23, "Data 20"+year, "ep")

    if channel == "Zee":

        pt.drawAtlasLabel(0.6, ymax-0.46, "Internal")
        if year in ['15', '16', '17', '18']:
            pt.drawText(0.2, ymax-0.46, date_tag)
        else:
            pt.drawText(0.2, ymax-0.46, date_tag)
        pt.drawText(0.2, ymax-0.52, channel_string + " counting")

    elif channel == "Zmumu":

        pt.drawAtlasLabel(0.6, ymax-0.56, "Internal")
        if year in ['15', '16', '17', '18']:
            pt.drawText(0.2, ymax-0.56, date_tag)
        else:
            pt.drawText(0.2, ymax-0.56, date_tag)
        pt.drawText(0.2, ymax-0.62, channel_string + " counting")

    leg.Draw()
    c1.SaveAs(outdir + "event_eff_v_mu_"+channel+"_data"+out_tag+"_"+".eps")
    c1.SaveAs(outdir + "event_eff_v_mu_"+channel+"_data"+out_tag+"_"+".pdf")

if __name__ == "__main__":
    pt.setAtlasStyle()
    R.gROOT.SetBatch(R.kTRUE)
    main()
