#!/usr/bin/env python3
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

import numpy as np
import pandas as pd
from . import python_tools as pt # for T0 running
#import python_tools as pt # for local running
import ROOT as R
import os
from array import array
import argparse


parser = argparse.ArgumentParser()
parser.add_argument('--infile', type=str, help='input file')
parser.add_argument('--outdir', type=str, help='output directory')
parser.add_argument('--usemu', action='store_true', help='Plot vs. mu. Default == LB')
parser.add_argument('--absolute', action='store_true', help='Use for official lumi absolute comparison')
parser.add_argument('--t0', action='store_true', help='Modifications for t0 operation')
parser.add_argument('--dropzero', action='store_true', help='Drop LBs with zero Z counting lumi in either channel')

args = parser.parse_args()
infilename = args.infile
outdir = args.outdir

def main():
    plot_channel('Zmumu')
    plot_channel('Zee')
    plot_channel('Zll')
    plot_ratio()

def plot_channel(channel):
    dfz = pd.read_csv(infilename, delimiter=',')
    if dfz.empty:
        print('No data in file', infilename, ', exiting.')
        return

    run_number = dfz.RunNum[0]
    lhc_fill   = dfz.FillNum[0]

    if args.t0:
        rfo = R.TFile.Open(os.path.join(outdir, 'zlumi.root'), 'RECREATE')

    Lumi = channel+'Lumi'
    LumiErr = channel+'LumiErr'
    LumiString = pt.plotlabel[channel]
        
    # Drop LBs with no Z-counting information or short livetime
    dfz0 = dfz.copy()
    if args.dropzero:
        dfz = dfz.drop(dfz[(dfz[Lumi] == 0)].index)
    dfz = dfz.drop(dfz[(dfz['LBLive']<pt.lblivetimecut) | (dfz['PassGRL']==0)].index)
    if len(dfz) == 0:
        print("No valid LBs found. Exiting")
        return

    dfz['OffDelLumi'] = dfz['OffLumi']*dfz['LBFull']

    # Add by groups of 20 LBs or pileup bins: scale by livetime, square errors, add, unscale livetime
    # overall ratio against ATLAS after livetime scaling
    for entry in [Lumi, LumiErr, 'OffLumi']:  
        dfz[entry] *= dfz['LBLive']
    normalisation = dfz[Lumi].sum() / dfz['OffLumi'].sum()
    dfz[LumiErr] *= dfz[LumiErr]
    if args.usemu:
        dfz['OffMu'] = dfz['OffMu'].astype(int)
        dfz = dfz.groupby(['OffMu']).sum()
    else: 
        dfz['LBNum'] = (dfz['LBNum']//20)*20
        dfz = dfz.groupby(['LBNum']).sum()
    dfz[LumiErr]   = np.sqrt(dfz[LumiErr])
    for entry in [Lumi, LumiErr, 'OffLumi']:  
        dfz[entry] /= dfz['LBLive']

    print(f"{Lumi}/OffLumi mean ratio = {normalisation:.3f}")

    if args.t0:
        translist = []
        timeformat = '%y/%m/%d %H:%M:%S'
        import time
        for k in range(len(dfz)):
            thisrow = dfz.iloc[k]
            lb = thisrow.name
            lbselector = (lb <= dfz0['LBNum']) & (dfz0['LBNum'] < lb+20)
            translist.append({'FillNum' : dfz0[lbselector]['FillNum'].iloc[0],
                              'beginTime' : time.strftime(timeformat,
                                                        time.gmtime(dfz0[lbselector]['LBStart'].iloc[0])),
                              'endTime': time.strftime(timeformat,
                                                     time.gmtime(dfz0[(dfz0['LBNum']//20)*20 == lb]['LBEnd'].iloc[-1])),
                              'ZmumuRate': thisrow['ZmumuRate']/thisrow['LBLive'],
                              'instDelLumi': thisrow['OffDelLumi']/thisrow['LBFull']/1e3,  # time-averaged instantaneous delivered lumi
                              'delLumi': thisrow['OffLumi']/1e3,  # livetime * instantaneous delivered lumi
                              'ZDel': thisrow['ZmumuRate']  # after above, this variable is yield not rate
                              })
        pdn = pd.DataFrame.from_records(translist)
        pdn.to_csv(os.path.join(args.outdir, 'zrate.csv'), header=False, index=False)

    if args.usemu: 
        leg = R.TLegend(0.6, 0.33, 0.75, 0.5)
        xtitle = "<#mu>"
        outfile = channel+"ATLAS_vs_mu"
    else: 
        leg = R.TLegend(0.62, 0.75, 0.75, 0.9)
        xtitle = "Luminosity Block Number"
        outfile = channel+"ATLAS_vs_lb"

    if args.absolute:
        outfile += "_abs"
        normalisation = 1.

    arr_bins      = array('d', dfz.index)
    arr_zlumi     = array('d', dfz[Lumi] / normalisation)
    arr_zlumi_err = array('d', dfz[LumiErr] / normalisation)
    arr_rat       = array('d', dfz[Lumi] / dfz['OffLumi'] / normalisation)
    arr_rat_err   = array('d', dfz[LumiErr] / dfz['OffLumi'] / normalisation)
    arr_olumi     = array('d', dfz['OffLumi'])

    hz = R.TGraphErrors(len(arr_bins), arr_bins, arr_zlumi, R.nullptr, arr_zlumi_err)
    ho = R.TGraphErrors(len(arr_bins), arr_bins, arr_olumi, R.nullptr, R.nullptr)
    hr = R.TGraphErrors(len(arr_bins), arr_bins, arr_rat, R.nullptr, arr_rat_err)

    ho.SetLineColor(R.kAzure)
    ho.SetLineWidth(3)
        
    c1 = R.TCanvas()

    pad1 = R.TPad("pad1", "pad1", 0, 0, 1, 1)
    pad1.SetBottomMargin(0.3)
    pad1.SetFillStyle(4000)
    pad1.Draw()
    pad1.cd()
    pad1.RedrawAxis()
    hz.GetXaxis().SetLabelSize(0)
        
    xmin = hz.GetXaxis().GetXmin()
    xmax = hz.GetXaxis().GetXmax()

    hz.SetMarkerStyle(4)
    hz.Draw('ap')
    ho.Draw("same L")
    hz.Draw('same p')
    hz.GetYaxis().SetTitle("Luminosity [10^{33} cm^{-2}s^{-1}]")
    hz.GetXaxis().SetTitle(xtitle)
    hz.GetXaxis().SetTitleOffset(0.8)
    ymax = hz.GetHistogram().GetMaximum()
    ymin = hz.GetHistogram().GetMinimum()
    if not args.usemu:
        hz.GetYaxis().SetRangeUser(ymin*0.8, ymax*1.4)

    leg.SetBorderSize(0)
    leg.SetTextSize(0.055)
    if args.absolute:
        leg.AddEntry(hz, "L_{"+LumiString+"}", "ep")
    else:
        leg.AddEntry(hz, "L_{"+LumiString+"}^{normalised to L_{ATLAS}^{fill}}", "ep")
    leg.AddEntry(ho, "L_{ATLAS}", "l")
    leg.Draw()

    pad2 = R.TPad("pad2", "pad2", 0, 0, 1, 1)
    pad2.SetTopMargin(0.72)
    pad2.SetBottomMargin(0.1)
    pad2.SetFillStyle(4000)
    pad2.Draw()
    pad2.cd()

    hr.Draw("ap0")
    hr.GetYaxis().SetTitle("Ratio")
    hr.GetXaxis().SetTitle(xtitle)
    hr.GetXaxis().SetTitleOffset(0.865)

    ymin, ymax = 0.95, 1.05
    median = np.median(arr_rat)
    spread = np.percentile(abs(arr_rat - median), 68)
    if spread > 0.03: ymin, ymax = 0.9, 1.1
    if args.absolute and args.t0: ymin, ymax = 0.9, 1.1
    hr.GetYaxis().SetRangeUser(ymin, ymax)

    line0 = R.TLine(xmin, 1.0, xmax, 1.0)
    line0.SetLineStyle(2)
    line0.Draw()

    line1 = R.TLine(xmin, (ymin+1.)/2., xmax, (ymin+1.)/2.)
    line1.SetLineStyle(2)
    line1.Draw()

    line2 = R.TLine(xmin, (ymax+1.)/2., xmax, (ymax+1.)/2.)
    line2.SetLineStyle(2)
    line2.Draw()

    hr.GetYaxis().SetNdivisions(3)

    pt.drawAtlasLabel(0.2, 0.88, "Internal")
    if not args.t0:
        pt.drawText(0.2, 0.82, pt.get_yearsqrtstxt(run_number), size=22)
    pt.drawText(0.2, 0.77, "LHC Fill " + str(lhc_fill), size=22)
    pt.drawText(0.2, 0.72, LumiString + " counting", size=22)
       
    line4 = pt.make_bands(arr_bins, spread, median)
    line4.Draw("same 3")
    hr.Draw("same ep0")

    if not args.t0:
        c1.SaveAs(os.path.join(outdir, outfile + ".pdf"))
    else:
        rfo.WriteTObject(c1, outfile)
        if channel == 'Zmumu':
            rfo.WriteTObject(hz, 'z_lumi')
            rfo.WriteTObject(hr, 'z_lumi_ratio')
    c1.Clear()


    #Plot ratio with fit
    c2 = R.TCanvas()
    hr.GetXaxis().SetTitleOffset(1)
    hr.Draw('ap0')

    hr.Fit('pol0', 'q0')
    hr.GetFunction('pol0').SetLineColor(R.kRed)

    hr.GetYaxis().SetTitle("Ratio")
    hr.GetXaxis().SetTitle(xtitle)
    hr.GetYaxis().SetRangeUser(0.9, 1.1)
    hr.GetYaxis().SetNdivisions()
    
    mean = hr.GetFunction("pol0").GetParameter(0)
    line0 = pt.make_bands(arr_bins, spread, median)
    line0.Draw("same 3")
    hr.Draw("same ep0")
    hr.GetFunction('pol0').Draw('same l')

    pt.drawAtlasLabel(0.2, 0.88, "Internal")
    if not args.t0:
        pt.drawText(0.2, 0.82, pt.get_yearsqrtstxt(run_number), size=22)
    pt.drawText(0.2, 0.77, "LHC Fill " + str(lhc_fill), size=22)
    pt.drawText(0.2, 0.72, LumiString + " counting", size=22)

    leg = R.TLegend(0.17, 0.2, 0.90, 0.3)
    leg.SetBorderSize(0)
    leg.SetTextSize(0.05)
    leg.SetNColumns(3)
    leg.AddEntry(hr, "L_{"+LumiString+"}/L_{ATLAS}", "ep")
    leg.AddEntry(hr.GetFunction("pol0"), f"Mean = {mean:.3f}", "l")
    leg.AddEntry(line0, "68% band", "f")
    leg.Draw()

    if not args.t0:
        c2.SaveAs(os.path.join(outdir, outfile+"_ratio.pdf"))
    else:
        rfo.WriteTObject(c2, f'{outfile}_ratio')
    
    return

def plot_ratio():
    dfz = pd.read_csv(infilename, delimiter=',')
    if dfz.empty:
        print('No data in file', infilename, ', exiting.')
        return

    run_number = dfz.RunNum[0]
    lhc_fill   = dfz.FillNum[0]

    if args.t0:
        rfo = R.TFile.Open(os.path.join(outdir, 'zlumi.root'), 'RECREATE')

    dfz = dfz.drop(dfz[(dfz.ZeeLumi == 0) | (dfz.ZmumuLumi == 0)].index)
    dfz = dfz.drop(dfz[(dfz['LBLive']<pt.lblivetimecut) | (dfz['PassGRL']==0)].index)
    if len(dfz) == 0:
        print("No valid LBs found. Exiting")
        return
        
    # Add by groups of 20 LBs or pileup bins: scale by livetime, square errors, add, unscale livetime
    for entry in ['ZeeLumi','ZmumuLumi','ZeeLumiErr','ZmumuLumiErr']:  
        dfz[entry] *= dfz['LBLive']
    for entry in ['ZeeLumiErr','ZmumuLumiErr']:  
        dfz[entry] *= dfz[entry]
    if args.usemu:
        dfz['OffMu'] = dfz['OffMu'].astype(int)
        dfz = dfz.groupby(['OffMu']).sum()
    else: 
        dfz['LBNum'] = (dfz['LBNum']//20)*20
        dfz = dfz.groupby(['LBNum']).sum()
    for entry in ['ZeeLumiErr','ZmumuLumiErr']:  
        dfz[entry] = np.sqrt(dfz[entry])
    for entry in ['ZeeLumi','ZmumuLumi','ZeeLumiErr','ZmumuLumiErr']:  
        dfz[entry] /= dfz['LBLive']

    arr_bins      = array('d', dfz.index)
    arr_rat       = array('d', dfz['ZeeLumi'] / dfz['ZmumuLumi'])
    arr_rat_err   = array('d', (dfz['ZeeLumi'] / dfz['ZmumuLumi']) * np.sqrt(pow(dfz['ZeeLumiErr']/dfz['ZeeLumi'], 2) + pow(dfz['ZmumuLumiErr']/dfz['ZmumuLumi'], 2)))

    gr = R.TGraphErrors(len(arr_rat), arr_bins, arr_rat, R.nullptr, arr_rat_err)

    c1 = R.TCanvas()
    gr.SetTitle("")
    gr.Draw("ap")

    if args.usemu: 
        xtitle = "<#mu>"
        outfile = "ZeeZmm_ratio_vs_mu.pdf"
    else: 
        xtitle = "Luminosity Block Number"
        outfile = "ZeeZmm_ratio_vs_lb.pdf"

    gr.GetXaxis().SetTitle(xtitle)
    
    gr.GetYaxis().SetTitle(pt.Leemumuratiolabel)

    ymin, ymax = 0.85, 1.15
    median = np.median(arr_rat)
    spread = np.percentile(abs(arr_rat - median), 68)
    if spread > 0.05: ymin, ymax = 0.75, 1.25
    gr.GetYaxis().SetRangeUser(ymin, ymax)

    gr.Fit("pol0", "q0")
    gr.GetFunction("pol0").SetLineColor(R.kRed)
   
    mean = gr.GetFunction("pol0").GetParameter(0)
    line1 = pt.make_bands(arr_bins, spread, median)
    line1.Draw("same 3")
    gr.GetFunction("pol0").Draw("same l")
    gr.Draw("same ep")

    pt.drawAtlasLabel(0.2, 0.88, "Internal")
    if not args.t0:
        pt.drawText(0.2, 0.82, pt.get_yearsqrtstxt(run_number), size=22)
    pt.drawText(0.2, 0.77, "LHC Fill " + str(lhc_fill), size=22)

    leg = R.TLegend(0.17, 0.2, 0.90, 0.3)
    leg.SetBorderSize(0)
    leg.SetTextSize(0.05)
    leg.SetNColumns(3)
    leg.AddEntry(gr, pt.Leemumuratiolabel, "ep")
    leg.AddEntry(gr.GetFunction("pol0"), f"Mean = {mean:.3f}", "l")
    leg.AddEntry(line1, "68% band", "f")
    leg.Draw()

    if not args.t0:
        c1.SaveAs(os.path.join(outdir, outfile))
    else:
        rfo.WriteTObject(c1, 'zeezmmratio')

if __name__ == "__main__":
    pt.setAtlasStyle()
    R.gStyle.SetTitleOffset(1, 'xy')
    R.gStyle.SetLabelSize(0.045, 'xy')
    R.gStyle.SetTitleSize(0.052, 'xy')
    R.gROOT.SetBatch(R.kTRUE)
    main()
