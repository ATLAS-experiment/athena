#!/usr/bin/env python3
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

import numpy as np
import pandas as pd
import python_tools as pt
import ROOT as R
import os
from array import array
import argparse
import re


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

substring = "data"
pattern = re.compile(substring + r"(\d{2})")
match = pattern.search(infilename)
if match:
    year = match.group(1)
else:
    print("Year not found")
    year = "XX"

def main():
    zmumulumi, zmumulumierr = plot_channel('Zmumu')
    zeelumi, zeelumierr  = plot_channel('Zee')
    zlllumi, zlllumierr = plot_channel('Zll')
    plot_ratio(zeelumi, zeelumierr, zmumulumi, zmumulumierr)
    

def plot_channel(channel):
    dfz = pd.read_csv(infilename, delimiter=',')
    if dfz.empty:
        print('No data available. Exiting')
        return

    run_number = str(int(dfz.RunNum[0]))
    lhc_fill   = str(int(dfz.FillNum[0]))

    if not args.t0:
        outdirstr = outdir + run_number
        filepfx = f'data{year}_{run_number}_'
    else:
        outdirstr = outdir
        filepfx = ''

    if args.t0:
        rfo = R.TFile.Open(os.path.join(outdirstr, 'zlumi.root'), 'RECREATE')

    if channel == 'Zee':
        Lumi = 'ZeeLumi'
        LumiErr = 'ZeeLumiErr'
        LumiString = "L_{Z #rightarrow ee}"
        name = 'e'

        #Drop LBs with no Z-counting information
        dfz0 = dfz.copy()
        if args.dropzero:
            dfz = dfz.drop(dfz[(dfz.ZeeLumi == 0)].index)

    elif channel == 'Zmumu':
        Lumi = 'ZmumuLumi'
        LumiErr = 'ZmumuLumiErr'
        LumiString = "L_{Z #rightarrow #mu#mu}"
        name = '#mu'

        #Drop LBs with no Z-counting information
        dfz0 = dfz.copy()
        if args.dropzero:
            dfz = dfz.drop(dfz[(dfz.ZmumuLumi == 0)].index)

    elif channel == 'Zll':
        Lumi = 'ZllLumi'
        LumiErr = 'ZllLumiErr'
        LumiString = "L_{Z #rightarrow ll}"
        name = 'l'

        #Drop LBs with no Z-counting information
        dfz0 = dfz.copy()
        if args.dropzero:
            dfz = dfz.drop(dfz[(dfz.ZllLumi == 0)].index)

    #Calculate mean per LB against ATLAS
    dfz = dfz.drop(dfz[(dfz['LBLive']<10) | (dfz['PassGRL']==0)].index)

    if len(dfz):
        ratio = array('d', dfz[channel+'Lumi']/dfz['OffLumi'])
        print("mean for "+channel+": ", np.mean(ratio))
    else:
        ratio = 1
        print("No valid LBs found")

    dfz['OffDelLumi'] = dfz['OffLumi']*dfz['LBFull']

    #Scale by livetime
    for entry in [Lumi, LumiErr,'OffLumi']:  
        dfz[entry] *= dfz['LBLive']

    #Square uncertainties
    dfz[LumiErr] *= dfz[LumiErr]

    #Merge by groups of 20 LBs or pileup bins
    if args.usemu:
        dfz['OffMu'] = dfz['OffMu'].astype(int)
        dfz = dfz.groupby(['OffMu']).sum()
    else: 
        dfz['LBNum'] = (dfz['LBNum']//20)*20
        dfz = dfz.groupby(['LBNum']).sum()
    
    dfz[LumiErr]   = np.sqrt(dfz[LumiErr])

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

    print("Making Z-counting vs. ATLAS plots!")
    channel_string = "Z #rightarrow "+name+name
    
    if args.usemu: 
        leg = R.TLegend(0.6, 0.28, 0.75, 0.50)
        xtitle = "<#mu>"
        if args.absolute:
            outfile = filepfx+channel+"_vs_atlas_mu_abs"
        else:
            outfile = filepfx+channel+"_vs_atlas_mu"
    else: 
        leg = R.TLegend(0.6, 0.65, 0.75, 0.87)
        xtitle = "Luminosity Block Number"
        if args.absolute:
            outfile = filepfx+channel+"_vs_atlas_abs"
        else:
            outfile = filepfx+channel+"_vs_atlas"

    normalisation = 1
    if len(dfz) and not args.absolute:
        normalisation = dfz[channel+'Lumi'].sum() / dfz['OffLumi'].sum()

    if len(dfz):

        arr_bins      = array('d', dfz.index)
        arr_zlumi     = array('d', dfz[channel+'Lumi'] / dfz['LBLive'] / normalisation)
        arr_zlumi_err = array('d', dfz[channel+'LumiErr'] / dfz['LBLive'] / normalisation)
        arr_rat       = array('d', dfz[channel+'Lumi'] / dfz['OffLumi'] / normalisation)
        arr_rat_err   = array('d', dfz[channel+'LumiErr'] / dfz['OffLumi'] / normalisation)
        arr_olumi     = array('d', dfz['OffLumi'] / dfz['LBLive'])

        hz = R.TGraphErrors(len(arr_bins), arr_bins, arr_zlumi, R.nullptr, arr_zlumi_err)
        ho = R.TGraphErrors(len(arr_bins), arr_bins, arr_olumi, R.nullptr, R.nullptr)
        hr = R.TGraphErrors(len(arr_bins), arr_bins, arr_rat, R.nullptr, arr_rat_err)

    else:
        arr_bins      = array('d', [])
        arr_zlumi     = array('d', [])
        arr_zlumi_err = array('d', [])
        arr_rat       = array('d', [])
        arr_rat_err   = array('d', [])
        arr_olumi     = array('d', [])
        
        hz = R.TGraphErrors()
        ho = R.TGraphErrors()
        hr = R.TGraphErrors()


    hz.SetTitle("")
    ho.SetTitle("")
    hr.SetTitle("")

    ho.SetLineColor(R.kAzure)
    ho.SetLineWidth(3)
        
    c1 = R.TCanvas()

    pad1 = R.TPad("pad1", "pad1", 0, 0, 1, 1)
    pad1.SetBottomMargin(0.25)
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
        if args.absolute and not args.t0:
            hz.GetYaxis().SetRangeUser(0, 33)
        else: 
            hz.GetYaxis().SetRangeUser(ymin-2, ymax*1.6)
            hz.GetYaxis().SetRangeUser(ymin*0.5, ymax*2)
       
    leg.SetBorderSize(0)
    leg.SetTextSize(0.06)
    if args.absolute:
        leg.AddEntry(hz, "L_{"+channel_string+"}", "ep")
    else:
        leg.AddEntry(hz, "L_{"+channel_string+"}^{normalised to L_{ATLAS}^{fill}}", "ep")
    leg.AddEntry(ho, "L_{ATLAS}", "l")
    leg.Draw()

    pad2 = R.TPad("pad2", "pad2", 0, 0, 1, 1)
    pad2.SetTopMargin(0.78)
    pad2.SetBottomMargin(0.09)
    pad2.SetFillStyle(4000)
    pad2.Draw()
    pad2.cd()

    hr.Draw("ap0")
    hr.GetXaxis().SetTitleSize(0.05)
    hr.GetXaxis().SetTitleOffset(0.865)
    hr.GetYaxis().SetTitle("Ratio")
    hr.GetYaxis().SetTitleOffset(1)
    hr.GetXaxis().SetTitle(xtitle)
    
    hr.GetYaxis().SetTitleSize(0.05)
    if args.absolute and args.t0:
        hr.GetYaxis().SetRangeUser(0.9, 1.1)
    else:
        hr.GetYaxis().SetRangeUser(0.95, 1.05)

    line0 = R.TLine(xmin, 1.0, xmax, 1.0)
    line0.SetLineStyle(2)
    line0.Draw()

    line1 = R.TLine(xmin, 0.975, xmax, 0.975)
    line1.SetLineStyle(2)
    line1.Draw()

    line2 = R.TLine(xmin, 1.025, xmax, 1.025)
    line2.SetLineStyle(2)
    line2.Draw()

    hr.GetXaxis().SetLabelSize(0.045)
    hr.GetYaxis().SetLabelSize(0.045)
    hr.GetYaxis().SetNdivisions(3)

    pt.drawAtlasLabel(0.2, 0.86, "Internal")
    if not args.t0:
        if year in ['15', '16', '17', '18']:
            pt.drawText(0.2, 0.80, "Data 20" + year + ", #sqrt{s} = 13 TeV")
        else:
            pt.drawText(0.2, 0.80, "Data 20" + year + ", #sqrt{s} = 13.6 TeV")
    pt.drawText(0.2, 0.74, "LHC Fill " + lhc_fill)
    pt.drawText(0.2, 0.68,  channel_string + " counting")
       

    if len(dfz):
        median = np.median(arr_rat)
        hr.Fit('pol0', '0q')
        mean = hr.GetFunction('pol0').GetParameter(0)
        stdev = np.percentile(abs(arr_rat - np.median(arr_rat)), 68)
        print("channel =", channel, "run =", run_number, "median =", median, "mean = ", mean, "stdev =", stdev)

        line4 = pt.make_bands(arr_bins, stdev, mean)
        line4.Draw("same 3")
        hr.Draw("same ep0")

    if not args.t0:
            c1.SaveAs(os.path.join(outdirstr, outfile + ".eps"))
            c1.SaveAs(os.path.join(outdirstr, outfile + ".pdf"))
    else:
        rfo.WriteTObject(c1, outfile)
        if channel == 'Zmumu':
            rfo.WriteTObject(hz, 'z_lumi')
            rfo.WriteTObject(hr, 'z_lumi_ratio')
    c1.Clear()

    #Plot ratio with fit
    c2 = R.TCanvas()
    hr.Draw('ap0')

    if len(dfz):
        hr.Fit('pol0')
        hr.GetFunction('pol0').SetLineColor(R.kRed)

        hr.GetYaxis().SetTitle("Ratio")
        hr.GetXaxis().SetTitle(xtitle)
        hr.GetYaxis().SetTitleOffset(0.865)
        hr.GetYaxis().SetRangeUser(0.9, 1.1)
        hr.GetYaxis().SetNdivisions()

        mean = hr.GetFunction("pol0").GetParameter(0)
        stdev = np.percentile(abs(arr_rat - np.median(arr_rat)), 68)

        line0 = pt.make_bands(arr_bins, stdev, mean)
        line0.Draw("same 3")
        hr.Draw("same ep0")
        hr.GetFunction('pol0').Draw('same l')

        print("####")
        print("stdev = ", stdev)
        print("mean = ", mean)
        print("####")

    pt.drawAtlasLabel(0.2, 0.86, "Internal")
    if not args.t0:
        if year == "22" or year == "23":
            pt.drawText(0.2, 0.80, "Data 20" + year + ", #sqrt{s} = 13.6 TeV")
        else:
            pt.drawText(0.2, 0.80, "Data 20" + year + ", #sqrt{s} = 13 TeV")
    pt.drawText(0.2, 0.74, "LHC Fill " + lhc_fill)
    pt.drawText(0.2, 0.68,  channel_string + " counting")
    
    mean = hr.GetFunction("pol0").GetParameter(0)
    leg = R.TLegend(0.17, 0.2, 0.90, 0.3)
    leg.SetBorderSize(0)
    leg.SetTextSize(0.05)
    leg.SetNColumns(3)
    leg.AddEntry(hr, LumiString+"/L_{ATLAS}", "ep")
    leg.AddEntry(hr.GetFunction("pol0"), "Mean = {:.3f}".format(round(mean, 3)), "l")
    leg.AddEntry(line0, "68% band", "f")
    leg.Draw()

    if not args.t0:
        c2.SaveAs(os.path.join(outdirstr, outfile+"_ratio.eps"))
        c2.SaveAs(os.path.join(outdirstr, outfile+"_ratio.pdf"))
    else:
        rfo.WriteTObject(c2, f'{outfile}_ratio')
    
    return arr_zlumi, arr_zlumi_err

def plot_ratio(zeelumi, zeelumierr, zmumulumi, zmumulumierr):
    dfz = pd.read_csv(infilename, delimiter=',')
    if dfz.empty:
        print('No data available. Exiting')
        return

    run_number = str(int(dfz.RunNum[0]))
    lhc_fill   = str(int(dfz.FillNum[0]))
    
    if not args.t0:
        outdirstr = outdir + run_number
        filepfx = f'data{year}_{run_number}_'
    else:
        outdirstr = outdir
        filepfx = ''

    if args.t0:
        rfo = R.TFile.Open(os.path.join(outdirstr, 'zlumi.root'), 'RECREATE')

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

    if len(dfz):
        dfz = dfz.drop(dfz[(dfz.ZeeLumi == 0) | (dfz.ZmumuLumi == 0)].index)
        arr_bins      = array('d', dfz.index)
        arr_rat       = array('d', dfz['ZeeLumi'] / dfz['ZmumuLumi'])
        arr_rat_err   = array('d', (dfz['ZeeLumi'] / dfz['ZmumuLumi']) * np.sqrt(pow(dfz['ZeeLumiErr']/dfz['ZeeLumi'], 2) + pow(dfz['ZmumuLumiErr']/dfz['ZmumuLumi'], 2)))

        gr = R.TGraphErrors(len(arr_rat), arr_bins, arr_rat, R.nullptr, arr_rat_err)

    else:
        arr_bins      = array('d', [])
        arr_rat       = array('d', [])
        arr_rat_err   = array('d', [])

        gr = R.TGraphErrors()

    c1 = R.TCanvas()
    gr.SetTitle("")
    gr.Draw("ap")
    gr.GetXaxis().SetTitle("Luminosity Block")
    gr.GetYaxis().SetTitle("L_{Z #rightarrow ee} / L_{Z #rightarrow #mu#mu}")
    if year == "15": 
        gr.GetYaxis().SetRangeUser(0.6, 1.4)
    else: 
        gr.GetYaxis().SetRangeUser(0.8, 1.2)
    if len(dfz):
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
    R.ATLASLabel(0.2, 0.86, "Internal")
    if not args.t0:
        if year in ['15', '16', '17', '18']:
            latex.DrawLatexNDC(0.2, 0.80, "Data 20" +year+ ", #sqrt{s} = 13 TeV")
        else:
            latex.DrawLatexNDC(0.2, 0.80, "Data 20" +year+ ", #sqrt{s} = 13.6 TeV")
    latex.DrawLatexNDC(0.2, 0.74, "LHC Fill " + lhc_fill)

    if len(dfz):
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
    if len(dfz):
        leg.AddEntry(gr.GetFunction("pol0"), "Mean = {:.3f}".format(round(mean, 3)), "l")
        leg.AddEntry(line1, "68% band", "f")
    leg.Draw()

    if not args.t0:
        c1.SaveAs(os.path.join(outdirstr, filepfx+run_number+"_zeezmmratio.eps"))
        c1.SaveAs(os.path.join(outdirstr, filepfx+run_number+"_zeezmmratio.pdf"))
    else:
        rfo.WriteTObject(c1, 'zeezmmratio')

if __name__ == "__main__":
    pt.setAtlasStyle()
    R.gROOT.SetBatch(R.kTRUE)
    main()
