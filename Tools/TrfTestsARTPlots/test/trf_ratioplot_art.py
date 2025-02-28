#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

import ROOT
from ROOT import TCanvas, TColor, TGaxis, TH1F, TH1D, TPad, TLegend
from ROOT import kBlack, kBlue, kRed
from ROOT import TFile 
import argparse, sys

import plot_style
 
def createH1(h1):
  h1.SetLineColor(kBlue+1)
  h1.SetLineWidth(2)
  h1.GetYaxis().SetTitleSize(20)
  h1.GetYaxis().SetTitleFont(43)
  h1.GetYaxis().SetTitleOffset(1.55)
  h1.SetStats(0)
  return h1 
 
def createH2(h2):
  h2.SetLineColor(kRed)
  h2.SetLineWidth(2)
  return h2
  
def createRatio(h1, h2, ratiotitle):
  h3 = h1.Clone("h3")
  h3.SetLineColor(kBlack)
  h3.SetMarkerStyle(21)
  h3.SetTitle("")
  h3.SetMinimum(0.8)
  h3.SetMaximum(1.35)
  # Set up plot for markers and errors
  h3.Sumw2()
  h3.SetStats(0)
  h3.Divide(h2)
 
  # Adjust y-axis settings
  y = h3.GetYaxis()
  y.SetTitle(ratiotitle)
  y.SetNdivisions(505)
  y.SetTitleSize(20)
  y.SetTitleFont(43)
  y.SetTitleOffset(1.55)
  y.SetLabelFont(43)
  y.SetLabelSize(15)
 
  # Adjust x-axis settings
  x = h3.GetXaxis()
  x.SetTitleSize(20)
  x.SetTitleFont(43)
  x.SetTitleOffset(4.0)
  x.SetLabelFont(43)
  x.SetLabelSize(15)
 
  return h3
 
def createCanvasPads(c):
  c.cd()
  # Upper histogram plot is pad1
  pad1 = TPad("pad1", "pad1", 0, 0.3, 1, 1.0)
  pad1.SetBottomMargin(0)  # joins upper and lower plot
  pad1.SetGridx()
  pad1.Draw()
  # Lower ratio plot is pad2
  c.cd()  # returns to main canvas before defining pad2
  pad2 = TPad("pad2", "pad2", 0, 0.05, 1, 0.3)
  pad2.SetTopMargin(0)  # joins upper and lower plot
  pad2.SetBottomMargin(0.2)
  pad2.SetGridx()
  pad2.Draw()
 
  return pad1, pad2

def findallhistos(filename):
  allhistos = []

  f1 = TFile.Open(filename,"r")

  for k in f1.GetListOfKeys():
    if k.GetClassName() == "TDirectoryFile":
      dirname = k.GetName()
      f1.cd(dirname)
      subdir = ROOT.gDirectory;
      for j in subdir.GetListOfKeys():
        if j.GetClassName() == "TH1F":
          allhistos.append(dirname+"/"+j.GetName())
      f1.cd()

  f1.Close()    
  return allhistos
 
def ratioplot(filenameref, filenametest, allhistos, campaignname, ratiotitle):

  ks = 1.0
  f1 = TFile.Open(filenameref,"r")
  f2 = TFile.Open(filenametest,"r")

  c = TCanvas("c", "canvas", 800, 800)

  for histoname in allhistos:
    f1.cd()
    h1 = f1.Get(histoname)
    f2.cd()
    h2 = f2.Get(histoname)

    # create required parts
    h1 = createH1(h1)
    h2 = createH2(h2)
    h3 = createRatio(h1, h2, ratiotitle)
    pad1, pad2 = createCanvasPads(c)

    # draw everything
    pad1.cd()
    h1.Draw()
    h2.Draw("same")

    # Add legend
    leg = TLegend (0.2, 0.70, 0.95, 0.90)
    leg.SetFillColor(0)
    leg.SetFillStyle(0)
    leg.SetTextFont(42)
    leg.SetTextSize(0.035)
    leg.SetBorderSize(0)
    leg.SetHeader("%s %s" % (campaignname, histoname))
    leg.SetMargin(0.1)
    leg.AddEntry(h1, 'ref', "L")
    leg.AddEntry(h2, 'test', "L")
    leg.Draw()

    pad2.cd()
    h3.Draw("ep")
    c.Update()

    histoname = histoname.replace("/", "_")
    histoname = histoname.replace(" ", "_")
    c.Print("%s_%s.png" %(campaignname, histoname ))

    # Kolmogorov-Smirnoff
    res = h1.KolmogorovTest(h2)
    print ("*** Kolmogorov-Smirnoff h1 vs. h2 %s = %s" % (histoname, res ))
    if res < ks:
      ks = res

  f1.Close()
  f2.Close()

  return ks
 
if __name__ == "__main__":
  # Set ROOT batch mode
  ROOT.gROOT.SetBatch(True)

  parser = argparse.ArgumentParser(description="Plots all TH1F from a ROOT histogram reference vs. test file")
  parser.add_argument("--reffile", type=str, help="reference ROOT file", default="ref.root", action="store")
  parser.add_argument("--testfile", type=str, help="test ROOT file", default="test.root", action="store")

  args = parser.parse_args()

  if len(sys.argv) < 2:
    parser.print_help()
    sys.exit(1)

  # Setup ATLAS style
  plot_style.set_atlas()

  campaignname = "ref_vs_test"
  ratiotitle = "ratio ref/test"

  # Find all histograms in ROOT file
  allhistos = findallhistos(args.reffile)
  print(allhistos)

  # Plot all the histograms to PNG files
  res = ratioplot(args.reffile, args.testfile, allhistos, campaignname, ratiotitle)
  if res < 1.0:
    sys.exit(1)

  sys.exit(0)
