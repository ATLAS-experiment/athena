#!/usr/bin env python
# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
# Script to browse the unmerged HIST files and extract LBs for which at least N occurences of an object is found
# at a position found to be noisy
# Uses the pathExtract library to extract the EOS path
# See the twiki: https://twiki.cern.ch/twiki/bin/viewauth/Atlas/UsingDQInframerge
# Author: Benjamin Trocme (LPSC Grenoble) / 2015-2016
# Update for Run 3 by Benjamin Trocme (LPSC Grenoble): March 2022
#     * Adding a protection for the missing histograms in HIST file
#     * Commenting the getStruct method (added by who???)
#     * Adding the LoosePhotons object


import os, sys
import argparse
from xmlrpc import client as xmlrpclib
import math
from DataQualityUtils import pathExtract         

sys.path.append("/afs/cern.ch/user/l/larmon/public/prod/Misc")
from LArMonCoolLib import GetLBTimeStamps,GetOnlineLumiFromCOOL

import ROOT as R

############################################################################################################################################################################
############################################################################################################################################################################
def lbStr(lb):
  """Return formatted lumiblock string"""
  return "_lb"+lb.zfill(5) 
  
def getHistoInfo(objectType, runNumber):
  """Histo path definition base on object type"""
  # histoPath   : Histogram path in the ROOT file
  # histoLegend : Histogram legend
  # histoColor  : Colors for the final summary plot
  # histoName   : Name of object
  # Types of plot
  # 2d_etaPhi_Occupancy        : 2D occupancy plots: (eta/phi) required
  # 2d_etaPhi_MeanPt           : 2D mean Pt plots: (eta/phi) required
  # 2d_xy_Occupancy            : any 2D plots: (x/y) required
  # 1d_eta_Occupancy           : 1D occupancy plot along eta: (eta) required
  # 1d_phi_Occupancy           : 1D occupancy plot along phi: (phi) required
  # 1d_integralAbove_Occupancy : integral between (integralAbove) and infinity. (integralAbove) required
  # NB : if the required arguments are not provided, consider the whole histogram

  runstr = "run_"+str(runNumber)

  if objectType == "TopoClusters": # Release 22 : OK
    histoPath = {"ECA_thr1":runstr+"/CaloTopoClusters/CalECA/Thresh1ECAOcc",
                 "ECA_thr2":runstr+"/CaloTopoClusters/CalECA/Thresh2ECAOcc",
                 "ECA_thr3":runstr+"/CaloTopoClusters/CalECA/Thresh3ECAOcc",
                 "ECC_thr1":runstr+"/CaloTopoClusters/CalECC/Thresh1ECCOcc",
                 "ECC_thr2":runstr+"/CaloTopoClusters/CalECC/Thresh2ECCOcc",
                 "ECC_thr3":runstr+"/CaloTopoClusters/CalECC/Thresh3ECCOcc",
                 "BAR_thr1":runstr+"/CaloTopoClusters/CalBAR/Thresh1BAROcc",
                 "BAR_thr2":runstr+"/CaloTopoClusters/CalBAR/Thresh2BAROcc",
                 "BAR_thr3":runstr+"/CaloTopoClusters/CalBAR/Thresh3BAROcc"}
    histoLegend = {"ECA_thr1":"ECA - Et > 10 GeV",
                   "ECA_thr2":"ECA - Et > 15 GeV",
                   "ECA_thr3":"ECA - Et > 25 GeV",
                   "ECC_thr1":"ECC - Et > 10 GeV",
                   "ECC_thr2":"ECC - Et > 15 GeV",
                   "ECC_thr3":"ECC - Et > 25 GeV",
                   "BAR_thr1":"BAR - Et > 10 GeV",
                   "BAR_thr2":"BAR - Et > 15 GeV",
                   "BAR_thr3":"BAR - Et > 25 GeV"}
    histoType = "2d_etaPhi_Occupancy"
    histoName = "TopoClusters occupancy"

  elif objectType == "EMTopoClusters": # Release 22 : OK
    histoPath  = {"Et4GeV":runstr+"/CaloMonitoring/ClusterMon/LArClusterEMNoTrigSel/2d_Rates/m_clus_etaphi_Et_thresh1",
                  "Et10GeV":runstr+"/CaloMonitoring/ClusterMon/LArClusterEMNoTrigSel/2d_Rates/m_clus_etaphi_Et_thresh2",
                  "Et25GeV":runstr+"/CaloMonitoring/ClusterMon/LArClusterEMNoTrigSel/2d_Rates/m_clus_etaphi_Et_thresh3"}
    histoLegend = {"Et4GeV":"Et > 4GeV",
                   "Et10GeV":"Et > 10GeV",
                   "Et25GeV":"Et > 25GeV"}
    histoPath = {"ECA_thr1":runstr+"/CaloTopoClusters/CalEMECA/EMThresh1ECAOcc",
                 "ECA_thr2":runstr+"/CaloTopoClusters/CalEMECA/EMThresh2ECAOcc",
                 "ECA_thr3":runstr+"/CaloTopoClusters/CalEMECA/EMThresh3ECAOcc",
                 "ECC_thr1":runstr+"/CaloTopoClusters/CalEMECC/EMThresh1ECCOcc",
                 "ECC_thr2":runstr+"/CaloTopoClusters/CalEMECC/EMThresh2ECCOcc",
                 "ECC_thr3":runstr+"/CaloTopoClusters/CalEMECC/EMThresh3ECCOcc",
                 "BAR_thr1":runstr+"/CaloTopoClusters/CalEMBAR/EMThresh1BAROcc",
                 "BAR_thr2":runstr+"/CaloTopoClusters/CalEMBAR/EMThresh2BAROcc",
                 "BAR_thr3":runstr+"/CaloTopoClusters/CalEMBAR/EMThresh3BAROcc"}
    histoLegend = {"ECA_thr1":"ECA - Et > 4 GeV",
                   "ECA_thr2":"ECA - Et > 10 GeV",
                   "ECA_thr3":"ECA - Et > 15 GeV",
                   "ECC_thr1":"ECC - Et > 4 GeV",
                   "ECC_thr2":"ECC - Et > 10 GeV",
                   "ECC_thr3":"ECC - Et > 15 GeV",
                   "BAR_thr1":"BAR - Et > 4 GeV",
                   "BAR_thr2":"BAR - Et > 10 GeV",
                   "BAR_thr3":"BAR - Et > 15 GeV"}
    histoType = "2d_etaPhi_Occupancy"
    histoName = "EMTopoClusters occupancy"

  elif objectType == "EMTopoJet": # Release 22 : Missing histograms
    histoPath  = {"noCut":runstr+"/Jets/AntiKt4EMTopoJets/OccupancyEtaPhi",
                  "cut1":runstr+"/Jets/AntiKt4EMTopoJets/OccupancyEtaPhisel_20000_inf_pt_inf_500000",
                  "cut2":runstr+"/Jets/AntiKt4EMTopoJets/OccupancyEtaPhisel_500000_inf_pt_inf_1000000",
                  "cut3":runstr+"/Jets/AntiKt4EMTopoJets/OccupancyEtaPhisel_1000000_inf_pt_inf_2000000",
                  "cut4":runstr+"/Jets/AntiKt4EMTopoJets/OccupancyEtaPhisel_2000000_inf_pt_inf_8000000"}
    histoLegend = {"noCut":"No cut",
                   "cut1":"20GeV-500GeV",
                   "cut2":"500GeV-1TeV",
                   "cut3":"1TeV-2TeV",
                   "cut4":"2TeV-8TeV"}
    histoType = "2d_etaPhi_Occupancy"
    histoName = "EMTopoJets"

  elif objectType == "AntiKt4EMTopoJets": # Release 22 : OK
    histoPath  = {"noCut":runstr+"/Jets/AntiKt4EMTopoJets/standardHistos/phi_eta"}
    histoLegend = {"noCut":"No cut"}
    histoType = "2d_etaPhi_Occupancy"
    histoName = "Occupancy of AntiKt4EMTopoJets"

  elif objectType == "AntiKt4EMTopoJets_pt": # Release 22 : OK
    histoPath  = {"noCut":runstr+"/Jets/AntiKt4EMTopoJets/standardHistos/phi_eta_pt"}
    histoLegend = {"noCut":"No cut"}
    histoType = "2d_etaPhi_MeanPt"
    histoName = "Mean Pt of AntiKt4EMTopoJets"

  elif objectType == "MET_Topo_phi": # Release 22 : OK
    histoPath  = {"MET":runstr+"/MissingEt/AllTriggers/MET_Calo/EMTopo/MET_Topo_phi"}
    histoLegend = {"MET":"MET"}
    histoType = "1d_phi_Occupancy"
    histoName = "MET phi"

  elif objectType == "Tau": # Release 22 : OK
    histoPath  = {"NoCut":runstr+"/Tau/tauPhiVsEta",
                  "Et15GeV":runstr+"/Tau/tauPhiVsEta_et15"}
    histoLegend = {"NoCut":"Et > 0 GeV (tbc)",
                   "Et15GeV":"Et > 15 GeV"}
    histoType = "2d_etaPhi_Occupancy"
    histoName = "Tau occupancy"

  elif objectType == "Tau_phi": # Release 22 : OK
    histoPath  = {"single":runstr+"/Tau/tauPhi"}
    histoLegend = {"single":"All candidates"}
    histoType = "1d_phi_Occupancy"
    histoName = "Tau"

  elif objectType == "Tau_eta": # Release 22 : OK
    histoPath  = {"single":runstr+"/Tau/tauEta"}
    histoLegend = {"single":"All candidates"}
    histoType = "1d_eta_Occupancy"
    histoName = "Tau phi"

  elif objectType == "NumberTau": # Release 22 : OK
    histoPath  = {"highPt":runstr+"/Tau/nHightPtTauCandidates"}
    histoLegend = {"highPt":"High Pt (>100GeV) candidates"}
    histoType = "1d_integralAbove_Occupancy"
    histoName = "Number of Tau candidates"

  elif objectType == "TightFwdElectrons": # Release 22 : Missing histogram
    histoPath  = {"single":runstr+"/egamma/forwardElectrons/forwardElectronTightEtaPhi"}
    histoLegend = {"single":"10GeV"}
    histoType = "2d_etaPhi_Occupancy"
    histoName = "Tight electrons"

  elif objectType == "LoosePhotons": # Release 22 : Missing histogram
    histoPath  = {"single":runstr+"/egamma/CBLoosePhotons/Eta_Phi_distribution_with_Pt.gt.4GeV"}
    histoLegend = {"single":"4GeV"}
    histoType = "2d_etaPhi_Occupancy"
    histoName = "Loose photons"

  elif objectType == "NumberTightFwdElectrons": # Release 22 : Missing histogram
    histoPath  = {"single":runstr+"/egamma/forwardElectrons/forwardElectronTightN"}
    histoLegend = {"single":"All candidates"}
    histoType = "1d_integralAbove_Occupancy"
    histoName = "Number of tight forward electrons"

  elif objectType == "HLT_TausRoIs":
    histoPath  = {"single":runstr+"/HLT/ResultMon/HLT_Taus/HLT_TausRoIs"}
    histoLegend = {"single":""}
    histoType = "2d_etaPhi_Occupancy"
    histoName = "HLT TausRoIs"

  elif objectType == "NumberHLTJet": # Release 22 : Missing histogram
    histoPath  = {"HLTJet":runstr+"/HLT/JetMon/HLT/10j40_L14J20/HLTJet_n"}
    histoLegend = {"HLTJet":"All candidates"}
    histoType = "1d_integralAbove_Occupancy"
    histoName = "Number of HLT jets - 10J40_L14J20 trigger"

  elif objectType == "HLT_ConfigConsistency": # Missing histogram
    histoPath  = {"HLTJet":runstr+"/HLT/ResultMon/ConfigConsistency_HLT"}
    histoLegend = {"HLTJet":"Configuration consistency"}
    histoType = "1d_integralAbove_Occupancy"
    histoName = "Number of HLT config inconsistencies"

  elif objectType == "LArDigits": # Release 22 : not tested
    histoPath  = {"Null-EMBA":runstr+"/LAr/Digits/Barrel/NullDigitChan_BarrelA",
                  "Satu-EMBA":runstr+"/LAr/Digits/Barrel/SaturationChan_BarrelA",
                  "Null-EMBC":runstr+"/LAr/Digits/Barrel/NullDigitChan_BarrelC",
                  "Satu-EMBC":runstr+"/LAr/Digits/Barrel/SaturationChan_BarrelC",
    }
    histoLegend = {"Null-EMBA":"Null digit - EMBA",
                   "Satu-EMBA":"Saturated digit - EMBA",
                   "Null-EMBC":"Null digit - EMBC",
                   "Satu-EMBC":"Saturated digit - EMBC",}
    histoType = "2d_xy_Occupancy"
    histoName = "LAr saturated/null digits"
    
  else:
    print("Object type:",objectType,"not recognised!")
    sys.exit()

  cols = [R.kBlue+1, R.kGreen+1, R.kOrange+7, R.kMagenta+2, R.kCyan-3,
          R.kGreen-5, R.kRed, R.kCyan, R.kViolet,
          R.kAzure-4, R.kMagenta-9, R.kGreen-9, R.kYellow]
  histoColor = {k:cols[list(histoPath.keys()).index(k)] for k in histoPath.keys()}

  return histoPath, histoLegend, histoColor, histoType, histoName

############################################################################################################################################################################
############################################################################################################################################################################
def main(args):
  histoPath, histoLegend, histoColor, histoType, histoName = getHistoInfo(args.objectType, args.runNumber)

  #########################################################################
  # Depending of the histo/check type, define the summary title and
  # check that the position of the "hot spot" (or lower bound of the integral) is defined
  b_wholeHisto = False
  b_ValueNotEntries = False

  if histoType == "2d_etaPhi_Occupancy" or histoType == "2d_etaPhi_MeanPt":
    if histoType == "2d_etaPhi_Occupancy":
      summaryTitle = "Nb of hits in a region of %.2f around the position (%.2f,%.2f) - %s"%(args.deltaSpot,args.etaSpot,args.phiSpot,histoName)
    else:
      summaryTitle = "Mean Pt in a region of %.2f around the position (%.2f,%.2f) - %s"%(args.deltaSpot,args.etaSpot,args.phiSpot,histoName)
      if (args.deltaSpot != 0):
        print("Warning: you have been summing over several bins a variable that may be not summable (different from summing hits!)")

    statement = "I have looked for LBs with at least %.0f entries at position (%.2f,%.2f) in %s histogram"%(args.minInLB,args.etaSpot,args.phiSpot,histoName)
    if (args.etaSpot==-999. or args.phiSpot==-999.):
      print("No eta/phi defined -> whole histogram considered!")
      b_wholeHisto = True
  if histoType == "2d_xy_Occupancy":
    b_ValueNotEntries = True
    if (args.deltaSpot != 0):
      print("Warning: you have been summing over several bins a variable that may be not summable (different from summing hits!)")
    summaryTitle = "Value in a region of %.2f around the position (%.2f,%.2f) - %s"%(args.deltaSpot,args.xSpot,args.ySpot,histoName)
    statement = "I have looked for LBs with at least variable > %.2f at position (%.2f,%.2f) in %s histogram"%(args.minInLB,args.xSpot,args.ySpot,histoName)
    if (args.xSpot==-999. or args.ySpot==-999.):
      print("No x/y defined -> whole histogram considered!")
      print("Warning: you have been summing over several bins a variable that may be not summable (different from summing hits!)")
      b_wholeHisto = True
  elif histoType == "1d_eta_Occupancy":
    summaryTitle = "Nb of hits in a region of %.2f around the eta position %.2f - %s"%(args.deltaSpot,args.etaSpot,histoName)
    statement = "I have looked for LBs with at least %.0f entries at eta position %.2f in %s histogram"%(args.minInLB,args.etaSpot,histoName)
    if (args.etaSpot==-999.):
      print("No eta/phi -> whole histogram considered!")
      b_wholeHisto = True
  elif histoType == "1d_phi_Occupancy":
    summaryTitle = "Nb of hits in a region of %.2f around the phi position %.2f - %s"%(args.deltaSpot,args.phiSpot,histoName)
    statement = "I have looked for LBs with at least %.0f entries at phi position %.2f in %s histogram"%(args.minInLB,args.phiSpot,histoName)
    if (args.phiSpot==-999.):
      print("No eta/phi defined -> whole histogram considered!")
      b_wholeHisto = True
  elif histoType == "1d_integralAbove_Occupancy":
    summaryTitle = "Nb of hits in the band above %.2f - %s"%(args.integralAbove,histoName)
    statement = "I have looked for LBs with at least %.0f entries in band above %.2f in %s histogram"%(args.minInLB,args.integralAbove,histoName)
    if (args.integralAbove==-999.):
      print("No lower bound defined -> whole histogram considered!")
      b_wholeHisto = True

  # Definition of Canvas option depending on histogram type
  if (args.objectType == "NumberTightFwdElectrons" or args.objectType == "NumberTau"):
    canvasOption = "logy"
  else:
    canvasOption = ""

  #########################################################################
  # Look for the final merged HIST file
  # and retrieve all relevant merged histograms
  #########################################################################
  runFilePath = "root://eosatlas.cern.ch/%s"%(pathExtract.returnEosHistPath(args.runNumber,args.stream,args.amiTag,args.tag)).rstrip()
  if ("FILE NOT FOUND" in runFilePath):
    print("No merged file found for this run")
    print("HINT: check if there is a folder like","/eos/atlas/atlastier0/tzero/prod/"+args.tag+"/physics_CosmicCalo/00"+str(args.runNumber)+"/"+args.tag+".00"+str(args.runNumber)+".physics_CosmicCalo.*."+args.amiTag)
    sys.exit()

  f = R.TFile.Open(runFilePath)

  histo = {}

  # print("Looking in file",runFilePath)
  for iHisto in histoPath.keys():
    #if histoPath[iHisto] not in histPathList:
    #  print("The desired histo path",histoPath[iHisto],"is not in the input file!")
    histo[iHisto] = f.Get(histoPath[iHisto])
    histo[iHisto].SetTitle("%s (%s) - Run %d"%(histo[iHisto].GetTitle(),histoLegend[iHisto],args.runNumber))

  #########################################################################
  # Extract the list of bins where to count.
  # Scans the window to find all bins that fall in the window
  # The regionBins is defined for each histogram allowing different binning
  #########################################################################
  regionBins = {}
  for iHisto in histoPath.keys():
    if b_wholeHisto:
      regionBins[iHisto] = []
      if ("2d" in histoType):  
        maxBin = (histo[iHisto].GetNbinsX()+2)*(histo[iHisto].GetNbinsY()+2)
      else:
        maxBin = (histo[iHisto].GetNbinsX()+2)
      for iBin in range(maxBin):
        regionBins[iHisto].append(iBin)
    else:
      nSteps = 1000
      subStep = 2*args.deltaSpot/nSteps
      regionBins[iHisto] = []
      if histoType == "2d_etaPhi_Occupancy" or histoType == "2d_etaPhi_MeanPt":
        for ix in range(nSteps):# Assume that eta is on x axis
          iEta = args.etaSpot - args.deltaSpot + ix * subStep 
          for iy in range (nSteps):
            iPhi = args.phiSpot - args.deltaSpot + iy * subStep
            tmpBin = histo[iHisto].FindBin(iEta,iPhi)
            if (tmpBin not in regionBins[iHisto]):
              regionBins[iHisto].append(tmpBin)
      elif histoType == "2d_xy_Occupancy":
        for ix in range(nSteps):
          iX = args.xSpot - args.deltaSpot + ix * subStep 
          for iy in range (nSteps):
            iY = args.ySpot - args.deltaSpot + iy * subStep
            tmpBin = histo[iHisto].FindBin(iX,iY)
            if (tmpBin not in regionBins[iHisto]):
              regionBins[iHisto].append(tmpBin)
      elif histoType == "1d_eta_Occupancy":
        for ix in range(nSteps):
          iEta = args.etaSpot - args.deltaSpot + ix * subStep
          tmpBin = histo[iHisto].FindBin(iEta)
          if (tmpBin not in regionBins[iHisto]):
              regionBins[iHisto].append(tmpBin)
      elif histoType == "1d_phi_Occupancy":
        for ix in range(nSteps):
          iPhi = args.phiSpot - args.deltaSpot + ix * subStep
          tmpBin = histo[iHisto].FindBin(iPhi)
          if (tmpBin not in regionBins[iHisto]):
              regionBins[iHisto].append(tmpBin)
      elif (histoType == "1d_integralAbove_Occupancy"):
        for iBin in range(histo[iHisto].FindBin(args.integralAbove),histo[iHisto].GetNbinsX()):
          regionBins[iHisto].append(iBin)          

  #########################################################################
  # Display the Tier0 merged histograms
  #########################################################################

  c = {}
  box = {}
  line = {}
  line2 = {}
  arrow = {}
  for iHisto in histoPath.keys():
    total_tmp = 0
    for iBin in regionBins[iHisto]:
      total_tmp = total_tmp + histo[iHisto].GetBinContent(iBin)
    if total_tmp == 0: # If no entry in the concerned area, do not display the merged histogram
      continue

    c[iHisto] = R.TCanvas(histoLegend[iHisto])
    if "logy" in canvasOption:
      c[iHisto].SetLogy(1)
    # draw line, arrows, box to highlight the suspicious region considered
    if histoType.startswith("2d"):
      R.gStyle.SetPalette(1)
      R.gStyle.SetOptStat("")
      #print(iHisto, histo[iHisto].GetEntries())
      histo[iHisto].Draw("COLZ")
      if not b_wholeHisto:
        if histoType == "2d_etaPhi_Occupancy" or histoType == "2d_etaPhi_MeanPt":
          box[iHisto] = R.TBox(args.etaSpot-args.deltaSpot,args.phiSpot-args.deltaSpot,args.etaSpot+args.deltaSpot,args.phiSpot+args.deltaSpot)
        elif (histoType == "2d_xy_Occupancy"):
          box[iHisto] = R.TBox(args.xSpot-args.deltaSpot,args.ySpot-args.deltaSpot,args.xSpot+args.deltaSpot,args.ySpot+args.deltaSpot)
        box[iHisto].SetLineColor(R.kRed+1)
        box[iHisto].SetLineWidth(3)
        box[iHisto].SetFillStyle(0)
        box[iHisto].Draw()

    elif histoType.startswith("1d"):
      maxH = histo[iHisto].GetMaximum()*1.2
      histo[iHisto].SetMaximum(maxH)
      if histoType == "1d_eta_Occupancy" or histoType == "1d_phi_Occupancy":
        minH = histo[iHisto].GetMinimum()*0.8
        histo[iHisto].SetMinimum(minH)
      histo[iHisto].Draw()

      if not b_wholeHisto:
        if histoType == "1d_eta_Occupancy" or histoType == "1d_phi_Occupancy":
          if maxH >0.:
            if histoType == "1d_eta_Occupancy": 
              box[iHisto] = R.TBox(args.etaSpot-args.deltaSpot,minH,args.etaSpot+args.deltaSpot,maxH)
            if histoType == "1d_phi_Occupancy": 
              box[iHisto] = R.TBox(args.phiSpot-args.deltaSpot,minH,args.phiSpot+args.deltaSpot,maxH)
            box[iHisto].SetLineColor(R.kRed+1)
            box[iHisto].SetLineWidth(3)
            box[iHisto].SetFillStyle(0)
            box[iHisto].Draw()
        elif histoType == "1d_integralAbove_Occupancy":
          line[iHisto] = R.TLine(args.integralAbove,0,args.integralAbove,maxH)
          line[iHisto].SetLineColor(R.kRed+1)
          line[iHisto].SetLineWidth(3)
          line[iHisto].Draw()
          arrow[iHisto] = R.TArrow(args.integralAbove,0.2*histo[iHisto].GetMaximum(),histo[iHisto].GetBinLowEdge(histo[iHisto].GetNbinsX()),0.2*histo[iHisto].GetMaximum(),0.02,">")
          arrow[iHisto].SetLineColor(R.kRed+1)
          arrow[iHisto].SetLineWidth(3)
          arrow[iHisto].Draw()

  #########################################################################
  # Loop on all unmerged files available
  # and count the number of hits in the concerned area
  #########################################################################

  lbFilePathList = pathExtract.returnEosHistPathLB(args.runNumber,args.lowerLumiBlock,args.upperLumiBlock,args.stream,args.amiTag,args.tag)
  nbHitInHot = []
  if isinstance(lbFilePathList,str) and "NOT FOUND" in lbFilePathList:
    print("Could not find per-LB files for this run")
    print("HINT: check if there is a folder like","/eos/atlas/atlastier0/tzero/prod/"+args.tag+"/physics_CosmicCalo/00"+str(args.runNumber)+"/"+args.tag+".00"+str(args.runNumber)+".physics_CosmicCalo.*."+args.amiTag)
    sys.exit()

  LBs = [int(f.split("_lb")[1].split(".")[0]) for f in lbFilePathList]
  maxLB = max(LBs)
  print("Max LB is",maxLB)

  nLB=maxLB
  nbHitInHot = {}
  for iHisto in histoPath.keys():
    nbHitInHot[iHisto] = [0.] * (nLB+1)
  lowerLB = maxLB
  upperLB = 0
  lbCanvas = []
  histoLBNoisy = []
  fLB = {}

  print("I have found the merged HIST file %s"%(runFilePath))
  print("I have found %d unmerged HIST files"%(len(lbFilePathList)))
  print("The first one is root://eosatlas.cern.ch/%s"%(lbFilePathList[0]))
  print("The last one is root://eosatlas.cern.ch/%s"%(lbFilePathList[len(lbFilePathList)-1]))

  # Loop on all unmerged files

  for count,lbFile in enumerate(lbFilePathList):
    lbFilePath = "root://eosatlas.cern.ch/%s"%(lbFile).rstrip()
    # Extract lb from the filename and display it
    ilb = int((lbFile.split("_lb")[1]).split("._")[0])
    if (count%100 == 0):
      sys.stdout.write("\n I processed %d/%d files \n LBs:"%(count,len(lbFilePathList)))
    sys.stdout.write("%d "%(ilb))
    sys.stdout.flush()
    fLB[lbFile] = R.TFile.Open(lbFilePath)
    histoLB = {}
    for iHisto in histoPath.keys():
      histoLB[iHisto] = fLB[lbFile].Get(histoPath[iHisto])
      if (histoLB[iHisto] != None):
        for iBin in regionBins[iHisto]:
          nbHitInHot[iHisto][ilb] = nbHitInHot[iHisto][ilb] + histoLB[iHisto].GetBinContent(iBin)

    fLB[lbFile].Close()

  #########################################################################
  # Loop on all histos and extract range of luminosity block for which
  # the number of hits is above the thereshold
  #########################################################################
  
  for iHisto in histoPath.keys():
    for ilb in range(len(nbHitInHot[iHisto])):
      if (nbHitInHot[iHisto][ilb]>=args.minInLB):
        if ilb<lowerLB : lowerLB = ilb
        if ilb>upperLB : upperLB = ilb  

  if (lowerLB == upperLB):
    lowerLB = lowerLB - 1
    upperLB = upperLB + 4

  print("")
  print(statement)

  maxNbInHot = 0
  maxNbInHot_histo = ""
  totalInRegionRecomp = {} 
  totalInRegion = {} 
  suspiciousLBlist = []

  # Initialize the number of events in suspicious regions for both the merged
  # and the remerged files. 
  for iHisto in histoPath.keys():
    totalInRegionRecomp[iHisto] = 0
    totalInRegion[iHisto] = 0
  # Then count the number of events and check if equal
  # Also sort the LB to highlight most problematic LB
  sortedLB = {}

  for iHisto in histoPath.keys():
    print("======= ",histoLegend[iHisto])
    for iBin in regionBins[iHisto]:
      totalInRegion[iHisto] = totalInRegion[iHisto] + histo[iHisto].GetBinContent(iBin)

    sortedLB[iHisto] = [0] * nLB
    for i in range(nLB):
      totalInRegionRecomp[iHisto] = totalInRegionRecomp[iHisto] + nbHitInHot[iHisto][i]

      sortedLB[iHisto][i] = i
      if (nbHitInHot[iHisto][i]>=args.minInLB):
        suspiciousLBlist.append(i)
      if (nbHitInHot[iHisto][i]>maxNbInHot):
        maxNbInHot = nbHitInHot[iHisto][i]
        maxNbInHot_histo = iHisto

    sortedLB[iHisto].sort(key=dict(zip(sortedLB[iHisto],nbHitInHot[iHisto])).get,reverse=True)
    nLB_aboveThreshold = 0
    for i in range(nLB):
      if nbHitInHot[iHisto][sortedLB[iHisto][i]]>=args.minInLB:
        nLB_aboveThreshold = nLB_aboveThreshold + 1
        if not b_ValueNotEntries:
          print("%d-LB: %d -> %d hits"%(i,sortedLB[iHisto][i],nbHitInHot[iHisto][sortedLB[iHisto][i]]))
        else:
          print("%d-LB: %d -> %.2f"%(i,sortedLB[iHisto][i],nbHitInHot[iHisto][sortedLB[iHisto][i]]))

    if not b_ValueNotEntries:
      print("In the whole run, there are %d entries"%(totalInRegion[iHisto]))
      if (totalInRegionRecomp[iHisto] != totalInRegion[iHisto]):
        print("To be compared with %d entries cumulated from unmerged files"%(totalInRegionRecomp[iHisto]))
        if (totalInRegionRecomp[iHisto] < totalInRegion[iHisto]):
          print("This is normal only if you restricted the LB range...")
        if (totalInRegionRecomp[iHisto] > totalInRegion[iHisto]):
          print("This can be also caused by multiple processing, try to filter with -a option")
          print("File path of the first file:",lbFilePathList[0])
    else:
      print("In the whole run, the value is %.2f"%(totalInRegion[iHisto]))

  affectedLB_forSummaryBinning = []
  if (maxNbInHot != 0):
    for i in range(35):
      if (nbHitInHot[maxNbInHot_histo][sortedLB[maxNbInHot_histo][i]] > 0.05*maxNbInHot):
        affectedLB_forSummaryBinning.append(sortedLB[maxNbInHot_histo][i])

  #########################################################################
  ## Plot evolution vs LB
  #########################################################################
  leg = R.TLegend(0.855, 0.90-0.05*len(histoPath.keys()),
                  0.98, 0.9 )
  leg.SetBorderSize(0)

  if (upperLB>=lowerLB): # check that at least one noisy LB was found
    c0 = R.TCanvas("c0","Evolution vs LB",1400,550)
    R.gStyle.SetOptStat("")
    if histoType != "2d_xy_Occupancy":
      c0.SetLogy(1)
    c0.SetLeftMargin(.045)
    c0.SetRightMargin(.15)
    h0Evol = {}
    first = True
    for iHisto in histoPath.keys():
      h0Evol[iHisto] = R.TH1I("h0Evol%s"%(iHisto),summaryTitle,upperLB-lowerLB+1,lowerLB-0.5,upperLB+0.5)
      h0Evol[iHisto].SetXTitle("Run %d - LumiBlock (Only LB with >= %.0f entries)"%(args.runNumber,args.minInLB))
      h0Evol[iHisto].SetLineColor(histoColor[iHisto])
      h0Evol[iHisto].SetMarkerColor(histoColor[iHisto])
      h0Evol[iHisto].SetMarkerStyle(20)
      for i in range(lowerLB,upperLB+1):
        h0Evol[iHisto].Fill(i,nbHitInHot[iHisto][i])  

      if (h0Evol[iHisto].GetMaximum() >= args.minInLB):
        leg.AddEntry(h0Evol[iHisto],"#splitline{%s}{(%d entries in the run)}"%(histoLegend[iHisto],totalInRegion[iHisto]))

      if first:
        h0Evol[iHisto].Draw("P HIST")
        if histoType != "2d_xy_Occupancy":
          h0Evol[iHisto].SetMinimum(args.minInLB-0.8)
          h0Evol[iHisto].SetMaximum(maxNbInHot*1.5)
        first = False
      else:
        h0Evol[iHisto].Draw("PSAME HIST")
    leg.Draw()
    c0.Update()


  leg2 = R.TLegend(0.855, 0.90-0.05*len(histoPath.keys()),
                  0.98, 0.9 )
  leg2.SetBorderSize(0)

  h_affectedLB = {}
  
  print("Affected luminosity blocks sorted by the noise amplitude",affectedLB_forSummaryBinning)

  if (len(affectedLB_forSummaryBinning)>0):
    c0_2 = R.TCanvas("c0_2","Most affected LB summary", 1400,550)
    R.gStyle.SetOptStat("")
    if histoType != "2d_xy_Occupancy":
      c0_2.SetLogy(1)
    c0_2.SetLeftMargin(.045)
    c0_2.SetRightMargin(.15)

    onlineLumi = GetOnlineLumiFromCOOL(args.runNumber,0)
    cumulatedAffectedLumi = 0.

    first = True
    for iHisto in histoPath.keys():
      h_affectedLB[iHisto] = R.TH1F("affectedLB%s"%(iHisto),"%s - Most affected LB"%summaryTitle,len(affectedLB_forSummaryBinning),0.5,len(affectedLB_forSummaryBinning)+0.5)
        
      h_affectedLB[iHisto].SetXTitle("Run %d - LumiBlock / Cumulated luminosity in pb-1"%(args.runNumber))
      h_affectedLB[iHisto].SetLineColor(histoColor[iHisto])
      h_affectedLB[iHisto].SetMarkerColor(histoColor[iHisto])
      h_affectedLB[iHisto].SetMarkerStyle(20)

      for i in range(1,len(affectedLB_forSummaryBinning)+1):
        cumulatedAffectedLumi = cumulatedAffectedLumi + onlineLumi[affectedLB_forSummaryBinning[i-1]]*60/1e6
        h_affectedLB[iHisto].GetXaxis().SetBinLabel(i,"%d / %.1f"%(affectedLB_forSummaryBinning[i-1],cumulatedAffectedLumi))
        h_affectedLB[iHisto].GetXaxis().SetTitleOffset(1.4)
        h_affectedLB[iHisto].Fill(i,nbHitInHot[iHisto][affectedLB_forSummaryBinning[i-1]])

      if (h_affectedLB[iHisto].GetMaximum() >= args.minInLB):
        leg2.AddEntry(h_affectedLB[iHisto],"#splitline{%s}{(%d entries in the run)}"%(histoLegend[iHisto],totalInRegion[iHisto]))
        
      if first:
        h_affectedLB[iHisto].Draw("P HIST")
        if histoType != "2d_xy_Occupancy":
          h_affectedLB[iHisto].SetMinimum(args.minInLB-0.8)
          h_affectedLB[iHisto].SetMaximum(maxNbInHot*1.5)
        first = False
      else:
        h_affectedLB[iHisto].Draw("PSAME HIST")

    leg2.Draw()

    c0_2.SetGridx()
    c0_2.Update()

    print("WARNING: only the 35 most affected LB are displayed in the summary plot")

      
  if args.defectQuery:
    print("I am looking for LAr/Tile/Calo defects defined for the suspicious LB")
    from DQDefects import DefectsDB
    db = DefectsDB()
    defectList = [d for d in (db.defect_names | db.virtual_defect_names) if ((d.startswith("LAR") and "SEV" in d) or (d.startswith("TILE")) or (d.startswith("CALO")))]
    defects = db.retrieve((args.runNumber, 1), (args.runNumber+1, 0), defectList)
    for iDef in defects:
      associatedSuspicious = False
      for iLB in range(iDef.since.lumi,iDef.until.lumi):
        if iLB in suspiciousLBlist:
          associatedSuspicious = True
      if associatedSuspicious:
        if (iDef.since.lumi == iDef.until.lumi-1):
          print("%s: %d set by %s - %s"%(iDef.channel,iDef.since.lumi,iDef.user,iDef.comment))
        else:
          print("%s: %d->%d set by %s - %s"%(iDef.channel,iDef.since.lumi,iDef.until.lumi-1,iDef.user,iDef.comment))

  input("Please press the Enter key to proceed")

  sys.exit()

############################################################################################################################################################################
############################################################################################################################################################################
if __name__ == "__main__":
  R.gStyle.SetPalette(1)
  R.gStyle.SetOptStat("em")
  # command-line arguments
  parser = argparse.ArgumentParser(description='Process some integers.')
  parser.add_argument('-r','--run',type=int,dest='runNumber',default='267599',help="Run number",action='store')
  parser.add_argument('-ll','--lowerlb',type=int,dest='lowerLumiBlock',default='0',help="Lower lb",action='store')
  parser.add_argument('-ul','--upperlb',type=int,dest='upperLumiBlock',default='999999',help="Upper lb",action='store')
  parser.add_argument('-s','--stream',dest='stream',default='Main',help="Stream with or without prefix: express/CosmicCalo/Main/ZeroBias/MinBias",action='store')
  parser.add_argument('-t','--tag',dest='tag',default='',help="DAQ tag: data16_13TeV, data16_cos... By default retrieve it via atlasdqm",action='store')
  parser.add_argument('-a','--amiTag',dest='amiTag',default='f',help="First letter of AMI tag: x->express / f->bulk",action='store')
  parser.add_argument('-e','--eta',type=float,dest='etaSpot',default='-999.',help='Eta of hot spot',action='store')
  parser.add_argument('-p','--phi',type=float,dest='phiSpot',default='-999.',help='Phi of hot spot',action='store')
  parser.add_argument('-x','--x',type=float,dest='xSpot',default='-999.',help='X of hot spot',action='store')
  parser.add_argument('-y','--y',type=float,dest='ySpot',default='-999.',help='Y of hot spot',action='store')
  parser.add_argument('-ia','--integralAbove',type=float,dest='integralAbove',default='-999.',help='Lower bound of integral',action='store')
  parser.add_argument('-d','--delta',type=float,dest='deltaSpot',default='0.1',help='Distance to look around hot spot',action='store')
  parser.add_argument('-o','--object',dest='objectType',default='TopoClusters',help='2D OCCUPANCY: TopoClusters,EMTopoClusters,\n              EMTopoJets,TightFwdElectrons,Tau \n1D OCCUPANCY: EMTopoJets_eta,Tau_eta,Tau_phi \nINTEGRAL    : NumberTau,NumberTightFwdElectrons,NumberHLTJet',action='store')
  parser.add_argument('-m','--min',type=float,dest='minInLB',default='5',help='Min number of occurences in a LB',action='store')
  parser.add_argument('-g','--grl',dest='defectQuery',help='Look for Calo/LAr/Tile defects set in suspicious LBs',action='store_true')
  
  args = parser.parse_args()

  if args.tag == "":
    # Try to retrieve the data project tag via atlasdqm
    if (not os.path.isfile("atlasdqmpass.txt")):
      print("To retrieve the data project tag, you need to generate an atlasdqm key and store it in this directory as atlasdqmpass.txt (yourname:key)")
      print("To generate a key, go here : https://atlasdqm.cern.ch/dqauth/")
      print("You can also define by hand the data project tag wit hthe option -t")
      sys.exit()
    passfile = open("atlasdqmpass.txt")
    passwd = passfile.read().strip(); passfile.close()
    passurl = 'https://%s@atlasdqm.cern.ch'%passwd
    s = xmlrpclib.ServerProxy(passurl)
    run_spec = {'stream': 'physics_CosmicCalo', 'proc_ver': 1,'source': 'tier0', 'low_run': args.runNumber, 'high_run':args.runNumber}
    run_info= s.get_run_information(run_spec)
    if '%d'%args.runNumber not in run_info.keys() or len(run_info['%d'%args.runNumber])<2:
      print("Unable to retrieve the data project tag via atlasdqm... Please double check your atlasdqmpass.txt or define it by hand with -t option")
      sys.exit()
    args.tag = run_info['%d'%args.runNumber][1]

  
  # parser.print_help()
  main(args)
