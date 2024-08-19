# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
# Author : Benjamin Trocme (CNRS/IN2P3 - LPSC Grenoble)- 2017 - 2023
# Python 3 migration by Miaoran Lu (University of Iowa)- 2022
#
# Definition of system/year/tag characteristisc and functions used by DeMoUpdate, DeMoStatus
# and DemoScan
#
# Documentation: https://twiki.cern.ch/twiki/bin/viewauth/Atlas/DataQualityDemo
#############################################################################################

from ROOT import THStack
from ROOT import TCanvas,TLegend
from ROOT import kYellow,kOrange,kRed,kBlue,kPink,kMagenta,kGreen,kSpring,kViolet,kAzure,kCyan,kTeal,kBlack,kWhite
from ROOT import TProfile,TH1F

import time,os,subprocess

def MakeTProfile(name,xtitle,ytitle,xmin,xmax,nbins,color):
   h = TProfile(name,name, nbins, xmin, xmax)
   h.SetYTitle(ytitle)
   h.SetXTitle(xtitle)
   h.SetMarkerStyle(22)
   h.SetMarkerColor(color)
   h.SetLineColor(1)
   h.SetFillColor(color)
   h.SetStats(0)
   h.SetTitle("")
   return h

def MakeTH1(name,xtitle,ytitle,xmin,xmax,nbins,color):
   h = TH1F(name,name,nbins, xmin,xmax)
   h.SetYTitle(ytitle)
   h.SetXTitle(xtitle)
   h.SetLineColor(1)
   h.SetMarkerStyle(22)
   h.SetMarkerColor(color)
   h.SetFillColor(color)
   h.Sumw2()
   return h

def SetXLabel(h,list):
    for i in range(len(list)):
         h.GetXaxis().SetBinLabel(i+1,str(list[i]))
    return

def MakeLegend(xmin,ymin,xmax,ymax):
    l = TLegend(xmin,ymin,xmax,ymax)
    l.SetFillColor(kWhite)
    l.SetTextSize(0.03)
    l.SetBorderSize(0)
    return l

########################################################################
# Return the data period of a run based on the information stored in
# YearStats-[system]/[year]/[tag]/runs-ALL.dat
def returnPeriod(runNb,system,year,tag):
   p = subprocess.Popen(["grep","%d"%runNb,"YearStats-%s/%s/%s/runs-ALL.dat"%(system,year,tag)],stdout = subprocess.PIPE)
   (o,e) = p.communicate()
   tmp = o.decode().split("(")
   if len(tmp)>0:
      tmp2 = tmp[1].split(")")[0]

   return tmp2
   
########################################################################
# Return the year/tag properties (defect/veto/lumi tags...) stored in
# YearStats-common/DeMoConfig-[year]-[tag].dat
def retrieveYearTagProperties(year,tag):
   ytp = {"Description":"","Defect tag":"","Veto tag":"","OflLumi tag":"","OflLumiAcct tag":""}
   DeMoConfigFile = open("YearStats-common/%s/DeMoConfig-%s-%s.dat"%(year,year,tag),"r")
   for line in DeMoConfigFile:
      for i_ytp in ytp.keys():
         if ("%s: "%i_ytp in line):
            ytp[i_ytp] = (line.split("%s: "%i_ytp)[1]).replace("\n","")
   DeMoConfigFile.close()

   for i_ytp in ytp.keys():
      if (ytp[i_ytp] == ""):
         print("ERROR: Missing (%s) in YearStats-common/%s/DeMoConfig-%s-%s.dat -> Please check or define it with DeMoSetup.py"%(i_ytp,year,year,tag))

   return ytp

########################################################################
# Return a string with the luminosity in a human readable way
# If the unit is %%, this is a percentage
def strLumi(lumi,unit="ub",latex = True,floatNumber = False):
  if (unit == "%%"):
    string0 = "%.2f%% "%(lumi)
  else:
    if (unit == "pb" or unit == "pb^{-1}" or unit == "pb-1"):
      lumi = lumi*1e6

    if lumi < 1e3:
      if latex:
        if (floatNumber):string0 = "%.2f #mub"%(lumi)
        else:string0 = "%.0f #mub"%(lumi)
      else:
        if (floatNumber):string0 = "%.2f ub"%(lumi)  
        else:string0 = "%.0f ub"%(lumi)  
    elif lumi<1e6:
      if (floatNumber):string0 = "%.2f nb"%(lumi/1e3)
      else: string0 = "%.0f nb"%(lumi/1e3)
    elif lumi<1e9:
      if (floatNumber):string0 = "%.2f pb"%(lumi/1e6)    
      else:string0 = "%.0f pb"%(lumi/1e6)    
    else:
      if (floatNumber):string0 = "%.3f fb"%(lumi/1e9)
      else:string0 = "%.1f fb"%(lumi/1e9)
    if latex:
      string0= string0+"^{-1}"
    else:
      string0= string0+"-1"

  return string0


########################################################################
def plotStack(name,histo,index,indexName,histoIntLumi,lumiBool,resStack,resCanvas,resLegend,recovBool = True,compBool = False,approvedPlots = False):
# name: Mainly an index of the output. Also used to define TAxis Title
# histo: dict of histograms or TProfile to be displayed
# index: list of keys of histo to be displayed
# indexName: dict of namesof index used for the TLegend
# histoIntLumi : integrated lumi with the same x binning as histo
# lumiBool : display results in term of lumi and not percent
# resStack, resCanvas, resLegend: dict of (stacks, canvas, legend) outputs
# recovBool: display the recoverable histograms (referenced as %%%__recov in histo
# compBool: this is a >=2 yearTag plots. Write all numbers in TLegend and not only last bin (that is not meaningful in yearTag)

  # unit is the main unit. unitAux is the complementary one used only the TLegend
  if (lumiBool):
    unit = "pb^{-1}"
    unitAux = "%%"
  else:
    unit = "%%"
    unitAux = "pb^{-1}"

  nameSplitted = name.split("--") # Assume that the name is "Veto/defect (y axis) - Year/Run (x axis)- Dataset name"
  xAxisTitle = nameSplitted[1]
  if unit == "%%":
    yAxisTitle = "Lost luminosity due to %s [%%]"%(nameSplitted[0])
  else:
    yAxisTitle = "Lost luminosity due to %s [%s]"%(nameSplitted[0],unit)
  legendHeader = "%s - %s"%(nameSplitted[2],time.strftime("%d %b", time.localtime()))

  resCanvas[name] = TCanvas(name,"%s - %s"%(yAxisTitle,xAxisTitle),100, 10, 1350, 500)
  resCanvas[name].SetLeftMargin(0.08)
  resCanvas[name].SetRightMargin(0.35)
  resCanvas[name].SetGridy(1)
  resStack[name] = THStack("%s_stack"%name,"")
  resLegend[name] = MakeLegend(0.66,0.8,0.98,0.95) # Y1 will be redefined later according to the number of entries
  resLegend[name].AddEntry(0, "", "")

  first = True

  nBinsX = histoIntLumi.GetNbinsX()
  totalIneff = 0.
  if (compBool):
    totalIneff_comp = nBinsX * [0.]

  totalIntegratedLumi = histoIntLumi.GetBinContent(nBinsX)
  if lumiBool:
    auxScaleFactor = 100./totalIntegratedLumi
  else:
    auxScaleFactor = totalIntegratedLumi/100.

  tmpColor = [kBlue-4,kOrange-7,kTeal+1,kMagenta-4,kPink-3,kGreen+3,kSpring-3,kViolet+4,kAzure-8,kCyan+1]
  tmpColorIndex = 0

  for iIndex in sorted(index,reverse=True):
    if first: # Create a recoverable histograms just in case of
      resStack["%s__recov"%name] = MakeTH1("h1_%s__recovTotal"%(name),"Recoverable","",-0.5,-0.5+nBinsX,nBinsX,1)
      resStack["%s__recov"%name].SetMarkerStyle(23)  
      first = False
    iIndexName = iIndex.split("_")[0]

    # Define a "temporary" color for the defect histogram for which it was not yet defined
    # Mandatory for defect that are not yet known as impacting the system (i.e. with a loss-[defect].dat file
    # Especially useful for weekly reports with a new type of defect affecting the system
    if (histo[iIndex].GetFillColor() == kBlack and tmpColorIndex < len(tmpColor) and histo[iIndex].GetBinContent(histo[iIndex].GetNbinsX()) != 0):
      histo[iIndex].SetFillColor(tmpColor[tmpColorIndex])
      tmpColorIndex = tmpColorIndex + 1

    # Fill histo["%s_toStack"] the main histo
    # and histo["%s_aux"] the complementary one used only for TLegend
    if (histo[iIndex]).IsA().InheritsFrom("TProfile"):
      histo['%s_toStack'%iIndex] = histo[iIndex].ProjectionX()
      histo['%s_toStack'%iIndex].SetFillColor(histo[iIndex].GetFillColor())
    else:
      histo['%s_toStack'%iIndex] = histo[iIndex]
    if lumiBool:
      histo['%s_toStack'%iIndex].Multiply(histo['%s_toStack'%iIndex],histoIntLumi,0.01)
    histo['%s_toStack'%iIndex].LabelsOption("v")
    resStack[name].Add(histo['%s_toStack'%iIndex])

  entryNb = 0
  for iIndex in sorted(index): # Reverse order to have the TLegend ordered as the stacks
    iIndexName = iIndex.split("__")[0] # Trick needed to get the defect name for the recoverable defect histogram    
    baseEntry = "%s"%(strLumi(histo['%s_toStack'%iIndex].GetBinContent(nBinsX),unit))
    auxEntry = "%s"%(strLumi(histo['%s_toStack'%iIndex].GetBinContent(nBinsX)*auxScaleFactor,unitAux))

    if (recovBool and "%s__recov"%iIndex in list(histo.keys()) and histo["%s__recov"%iIndex].GetBinContent(nBinsX) != 0.):
      baseEntryRecov = "%s"%(strLumi(histo["%s__recov"%iIndex].GetBinContent(nBinsX),unit))
      entry = "#splitline{%s}{%s(recov:%s) / %s}"%(indexName[iIndexName],baseEntry,baseEntryRecov,auxEntry) # Second part of Legend to fix
      for iBin in range(nBinsX+1):
        resStack["%s__recov"%name].Fill(iBin-1,histo["%s__recov"%iIndex].GetBinContent(iBin))
    else:
      entry = "#splitline{%s}{%s / %s}"%(indexName[iIndex],baseEntry,auxEntry)

    if (compBool): # This is a >=2 yearTag histograms
      if histo[iIndex].GetNbinsX() >= 2 and histo[iIndex].GetNbinsX()<5:
        if histo[iIndex].GetBinContent(1) != 0.:
          entry = "#splitline{%s}{%s"%(indexName[iIndexName],strLumi(histo[iIndex].GetBinContent(1),unit))
        else:
          entry = "#splitline{%s}{     -     "%(indexName[iIndexName])
        for iTag in range(2,histo[iIndex].GetNbinsX()+1):
          if histo[iIndex].GetBinContent(iTag) != 0.:
            entry += " / %s "%(strLumi(histo[iIndex].GetBinContent(iTag),unit))
          else:
            entry += " /     -      "
        entry += "}"
      else:
        entry = "%s"%(indexName[iIndexName])

    if (histo[iIndex].GetMaximum() != 0): 
      resLegend[name].AddEntry(histo[iIndex],entry,"f")
      entryNb = entryNb+1

  # Extract the total inefficiency 
  allIndex = ""
  if ("allIntol" in list(histo.keys())):
    allIndex = "allIntol"
  if ("allVeto" in list(histo.keys())):
    allIndex = "allVeto"

  if allIndex != "":
    if compBool:
      for iBin in range(1,nBinsX+1):
        totalIneff_comp[iBin-1] = histo[allIndex].GetBinContent(iBin)          
    else:
      totalIneff = histo[allIndex].GetBinContent(nBinsX)

  mx = resStack[name].GetMaximum()*1.2
  resStack[name].SetMaximum(mx)
  resStack[name].Draw("hist")
  resStack[name].GetXaxis().LabelsOption("v")
  resStack[name].GetXaxis().SetLabelSize(0.04)
  resStack[name].GetXaxis().SetTitle("%s"%xAxisTitle)
  resStack[name].GetXaxis().SetTitleOffset(1.45)
  resStack[name].GetYaxis().SetTitle("%s"%yAxisTitle)
  resStack[name].GetYaxis().SetTitleOffset(0.7)
  if resStack[name].GetMaximum()>10.:
    resCanvas[name].SetLogy(1)

  if compBool:
    resStack[name].GetXaxis().SetLabelSize(0.065)
    if histo[iIndex].GetNbinsX() == 2:
      resLegend[name].SetHeader("#splitline{%s}{Total loss: %s / %s}"%(legendHeader,strLumi(totalIneff_comp[0],unit),strLumi(totalIneff_comp[1],unit)))
    elif histo[iIndex].GetNbinsX() == 3:
      resLegend[name].SetHeader("#splitline{%s}{Total loss: %s / %s / %s}"%(legendHeader,strLumi(totalIneff_comp[0],unit),strLumi(totalIneff_comp[1],unit),strLumi(totalIneff_comp[2],unit)))
    elif histo[iIndex].GetNbinsX() == 4:
      resLegend[name].SetHeader("#splitline{%s}{Total loss: %s /  %s /  %s /  %s}"%(legendHeader,strLumi(totalIneff_comp[0],unit),strLumi(totalIneff_comp[1],unit),strLumi(totalIneff_comp[2],unit),strLumi(totalIneff_comp[3],unit)))
    else:
      resLegend[name].SetHeader("%s"%(legendHeader))
  else:
    totalIneffAux = totalIneff*auxScaleFactor
    if (approvedPlots):
      resLegend[name].SetHeader("#splitline{%s}{Total loss: %s / %s}"%(legendHeader,strLumi(totalIneff,unit),strLumi(totalIneffAux,unitAux)))
    else:
       if (totalIneff != 0.):
          resLegend[name].SetHeader("#splitline{%s (%s)}{Total loss: %s / %s}"%(legendHeader,strLumi(totalIntegratedLumi,"pb"),strLumi(totalIneff,unit),strLumi(totalIneffAux,unitAux)))
       else:
          resLegend[name].SetHeader("%s (%s)"%(legendHeader,strLumi(totalIntegratedLumi,"pb")))
  if resStack["%s__recov"%name].GetEntries() != 0.:
    resStack["%s__recov"%name].SetMarkerStyle(20)
    resStack["%s__recov"%name].SetMarkerColor(kAzure+8)
    resStack["%s__recov"%name].Draw("PSAME HIST")
    resLegend[name].AddEntry(resStack["%s__recov"%name],"#splitline{Recoverable}{total: %.2f%%}"%(resStack["%s__recov"%name].GetBinContent(nBinsX)),"p")
    entryNb = entryNb + 1
   
  if (entryNb<9):
     resLegend[name].SetTextSize(0.035)
  else:
     resLegend[name].SetTextSize(0.028)

  resLegend[name].SetY1(max(0.83-entryNb*0.098,0.05))
  resLegend[name].Draw()

  resCanvas[name].Update()
  
  return totalIneff # totalIneff is used only with the savePage1 option in DeMoStatus

#########################################################################################
#########################################################################################
def initializeMonitoredDefects(system,partitions,defects0,defectVeto,veto,signOff,year,tag,runlist = {}):

  runlist["filename"] = "runlist-%s-%s.dat"%(year,tag)

#################################### NEWSYSTEM defects
###  if system == "NEWSYSTEM":
###    partitions["color"] = {}
###    partitions["list"] = partitions["color"].keys()
###
###    defects0["prefix"] = ["NEWSYSTEM"]
###    # Partition intolerable and tolerable defects - Order determines what defect is proeminent
###    # Warning : do not remove/edit the comment specifying the system. It is used to display the defects in the webpage
###    defects0["partIntol"] = [] # NEWSYSTEM system
###    defects0["partTol"] = [] # NEWSYSTEM system
###    # Global intolerable and tolerable defects
###    # Warning : do not remove/edit the comment specifying the system. It is used to display the defects in the webpage
###    defects0["globIntol"] = []  # NEWSYSTEM system
###    defects0["globTol"] = []  # NEWSYSTEM system
###    
###    veto["all"] = [] # Veto name as defined in the COOL database
###    veto["COOL"] = {} # Veto name as defined in the COOL database
###
###    defectVeto["description"] = {"":""}
###
###    signOff["EXPR."] = ["NEWSYSTEM_UNCHECKED"]
###    signOff["BULK"] = ["NEWSYSTEM_BULK_UNCHECKED"]
###    signOff["FINAL"] = []
###


  
#################################### Pixel defects
  if system == "Pixel":
    partitions["color"] = {'IBL':kYellow-9,'LAYER0':kYellow,'BARREL':kOrange,'ENDCAPC':kOrange-3,'ENDCAPA':kRed-3}
    partitions["list"] = list(partitions["color"].keys())

    defects0["prefix"] = ["PIXEL"]
    # Partition intolerable and tolerable defects - Order determines what defect is proeminent
    # Warning : do not remove/edit the comment specifying the system. It is used to display the defects in the webpage
    defects0["partIntol"] = ["DISABLED","GT30pct_NOTREADY","READOUT_PROBLEM","HVSCAN","TIMING","STANDBY"] # Pixel system
    defects0["partTol"] = [] # Pixel system
    # Global intolerable and tolerable defects
    # Warning : do not remove/edit the comment specifying the system. It is used to display the defects in the webpage
    defects0["globIntol"] = ["UNKNOWN"]  # Pixel system
    defects0["globTol"] = []  # Pixel system
    
    veto["all"] = [] # Veto name as defined in the COOL database
    veto["COOL"] = {} # Veto name as defined in the COOL database

    defectVeto["description"] = {"DISABLED":"One layer disabled",
                                 "IBL_DISABLED":"IBL disabled",
                                 "GT30pct_NOTREADY":">30% modules in error",
                                 "READOUT_PROBLEM":"Readout problem",
                                 "IBL_READOUT_PROBLEM":"IBL readout problem",
                                 "HVSCAN":"HV scan",
                                 "TIMING":"Timing scan",
                                 "UNKNOWN":"Unknown",
                                 "STANDBY":"Standby"}

    signOff["EXPR."] = ["PIXEL_UNCHECKED"]
    signOff["BULK"] = ["PIXEL_BULK_UNCHECKED"]
    signOff["FINAL"] = []
  
################################# SCT defects
# https://twiki.cern.ch/twiki/bin/view/Atlas/SCTOfflineMonitoringShifts#List_of_Defects
  if system == "SCT":
    partitions["color"] = {}
    partitions["list"] = list(partitions["color"].keys())

    defects0["prefix"] = ["SCT"]
    # Partition intolerable and tolerable defects - Order determines what defect is proeminent
    # Warning : do not remove/edit the comment specifying the system. It is used to display the defects in the webpage
    defects0["partIntol"] = []
    defects0["partTol"] = []
    # Global intolerable and tolerable defects
    # Warning : do not remove/edit the comment specifying the system. It is used to display the defects in the webpage
    defects0["globIntol"] = ["GLOBAL_STANDBY","CRATE_OUT","ROD_OUT_MAJOR","PERIOD_ERR_MAJOR","GLOBAL_DESYNC","GLOBAL_RECONFIG","GLOBAL_UNKNOWN","GLOBAL_DISABLED","SIMULATION_DATA_FLAG"] # SCT system
    defects0["globTol"] = ["MOD_OUT_GT40","MOD_ERR_GT40","MOD_NOISE_GT40","PERIOD_ERR_GT40","ROD_OUT","EFF_LT99","NOTNOMINAL_HV","NOTNOMINAL_THRESHOLD","NOTNOMINAL_TIMING","COOLINGLOOP_OUT_1","PS_TRIP"] # SCT system

    defectVeto["description"] = {"GLOBAL_STANDBY":"Standby", # Intolerable defects
                                 "CRATE_OUT":">=1 crate out",
                                 "ROD_OUT_MAJOR":"Large inefficency (ROD)",
                                 "PERIOD_ERR_MAJOR":"Large inefficiency (Period)",
                                 "GLOBAL_DESYNC":"Global desync",
                                 "GLOBAL_RECONFIG":"Global reconfig",
                                 "GLOBAL_UNKNOWN":"Unknown",
                                 "GLOBAL_DISABLED":"Disabled",
                                 "SIMULATION_DATA_FLAG":"Simulation data flag wrongly activated",
                                 "MOD_OUT_GT40":"More than 40 modules excluded in DAQ in addition to the permanent disabled modules (37 modules as of June 2017)", # Tolerable defects
                                 "MOD_ERR_GT40":"More than 40 modules with bytestream errors ",
                                 "MOD_NOISE_GT40":"More than 40 noisy modules",
                                 "PERIOD_ERR_GT40":"More than 80 links with errors in a short period of time (fine for the rest of the run), corresponding to about 40 modules",
                                 "ROD_OUT":"One or more ROD(s) excluded from readout, however less than 5% region in eta-phi plane have masked link errors among more than two layers.",
                                 "EFF_LT99":"Less than 99% efficiency for 1st BC and less than 98% efficiency for all bunches in one or more DQ regions",
                                 "NOTNOMINAL_HV":"SCT neither at 150 V nor at 50 V",
                                 "NOTNOMINAL_THRESHOLD":"SCT threshold not at 1 fC",
                                 "NOTNOMINAL_TIMING":"Unusual timing settings, e.g. timing scan",
                                 "COOLINGLOOP_OUT_1":"Loss of a single cooling loop",
                                 "PS_TRIP":"Failed modules due to a power supply trip"}

    veto["all"] = [] # Veto name as defined in the COOL database
    veto["COOL"] = {} # Veto name as defined in the COOL database

    signOff["EXPR."] = ["SCT_UNCHECKED"]
    signOff["BULK"] = ["SCT_BULK_UNCHECKED"]
    signOff["FINAL"] = []

################################# TRT defects
# https://twiki.cern.ch/twiki/bin/view/Atlas/TRTDQDefects
  if system == "TRT":
    partitions["color"] = {}
    partitions["list"] = list(partitions["color"].keys())

    defects0["prefix"] = ["TRT"]
    # Partition intolerable and tolerable defects - Order determines what defect is proeminent
    # Warning : do not remove/edit the comment specifying the system. It is used to display the defects in the webpage
    defects0["partIntol"] = [] # TRT system
    defects0["partTol"] = [] # TRT system
    # Global intolerable and tolerable defects
    # Warning : do not remove/edit the comment specifying the system. It is used to display the defects in the webpage
    defects0["globIntol"] = ["BADCALIBRATION","BADSTRAWLIST","BADGAS","DAQPROBLEMS_OTHER","DESYNC","NODATA_06","DISABLED","UNKNOWN"] # TRT system
    defects0["globTol"] = ["BADCALIBRATION_MINOR","BADGAS_MINOR","BYTESTREAM_BITFLIPS","DAQPROBLEMS_OTHER_MINOR","NODATA_01","NONNOMINAL_HT","NONNOMINAL_LT"] # TRT system

    # Some defects may not exist in past years. Remove them to avoid crashes
    # WARNING: this fix does not work with multiple year plot
    defectVeto["description"] = {"BADCALIBRATION":"Bad calibration",
                                 "BADSTRAWLIST":"Bad dead stram list",
                                 "BADGAS":"Bad gas mixture",
                                 "DAQPROBLEMS_OTHER":"DAQ problems",
                                 "DESYNC":"Desynchronisation",
                                 "NODATA_06":"Large part of TRT off",
                                 "DISABLED":"Disabled",
                                 "UNKNOWN":"Unknown"}

    veto["all"] = [] # Veto name as defined in the COOL database
    veto["COOL"] = {} # Veto name as defined in the COOL database

    signOff["EXPR."] = ["TRT_UNCHECKED"]
    signOff["BULK"] = ["TRT_BULK_UNCHECKED"]
    signOff["FINAL"] = []

################################# LAr defects
# DB tag for the (veto) condition database
# So far, only LAr use event veto
# can be found with the twiki: https://twiki.cern.ch/twiki/bin/viewauth/AtlasComputing/CoolProdTags#Tags_for_RUN_2_Bulk_Data_Process
  if system == "LAr":
    partitions["color"] = { 'EMBA':kYellow-9,'EMBC':kYellow,'EMECA':kOrange,'EMECC':kOrange-3,'HECA':kRed-3,'HECC':kRed+2,'FCALA':kBlue-3,'FCALC':kBlue+2}
    partitions["list"] = list(partitions["color"].keys())

    defects0["prefix"] = ["LAR","CALO_ONLINEDB"]
    # Partition intolerable and tolerable defects - Order determines what defect is proeminent
    # Warning : do not remove/edit the comment specifying the system. It is used to display the defects in the webpage
    defects0["partIntol"] = ["HVTRIP","SEVNOISEBURST","SEVCOVERAGE","HVNONNOMINAL","SEVNOISYCHANNEL","SEVMISCALIB","SEVUNKNOWN"] # LAr system - LAR Prefix - LAR_[PART]_[NAME]
    defects0["partTol"] = ["COVERAGE","HVNONNOM_CORRECTED"] # LAr system
    # Global intolerable and tolerable defects
    # Warning : do not remove/edit the comment specifying the system. It is used to display the defects in the webpage
    defects0["globIntol"] = ["DATACORRUPT","RECOCORRUPT","SPECIALSTUDIES","BADTIMING","LOWMUCONFIG_IN_HIGHMU_EmergencyMeasures"] # LAr system - LAR Prefix - LAR_[NAME] or # CALO Prefix - CALO_[NAME] (last defect)                              
    defects0["globTol"] = [] # LAr system

    defectVeto["description"] = {"HVTRIP":"High voltage trip", # First per partition LAr defects
                                 "NOISEBURST":"Noise bursts (before veto)",
                                 "HVNONNOMINAL":"HV non nominal",
                                 "SEVNOISEBURST":"Noise burst",
                                 "SEVNOISYCHANNEL":"Noisy channels",
                                 "SEVCOVERAGE":"Coverage",
                                 "SEVMISCALIB":"Global miscalibration",
                                 "SEVUNKNOWN":"Unknown reason",
                                 "DATACORRUPT":"Data corruption", # Then global LAr defects
                                 "RECOCORRUPT":"Corrupted reconstruction",
                                 "SPECIALSTUDIES":"Special studies (on purpose)",
                                 "BADTIMING":"Bad timing",
                                 "COVERAGE":"Coverage (tolerable)",
                                 "LOWMUCONFIG_IN_HIGHMU_EmergencyMeasures":"Trigger misconfiguration", # And the global CALO defects
                                 "noiseBurst":"Noise burst", # And finally the LAr veto
                                 "miniNoiseBurst":"Mini noise burst",
                                 "corruption":"Data corruption"}

    veto["all"] = ["noiseBurst","miniNoiseBurst","corruption"] # Veto name as defined in the COOL database
    veto["COOL"] = {"noiseBurst":"allNoise",
                 "miniNoiseBurst":"MNBNoise",
                 "corruption":"allCorruption"} # Veto name as defined in the COOL database

    signOff["EXPR."] = ["LAR_UNCHECKED"]
    signOff["BULK"] = ["LAR_BULK_UNCHECKED"]
    signOff["FINAL"] = ["LAR_UNCHECKED_FINAL"]

################################# Tile defects
# https://twiki.cern.ch/twiki/bin/view/Atlas/TileDQLeaderManual#Global_Tile_Defects
  if system == "Tile":
    partitions["color"] = { 'EBA':kYellow-9,'EBC':kYellow,'LBA':kOrange,'LBC':kOrange-3}
    partitions["list"] = list(partitions["color"].keys())

    defects0["prefix"] = ["TILE"]
    # Partition intolerable and tolerable defects - Order determines what defect is proeminent
    # Warning : do not remove/edit the comment specifying the system. It is used to display the defects in the webpage
    defects0["partIntol"] = ["BAD_COVER","DAQ_PRB","DB_SEVERE","TIMING_SEVERE","UNSPECIFIED_SEVERE"] # Tile system
    defects0["partTol"] = ["DB_MINOR","TIMING_MINOR","TRIP","UNSPECIFIED_MINOR"] # Tile system
    # Global intolerable and tolerable defects
    # Warning : do not remove/edit the comment specifying the system. It is used to display the defects in the webpage
    defects0["globIntol"] = ["LOWSTAT"] # Tile system
    defects0["globTol"] = ["TIMEJUMPS_UNDEFINED"] # Tile system

    # Some defects may not exist in past years. Remove them to avoid crashes
    # WARNING: this fix does not work with multiple year plot
    defectVeto["description"] = {"BAD_COVER":"Coverage",
                                 "DAQ_PRB":"DAQ problem",
                                 "DB_SEVERE":"DB issue",
                                 "TIMING_SEVERE":"Timing issue",
                                 "UNSPECIFIED_SEVERE":"Severe unspecified",
                                 "LOWSTAT":"Low stats"}

    veto["all"] = [] # Veto name as defined in the COOL database
    veto["COOL"] = {} # Veto name as defined in the COOL database

    signOff["EXPR."] = ["TILE_UNCHECKED"]
    signOff["BULK"] = ["TILE_UNCHECKED"]
    signOff["FINAL"] = []

#################################### MUON-CSC defects
# https://twiki.cern.ch/twiki/bin/view/Atlas/MuonMCPDefectList
# Obsolete in Run 3
  if system == "CSC":
    partitions["color"] = {"EA":kYellow-9,'EC':kRed-3}
                           
    partitions["list"] = list(partitions["color"].keys())

    defects0["prefix"] = ["MS_CSC"]
    # Partition intolerable and tolerable defects - Order determines what defect is proeminent
    defects0["partIntol"] = ["STANDBY_HV",
                             "PROBLEM",
                             "ROD_DISABLED",
                             "DISABLED"]

    defects0["partTol"] = ["DISCONNECTED2","LATENCY_MINOR","THRESHOLD"]
    # Global intolerable and tolerable defects
    defects0["globIntol"] = []
    defects0["globTol"] = [] 
    
    veto["all"] = [] # Veto name as defined in the COOL database
    veto["COOL"] = {} # Veto name as defined in the COOL database

    defectVeto["description"] = {"STANDBY_HV":"Standby HV",
                                 "PROBLEM":"Coverage loss > 10%%",
                                 "ROD_DISABLED":">=1 ROD not readout",
                                 "DISABLED":"Disabled"}

    signOff["EXPR."] = ["MS_UNCHECKED"]
    signOff["BULK"] = ["MS_BULK_UNCHECKED"]
    signOff["FINAL"] = []

#################################### MUON-MDT defects
# https://twiki.cern.ch/twiki/bin/view/Atlas/MuonMCPDefectList
  if system == "MDT":
    partitions["color"] = {"EA":kYellow-9,'EC':kRed-3,'BA':kBlue-3,'BC':kOrange-3}
                           
    partitions["list"] = list(partitions["color"].keys())

    defects0["prefix"] = ["MS_MDT"]
    # Partition intolerable and tolerable defects - Order determines what defect is proeminent
    # Warning : do not remove/edit the comment specifying the system. It is used to display the defects in the webpage
    defects0["partIntol"] = ["STANDBY_HV","PROBLEM","ROD_PROBLEM_5orMore","DISABLED"] # MDT system
    defects0["partTol"] = ["ROD_PROBLEM_1","ROD_PROBLEM_2to4"] # MDT system
    # Global intolerable and tolerable defects
    defects0["globIntol"] = [] # MDT system
    defects0["globTol"] = [] # MDT system
    
    veto["all"] = [] # Veto name as defined in the COOL database
    veto["COOL"] = {} # Veto name as defined in the COOL database

    defectVeto["description"] = {"STANDBY_HV":"Standby HV",
                                 "PROBLEM":"Coverage loss > 10%%",
                                 "ROD_PROBLEM_5orMore":">=5 RODs not readout",
                                 "DISABLED":"Disabled"}

    signOff["EXPR."] = ["MS_UNCHECKED"]
    signOff["BULK"] = ["MS_BULK_UNCHECKED"]
    signOff["FINAL"] = []

#################################### MUON-RPC defects
# https://twiki.cern.ch/twiki/bin/view/Atlas/MuonMCPDefectList
  if system == "RPC":
    partitions["color"] = {'BA':kBlue-3,'BC':kOrange-3}
                           
    partitions["list"] = list(partitions["color"].keys())

    defects0["prefix"] = ["MS_RPC"]
    # Partition intolerable and tolerable defects - Order determines what defect is proeminent
    # Warning : do not remove/edit the comment specifying the system. It is used to display the defects in the webpage
    defects0["partIntol"] = ["STANDBY_HV","PROBLEM","PROBLEM_10to15percent","PROBLEM_MoreThan15percent","OutOfSync_3orMore","LowEfficiency_MoreThan10percent","DISABLED"] # RPC system
    defects0["partTol"] = ["LowEfficiency_5to10percent","OutOfSync_2","OutOfSync_1","ROD_PROBLEM_1","PROBLEM_5to10percent"] # RPC system
    # Global intolerable and tolerable defects
    # Warning : do not remove/edit the comment specifying the system. It is used to display the defects in the webpage
    defects0["globIntol"] = [] 
    defects0["globTol"] = [] 
    
    veto["all"] = [] # Veto name as defined in the COOL database
    veto["COOL"] = {} # Veto name as defined in the COOL database

    defectVeto["description"] = {"STANDBY_HV":"Standby HV",
                                 "PROBLEM":"Coverage loss > 10%%",
                                 "PROBLEM_10to15percent":"Coverage loss > 10%%",
                                 "PROBLEM_MoreThan15percent":"Coverage loss > 15%%",
                                 "OutOfSync_3orMore":">3 Out of sync",
                                 "LowEfficiency_MoreThan10percent":"Low efficiency > 10%%",
                                 "DISABLED":"Disabled"}

    signOff["EXPR."] = ["MS_UNCHECKED"]
    signOff["BULK"] = ["MS_BULK_UNCHECKED"]
    signOff["FINAL"] = []

#################################### MUON-TGC defects
# https://twiki.cern.ch/twiki/bin/view/Atlas/MuonMCPDefectList
  if system == "TGC":
    partitions["color"] = {"EA":kYellow-9,'EC':kRed-3}

    partitions["list"] = list(partitions["color"].keys())

    defects0["prefix"] = ["MS_TGC"]
    # Partition intolerable and tolerable defects - Order determines what defect is proeminent
    # Warning : do not remove/edit the comment specifying the system. It is used to display the defects in the webpage
    defects0["partIntol"] = ["STANDBY_HV","PROBLEM","ROD_PROBLEM-2orMore","DISABLED"] # TGC system
    defects0["partTol"] = ["PROBLEM_1"] # TGC system
    # Global intolerable and tolerable defects
    # Warning : do not remove/edit the comment specifying the system. It is used to display the defects in the webpage
    defects0["globIntol"] = [] # TGC system
    defects0["globTol"] = [] # TGC system
    
    veto["all"] = [] # Veto name as defined in the COOL database
    veto["COOL"] = {} # Veto name as defined in the COOL database

    defectVeto["description"] = {"STANDBY_HV":"Standby HV",
                                 "PROBLEM":"Coverage loss > 10%%",
                                 "ROD_PROBLEM_2orMore":">=2 RODs not readout",
                                 "DISABLED":"Disabled"}

    signOff["EXPR."] = ["MS_UNCHECKED"]
    signOff["BULK"] = ["MS_BULK_UNCHECKED"]
    signOff["FINAL"] = []

#################################### MUON CP defects
# https://twiki.cern.ch/twiki/bin/view/Atlas/MuonMCPDefectList
  if system == "MuonCP":
    defects0["prefix"] = ["MCP"]
    # Partition intolerable and tolerable defects - Order determines what defect is proeminent
    # Warning : do not remove/edit the comment specifying the system. It is used to display the defects in the webpage
    defects0["partIntol"] = []
    defects0["partTol"] = []
    # Global intolerable and tolerable defects
    # Warning : do not remove/edit the comment specifying the system. It is used to display the defects in the webpage
    defects0["globIntol"] = ["ALIGN_GEO","LOW_EFFICIENCY_MAJOR"] # MuonCP system
    defects0["globTol"] = ["CHI2_PROBLEM","ID_PROBLEM","LOW_EFFICIENCY_MINOR","MS_PROBLEM"] # MuonCP system
    
    veto["all"] = [] # Veto name as defined in the COOL database
    veto["COOL"] = {} # Veto name as defined in the COOL database

    defectVeto["description"] = {"ALIGN_GEO":"[MCP] Bad alignment/geometry)",
                                 "LOW_EFFICIENCY_MAJOR":"[MCP] Low reconstruction efficiency" }

    signOff["EXPR."] = ["MS_UNCHECKED"]
    signOff["BULK"] = ["MS_BULK_UNCHECKED"]
    signOff["FINAL"] = []

#################################### ID defects
  if system == "IDGlobal":
    partitions["color"] = {}
    partitions["list"] = list(partitions["color"].keys())

    defects0["prefix"] = ["ID"]
    # Partition intolerable and tolerable defects - Order determines what defect is proeminent
    # Warning : do not remove/edit the comment specifying the system. It is used to display the defects in the webpage
    defects0["partIntol"] = []
    defects0["partTol"] = []
    # Global intolerable and tolerable defects
    # Warning : do not remove/edit the comment specifying the system. It is used to display the defects in the webpage
    defects0["globIntol"] = ["NOTRACKS","IBL_TRACKCOVERAGE_SEVERE","PIXEL_TRACKCOVERAGE_SEVERE","SCT_TRACKCOVERAGE_SEVERE","TRT_TRACKCOVERAGE_SEVERE","UNKNOWN","BS_RUNAVERAGE","BS_PARAMETERSTEP","BS_NOTNOMINAL"] # IDGlobal system
    defects0["globTol"] = ["ALIGN_DEGRADED","LOWSTAT","IBL_TRACKCOVERAGE","PIXEL_TRACKCOVERAGE","SCT_TRACKCOVERAGE","TRT_TRACKCOVERAGE","VERTEXBUG"] # IDGlobal system
    
    veto["all"] = [] # Veto name as defined in the COOL database
    veto["COOL"] = {} # Veto name as defined in the COOL database

    defectVeto["description"] = {"NOTRACKS":"No tracks",
                                 "IBL_TRACKCOVERAGE_SEVERE":"[IBL] > 10 %% coverage loss",
                                 "PIXEL_TRACKCOVERAGE_SEVERE":"[PIXEL] > 10 %% coverage loss",
                                 "SCT_TRACKCOVERAGE_SEVERE":"[SCT] > 10 %% coverage loss",
                                 "TRT_TRACKCOVERAGE_SEVERE":"[TRT] > 10 %% coverage loss",
                                 "UNKNOWN":"Unknown",
                                 "BS_RUNAVERAGE":"Problematic BS determination",
                                 "BS_PARAMETERSTEP":"Large changes in BS",
                                 "BS_NOTNOMINAL":"Sizable modulation in d0 vs phi",
                                 "ALIGN_DEGRADED":"Degarded alignment",
                                 "LOWSTAT":"Low statistics",
                                 "x_TRACKCOVERAGE":"Significant change in coverage, but not severe (between 5-10% coverage loss)",
                                 "VERTEXBUG":"Problems in the determination of the primary vertex"}

    signOff["EXPR."] = ["ID_UNCHECKED"]
    signOff["BULK"] = ["ID_BULK_UNCHECKED"]
    signOff["FINAL"] = []

################################# Jet/MET/EGamma/Tau/CaloGlobal defects
# https://twiki.cern.ch/twiki/bin/view/AtlasProtected/EgammaShifts
# https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/JetEtMissDataQuality2016#Jets_defects
# https://twiki.cern.ch/twiki/bin/view/Atlas/TauDataQualityMonitoringRun2#Frequent_problems_or_defects
  if system == "CaloCP":
    partitions["color"] = { 'BARREL':kYellow-9,'CRACK':kRed-3,'ENDCAP':kBlue-3, # EGamma partitions
                            'B':kYellow-9,'CR':kRed-3,'E':kBlue-3, # Tau partitions
                            'CALB':kYellow-9,'CALEA':kRed-3,'CALC':kBlue-3} # CaloGlobal partitions
                            
    partitions["list"] = list(partitions["color"].keys())

    defects0["prefix"] = ["JET","EGAMMA","MET","TAU","CALO_"]
    # Partition intolerable and tolerable defects - Order determines what defect is proeminent
    # Warning : do not remove/edit the comment specifying the system. It is used to display the defects in the webpage
    defects0["partIntol"] = ["LARNOISE","ENERGY","CALO","TRK","KIN","ID","TopoClusterNoiseSevere"] # CaloCP system - EGAMMA Prefix - EGAMMA_[NAME]_[PART] / TAU Prefix- TAU_[PART]_[NAME] / CaloGlobal Prefix - CALO_[PART]_[NAME]
    defects0["partTol"] = [] 
    # Global intolerable and tolerable defects
    # Warning : do not remove/edit the comment specifying the system. It is used to display the defects in the webpage
    defects0["globIntol"] = ["NOJETSREGION","SEVERE_HOTSPOT","BEAMSPOT","Ex_largeshift","Ey_largeshift","SumEt_largeshift"] # CaloCP system - JET Prefix - JET_[NAME] / EGAMMA Prefix - EGAMMA_[NAME] / MET Prefix - MET_[NAME]
    defects0["globTol"] = ["ETAPHI_SPIKES"] # CaloCP system


    defectVeto["description"] = {"NOJETSREGION":"[Jet] Cold region",
                                 "SEVERE_HOTSPOT":"[Jet] Hot region",
                                 "LARNOISE":"[EGamma] Noise in LAr",
                                 "ENERGY":"[EGamma] Problem in energy",
                                 "BEAMSPOT":"[EGamma] Problem in beam spot",
                                 "ETAPHI_SPIKES":"[Egamma] Eta/phi spikes",
                                 "Ex_largeshift":"[MEt] Ex large shift",
                                 "Ey_largeshift":"[MEt] EY large shift",
                                 "SumEt_largeshift":"[MEt] SumEt large shift",
                                 "CALO":"[Tau] Calo problem",
                                 "TRK":"[Tau] Tracking problem",
                                 "KIN":"[Tau] Kinetic problem",
                                 "ID":"[Tau] Identification problem",
                                 "TopoClusterNoiseSevere":"[TopoCluster] Hot spot"}

    veto["all"] = [] # Veto name as defined in the COOL database
    veto["COOL"] = {} # Veto name as defined in the COOL database

    signOff["EXPR."] = ['CALO_UNCHECKED','EGAMMA_UNCHECKED','JET_UNCHECKED','MET_UNCHECKED','TAU_UNCHECKED']
    signOff["BULK"] = ['CALO_BULK_UNCHECKED','EGAMMA_BULK_UNCHECKED','JET_BULK_UNCHECKED','MET_BULK_UNCHECKED','TAU_BULK_UNCHECKED']
    signOff["FINAL"] = []

################################# BTAG defects
# https://twiki.cern.ch/twiki/bin/view/Atlas/FlavourTaggingDataQualityMonitoringShifterInstructions#Run_signoff
  if system == "BTag":
    partitions["color"] = { } # No partition needed
                            
    partitions["list"] = list(partitions["color"].keys())

    defects0["prefix"] = ["BTAG"]
    # Partition intolerable and tolerable defects - Order determines what defect is proeminent
    defects0["partIntol"] = [] # No partition defect                             
    defects0["partTol"] = [] 
    # Global intolerable and tolerable defects
    # Warning : do not remove/edit the comment specifying the system. It is used to display the defects in the webpage
    defects0["globIntol"] = ["BLAYER_SERIOUS_PROBLEM","BTAG_SCT_SERIOUS_PROBLEM","BTAG_TRT_SERIOUS_PROBLEM","BTAG_JET_SEVHOTSPOT"] # BTag system
    defects0["globTol"] = ["BEAMSPOT_SHIFT","BTAG_BLAYER_PROBLEM","BTAG_SCT_PROBLEM","BTAG_TRT_PROBLEM","NOJETS"] # BTag system


    defectVeto["description"] = {"BLAYER_SERIOUS_PROBLEM":"B layer problem",
                                 "BTAG_SCT_SERIOUS_PROBLEM":"SCT problem",
                                 "BTAG_TRT_SERIOUS_PROBLEM":"TRT problem",
                                 "BTAG_JET_SEVHOTSPOT":"Jet hot spot",
                                 "BEAMSPOT_SHIFT":"Beamspot shift",
                                 "BTAG_NOJETS":"No jets in monitoring"}

    veto["all"] = [] # Veto name as defined in the COOL database
    veto["COOL"] = {} # Veto name as defined in the COOL database

    signOff["EXPR."] = ['BTAG_UNCHECKED']
    signOff["BULK"] = ['BTAG_BULK_UNCHECKED']
    signOff["FINAL"] = []

#################################### TRIG_L1 defects
# https://twiki.cern.ch/twiki/bin/view/Atlas/DataQualityTriggerDefects
  if system == "Trig_L1":
    partitions["color"] = {}
    partitions["list"] = list(partitions["color"].keys())

    defects0["prefix"] = ["TRIG_L1"]
    # Partition intolerable and tolerable defects - Order determines what defect is proeminent
                             

    defects0["partIntol"] = []
    defects0["partTol"] = []
    # Global intolerable and tolerable defects
    # Warning : do not remove/edit the comment specifying the system. It is used to display the defects in the webpage
    defects0["globIntol"] = ["CAL_coverage","CAL_misconf_calib","CAL_misconf_electronics","CAL_misconf_satBCID","CAL_off","MUB_busy","MUB_coverage","MUB_failed_electronics","MUB_lost_sync","MUB_misconf_electronics","MUE_busy","MUE_coverage","MUE_misconf_electronics","MUE_pt15GeV","MUE_pt20GeV","CTP_CTP_MuCTPI_bcid","CTP_CTP_ROD_bcid","CTP_CTPsim","CTP_TAPnoTBP","CTP_TAVnoTAP","CTP_UNKNOWN","CTP_bcid","CTP_bcidrange","CTP_candnumber","CTP_clock","CTP_counter","CTP_lumiblockrange","CTP_lumiblocktime","CTP_multpt","CTP_nanosectime","CTP_prescale_error","CTP_roiCand","CTP_roiNum","CTP_wrong_BGK","CTP_CTPIN_MU","CTP_CTPIN_JET2","TOPO_inputs","TOPO_outputs","TOPO_misconf_calib","TOPO_misconf_electronics","TOPO_off","TOPO_readout","CAL_LOWMUCONFIG_IN_HIGHMU"] # Trig_L1 system
    defects0["globTol"] = ["CAL_coverage_tolerable","CAL_misconf_calib_tolerable","CAL_misconf_electronics_tolerable","CAL_misconf_satBCID_tolerable","CAL_misconf_tile_drawer","CAL_readout_cpjep_tolerable","CAL_readout_pp_tolerable","CAL_mistimed_larTBB_SEU_tolerable","CTP_NONSTANDARD_CONFIG","MUB_lost_sync_tolerable","MUB_failed_electronics_tolerable","MUB_coverage_tolerable","MUB_misconf_electronics_tolerable","MUB_LOWER_EFFICIENCY_TOLERABLE","MUE_coverage_tolerable","MUE_pt15GeV_tolerable","MUE_pt20GeV_tolerable","MUE_FakeBurst","TOPO_inputs_tolerable","TOPO_outputs_tolerable","TOPO_misconf_calib_tolerable","TOPO_misconf_electronics_tolerable","TOPO_readout_tolerable","TOPO_readout_roib","TOPO_not_good_for_physics"] # Trig_L1 system

    veto["all"] = [] # Veto name as defined in the COOL database
    veto["COOL"] = {} # Veto name as defined in the COOL database

    defectVeto["description"] = {"CAL_coverage":"",
                                 "CAL_misconf_calib":"[Calo] Electronics miscalibration",
                                 "CAL_misconf_electronics":"[Calo] Electronics misconfiguration",
                                 "CAL_misconf_satBCID":"",
                                 "CAL_off":"[Calo] L1Calo off",
                                 "CAL_mistimed_larTBB_SEU_tolerable":"[LAr] TBB SEU leading to mistiming",
                                 "CAL_LOWMUCONFIG_IN_HIGHMU":"[Calo] Low-mu configuration in high-mu run",
                                 "MUB_busy":"[Muon] Busy",
                                 "MUB_coverage":"[Muon] Barrel coverage",
                                 "MUB_failed_electronics":"[Muon] Electronics failure",
                                 "MUB_lost_sync":"[Muon] Lost synchr.",
                                 "MUB_misconf_electronics":"[Muon] Electronics misconfig",
                                 "MUE_busy":"[Muon] Busy",
                                 "MUE_coverage":"[Muon] Forward coverage",
                                 "MUE_misconf_electronics":"[Muon] Electronics misconfig",
                                 "MUE_pt15GeV":"[Muon] Pt15GeV",
                                 "MUE_pt20GeV":"[Muon] Pt20GeV",
                                 "CTP_wrong_BGK":"[CTP] Wrong bunch group key",
                                 "CTP_CTPIN_MU":"[CTPIN] Cable problems for muons",
                                 "CTP_CTPIN_JET2":"[CTPIN] Cable problems for jets",
                                 "TOPO_misconf_electronics":"[CaloTopo] Electronics misconfiguration"}

    signOff["EXPR."] = ["TRIG_L1_CAL_UNCHECKED","TRIG_L1_CTP_UNCHECKED","TRIG_L1_MUB_UNCHECKED","TRIG_L1_MUE_UNCHECKED","TRIG_L1_TOPO_UNCHECKED"]
    signOff["BULK"] = []
    signOff["FINAL"] = []

#################################### Trig_HLT defects
  if system == "Trig_HLT":
    partitions["color"] = {}
    partitions["list"] = list(partitions["color"].keys())

    defects0["prefix"] = ["TRIG_HLT"]
    # Partition intolerable and tolerable defects - Order determines what defect is proeminent
    defects0["partIntol"] = []
    defects0["partTol"] = []
    # Global intolerable and tolerable defects
    # Warning : do not remove/edit the comment specifying the system. It is used to display the defects in the webpage
    defects0["globIntol"] = ["BJT_beam_spot_flag","BJT_no_secvtx","BJT_no_tracking","BJT_INACCURATE_ONLINE_BEAMSPOT","BJT_NO_MULTIBJET","BPH_no_muon","BPH_no_tracking","CAL_LAR_SourceMajor","CAL_TILE_SourceMajor","CAL_missing_data","CAL_no_primaries","ELE_no_clustering","ELE_no_tracking","ELE_primary_chain_misconfigured","ELE_unknown","ELE_tracking_issue","GAM_no_clustering","GAM_partial_clustering","GAM_primary_chain_misconfigured","GAM_unknown","GENERAL_debugstream","GENERAL_no_primaries","GENERAL_prescale_problem","GENERAL_standby","GENERAL_xpu_misconf","IDT_EF_FAIL","IDT_IDS_FAIL","IDT_SIT_FAIL","IDT_PRIVX_INEFF","JET_algo_problem","JET_menu_misconf","JET_unknown","MBI_no_tracking","MET_missing_data","MUO_Upstream_Barrel_problem","MUO_Upstream_Endcap_problem","TAU_misconf","TAU_caloIssue","TAU_nocalo","TAU_no_tracking","TRG_HLT_TAU_tracking_issue","TAU_dbIssue_BeamSpot","IDT_PIX_DATA_ERROR","MUO_chain_disabled"] # Trig_HLT system
    defects0["globTol"] = ["BJT_partial_tracking","BJT_unknown","BJT_ONLINE_BEAMSPOT_GT1p6MM","BJT_ONLINE_BEAMSPOT_GT2MM","BPH_algcrash","BPH_misconf","BPH_partial_muon","BPH_partial_tracking","BPH_unknown","CAL_LAR_SourceMinor","CAL_ROI_EXCESS","CAL_TILE_SourceMinor","CAL_partial_missing_data","CAL_spike","CAL_incorrect_BCID_correction","CAL_unknown","ELE_chain_misconfigured","ELE_clustering_issue","ELE_lowEfficiency_all_electrons","ELE_non_primary_poor_performance_e15_HLTtighter","ELE_non_primary_poor_performance_e15_tight","ELE_nonprimary_misconfigured","ELE_partial_clustering","ELE_partial_tracking","ELE_primary_poor_performance_e20_medium1","ELE_primary_poor_performance_e22_medium1","ELE_tracking_issue_Tolerable","GAM_chain_misconfigured","GAM_clustering_issue","GAM_nonprimary_misconfigured","GENERAL_streaming","GENERAL_tolerableDebugstream","GENERAL_no_1e34_primaries","GENERAL_no_12e33_primaries","GENERAL_no_15e33_primaries","GENERAL_no_17e33_primaries","IDT_BSPOT_FAILUR","IDT_BSPOT_INVALID_STATUS","IDT_BSPOT_INVALIDATOR_PROBLEM","IDT_EFT_FAIL","IDT_LOWSTAT","IDT_SCT_OUTOFTIMEHITS","IDT_TRT_DATA_LOST","IDT_TRT_OUTOFTIMEHITS","IDT_TSF_FAIL","IDT_unknown","JET_calib_issue","JET_energy_excess","JET_GSC_BEAMSPOT_PROBLEM","JET_hotspot","JET_partialscan_issue","MBI_HI_time_shift_mbts","MBI_partial_tracking","MBI_unknown","MBI_spacepoint_noise","MET_XS_Triggers_OFF","MET_missingEt_spike","MET_partial_missing_data","MET_phi_spike","MET_sumEt_spike","MET_unknown","MUO_EFMSonly_problem","MUO_Fullscan_problem","MUO_L2Iso_problem","MUO_L2muonSA_problem","MUO_MSonly_Barrel_problem","MUO_MSonly_Endcapl_problem","MUO_MuComb_problem","MUO_MuGirl_problem","MUO_Multi_Muon_problemchains","MUO_MuonEFTrackIso_problem","MUO_MuonEF_problem","MUO_Slow_problem","MUO_unknown","MUO_chain_misconfigured","TAU_unknown","TAU_dbIssue_mu","TAU_tracking_issue_Tolerable"] # Trig_HLT system
    
    veto["all"] = [] # Veto name as defined in the COOL database
    veto["COOL"] = {} # Veto name as defined in the COOL database

    defectVeto["description"] = {"CAL_LAR_SourceMajor":"[LAr] Major problem",
                                 "CAL_no_primaries":"[Calo] No primaries",
                                 "GENERAL_no_primaries":"No primaries",
                                 "GENERAL_prescale_problem":"Prescale problem",
                                 "MUO_Upstream_Barrel_problem":"[Muon] Upstream barrel problem",
                                 "BJT_INACCURATE_ONLINE_BEAMSPOT":"Inaccurate online beam spot",
                                 "BJT_ONLINE_BEAMSPOT_GT1p6MM":"Online beam spot > 1.6mm",
                                 "BJT_ONLINE_BEAMSPOT_GT2MM":"Online beam spot > 2mm",
                                 "BJT_beam_spot_flag":"Beam spot flag",
                                 "BJT_NO_MULTIBJET":"No multi b-jets",
                                 "GENERAL_debugstream":"Problem in debug stream",
                                 "BJT_no_secvtx":"[b-jet] No Secondary vertex",
                                 "IDT PIX DATA ERROR":""
                                 }

    signOff["EXPR."] = ["TRIG_HLT_BJT_UNCHECKED","TRIG_HLT_BPH_UNCHECKED","TRIG_HLT_CAL_UNCHECKED","TRIG_HLT_ELE_UNCHECKED","TRIG_HLT_GAM_UNCHECKED","TRIG_HLT_IDT_UNCHECKED","TRIG_HLT_JET_UNCHECKED","TRIG_HLT_MBI_UNCHECKED","TRIG_HLT_MET_UNCHECKED","TRIG_HLT_MUO_UNCHECKED","TRIG_HLT_TAU_UNCHECKED"]
    signOff["BULK"] = []
    signOff["FINAL"] = []

################################# LUMI defects
# Source : private email by Davide Caforio
  if system == "Lumi":
    partitions["color"] = { } # No partition needed
                            
    partitions["list"] = list(partitions["color"].keys())

    defects0["prefix"] = ["LUMI"]
    # Partition intolerable and tolerable defects - Order determines what defect is proeminent
    defects0["partIntol"] = [] # No partition defect                             
    defects0["partTol"] = [] 
    # Global intolerable and tolerable defects
    defects0["globIntol"] = ["BEAMS_MOVING","CALIB_UNCERTAIN","CTP_ERROR","EMITTANCESCAN","INFR_ERROR","LCD_MISSINGCURRENT","OLC2COOL_FAILURE","ONLINEDB_ERROR","ONL_DET_ERROR_SEVERE","ONL_OLC2HLT_SEVERE","ONL_ONLINEDB_ERROR","PERBCIDINFO_ABSENT","UNKNOWN","VDM"] # Lumi system
    defects0["globTol"] = [] 


    defectVeto["description"] = {"BEAMS_MOVING":"Beams moving",
                                 "CALIB_UNCERTAIN":"Uncertain calibration",
                                 "CTP_ERROR":"CTP error",
                                 "EMITTANCESCAN":"Emittance scan",
                                 "INFR_ERROR":"Infr. error",
                                 "LCD_MISSINGCURRENT":"Missing LCD current",
                                 "OLC2COOL_FAILURE":"OLC2COOL failure",
                                 "ONLINEDB_ERROR":"Online DB error",
                                 "ONL_DET_ERROR_SEVERE":"Detector error",
                                 "ONL_OLC2HLT_SEVERE":"OLC2HLT error",
                                 "ONL_ONLINEDB_ERROR":"Online DB error",
                                 "PERBCIDINFO_ABSENT":"Missing per BCID info",
                                 "UNKNOWN":"Unknown",
                                 "VDM":"VDM scan"}

    veto["all"] = [] # Veto name as defined in the COOL database
    veto["COOL"] = {} # Veto name as defined in the COOL database

    signOff["EXPR."] = ['LUMI_UNCHECKED']
    signOff["BULK"] = []
    signOff["FINAL"] = []

################################# LUCID defects
  if system == "LUCID":
    partitions["color"] = {}
    partitions["list"] = list(partitions["color"].keys())

    defects0["prefix"] = ["LCD"]
    # Partition intolerable and tolerable defects - Order determines what defect is proeminent
    defects0["partIntol"] = []
    defects0["partTol"] = []
    # Global intolerable and tolerable defects
    defects0["globIntol"] = ["DAQ_ERROR","DISABLED","GAS_CHANGING","HV_CHANGE","HV_RESET","LUMAT_ERROR","MAGNETS_RAMPING","MDA_FAIL"] 
    defects0["globTol"] = ["LCD_HV_OFF","LCD_LEDBOARD_ERROR","LCD_LV_OFF"] 
    
    veto["all"] = [] # Veto name as defined in the COOL database
    veto["COOL"] = {} # Veto name as defined in the COOL database

    defectVeto["description"] = {"DAQ_ERROR":"DAQ error",
                                 "DISABLED":"Disabled",
                                 "GAS_CHANGING":"Gas changing",
                                 "HV_CHANGE":"HV change",
                                 "HV_RESET":"HV reset",
                                 "LUMAT_ERROR":"Lumat error",
                                 "MAGNETS_RAMPING":"Ramping magnets",
                                 "MDA_FAIL":"MDA fail"}

    signOff["EXPR."] = ["LCD_UNCHECKED"]
    signOff["BULK"] = []
    signOff["FINAL"] = []

################################# ALFA defects
  if system == "ALFA":
    partitions["color"] = {}
    partitions["list"] = list(partitions["color"].keys())

    defects0["prefix"] = ["ALFA"]
    # Partition intolerable and tolerable defects - Order determines what defect is proeminent
    defects0["partIntol"] = []
    defects0["partTol"] = []
    # Global intolerable and tolerable defects
    # Warning : do not remove/edit the comment specifying the system. It is used to display the defects in the webpage
    defects0["globIntol"] = ["MISSING_TRIGGER","WRONG_POSITION","MISSING_PMF_5orMore"] # ALFA system
    defects0["globTol"] = ["MISSING_PMF_1to4"] # ALFA system
    
    veto["all"] = [] # Veto name as defined in the COOL database
    veto["COOL"] = {} # Veto name as defined in the COOL database

    defectVeto["description"] = {"MISSING_TRIGGER":"Missing trigger",
                                 "WRONG_POSITION":"Wrong position",
                                 "MISSING_PMF_5orMore":"Missing PMF"}

    signOff["EXPR."] = []
    signOff["BULK"] = []
    signOff["FINAL"] = []

#################################### AFP defects
  if system == "AFP":
    partitions["color"] = {'C':kYellow-9,'A':kYellow,'C_FAR':kOrange,'C_NEAR':kOrange-3,'A_FAR':kRed-3,'A_NEAR':kBlue-3}
    partitions["list"] = partitions["color"].keys()

    defects0["prefix"] = ["AFP"]
    # Partition intolerable and tolerable defects - Order determines what defect is proeminent
    # Warning : do not remove/edit the comment specifying the system. It is used to display the defects in the webpage
    defects0["partIntol"] = ["TOF_NOT_OPERATIONAL_HV","TOF_NOT_OPERATIONAL_LV","TOF_WRONG_CABLING","TOF_WRONG_TIMING","TOF_TRIGGER_PROBLEM","SIT_NOT_OPERATIONAL_HV","SIT_NOT_OPERATIONAL_LV","SIT_NOT_OPERATIONAL","SIT_WRONG_TIMING","IN_GARAGE"] # AFP system
    defects0["partTol"] = [] # AFP system
    # Global intolerable and tolerable defects
    # Warning : do not remove/edit the comment specifying the system. It is used to display the defects in the webpage
    defects0["globIntol"] = ["WRONG_MAPPING","ERROR"]  # AFP system
    defects0["globTol"] = []  # AFP system
    
    veto["all"] = [] # Veto name as defined in the COOL database
    veto["COOL"] = {} # Veto name as defined in the COOL database

    defectVeto["description"] = {"TOF_NOT_OPERATIONAL_HV":"[ToF] Non-nominal HV",
                                 "TOF_NOT_OPERATIONAL_LV":"[ToF] Non-operational LV",
                                 "TOF_WRONG_CABLING":"[ToF] Wrong cabling",
                                 "TOF_WRONG_TIMING":"[ToF] Wrong timing",
                                 "TOF_TRIGGER_PROBLEM":"[ToF] Trigger problem",
                                 "SIT_NOT_OPERATIONAL_HV":"[SiT] < 3 planes at nominal HV",
                                 "SIT_NOT_OPERATIONAL_LV":"[SiT] < 3 planes available (LV)",
                                 "SIT_NOT_OPERATIONAL":"[SiT] < 3 SiT planes available",
                                 "SIT_WRONG_TIMING":"[SiT] Wrong timing",
                                 "IN_GARAGE":">= 1 RP in the garage or moving",
                                 "WRONG_MAPPING":"[ToF] Wrong mapping",
                                 "ERROR":"Unknown problem"}

    signOff["EXPR."] = ["AFP_UNCHECKED"]
    signOff["BULK"] = ["AFP_BULK_UNCHECKED"]
    signOff["FINAL"] = []


################################# ZDC defects
  if system == "ZDC":
    partitions["color"] = {}
    partitions["list"] = list(partitions["color"].keys())

    defects0["prefix"] = ["ALPHA"]
    # Partition intolerable and tolerable defects - Order determines what defect is proeminent
    defects0["partIntol"] = []
    defects0["partTol"] = []
    # Global intolerable and tolerable defects
    # Warning : do not remove/edit the comment specifying the system. It is used to display the defects in the webpage
    defects0["globIntol"] = ["SEVUNKNOWN"] # ZDC system
    defects0["globTol"] = ["ZDC_NONOPTIMALHV","ZDC_NONOPTIMALTIMING","ZDC_PERBCIDINFO_ABSENT","ZDC_UNKNOWN"] # ZDC system
    
    veto["all"] = [] # Veto name as defined in the COOL database
    veto["COOL"] = {} # Veto name as defined in the COOL database

    defectVeto["description"] = {}

    signOff["EXPR."] = []
    signOff["BULK"] = []
    signOff["FINAL"] = []

#################################### GLOBAL defects
  if system == "Global":
    partitions["color"] = {}
    partitions["list"] = list(partitions["color"].keys())

    defects0["prefix"] = ["GLOBAL"]
    # Partition intolerable and tolerable defects - Order determines what defect is proeminent
    defects0["partIntol"] = []
    defects0["partTol"] = []
    # Global intolerable and tolerable defects
    # Warning : do not remove/edit the comment specifying the system. It is used to display the defects in the webpage
    defects0["globIntol"] = ["BUSY_GREATER50PCT","LOWMUCONFIG_IN_HIGHMU","OFFLINE_PROCESSING_PROBLEM","SOLENOID_OFF","SOLENOID_RAMPING","SOLENOID_NOTNOMINAL","TOROID_OFF","TOROID_NOTNOMINAL","TOROID_RAMPING",                                 "NOTCONSIDERED","LHC_HIGHBETA","NONSTANDARD_BC","LOW_N_BUNCHES"] # Global system
    defects0["globTol"] = [] # Global system
    
    veto["all"] = [] # Veto name as defined in the COOL database
    veto["COOL"] = {} # Veto name as defined in the COOL database

    defectVeto["description"] = {"BUSY_GREATER50PCT":"Busy",
                                 "LOWMUCONFIG_IN_HIGHMU":"Low mu config in high mu",
                                 "OFFLINE_PROCESSING_PROBLEM":"Offline processing problem",
                                 "SOLENOID_OFF":"Solenoid off",
                                 "SOLENOID_RAMPING":"Solenoid ramping",
                                 "SOLENOID_NOTNOMINAL":"Solenoid not nominal",
                                 "TOROID_OFF":"Toroid off",
                                 "TOROID_NOTNOMINAL":"Toroid not nominal",
                                 "TOROID_RAMPING":"Toroid ramping",
                                 "NOTCONSIDERED":"Not considered",
                                 "LHC_HIGHBETA":"LHC: high beta*",
                                 "NONSTANDARD_BC":"LHC: non standard BC",
                                 "LOW_N_BUNCHES":"LHC: low nbunches"
                                 }

    signOff["EXPR."] = []
    signOff["BULK"] = []
    signOff["FINAL"] = []

#########################################################################################
################ Definitions common to all systems
  # 
  defects0["globalFilterDefects"] = ["GLOBAL_LHC_50NS","GLOBAL_LHC_NONSTANDARD_BC","GLOBAL_LHC_LOW_N_BUNCHES","GLOBAL_LHC_NOBUNCHTRAIN","GLOBAL_LHC_COMMISSIONING","GLOBAL_LHC_HIGHBETA","GLOBAL_LOWMUCONFIG_IN_HIGHMU","LUMI_VDM","GLOBAL_NOTCONSIDERED"]
  if system == "Global": # In the case of the Global system, do not apply global filtering as they may be common intolerable defects. 
     for iDef in defects0["globalFilterDefects"]:
        if iDef not in defects0["globIntol"]:
           defects0["globIntol"].append(iDef)
     defects0["globalFilterDefects"] = []

  defects0["part"] = defects0["partIntol"] + defects0["partTol"]
  defects0["glob"] = defects0["globIntol"] + defects0["globTol"]

  defects0["intol"] = defects0["globIntol"] + defects0["partIntol"]
  defects0["tol"] = defects0["globTol"] + defects0["partTol"]

  defects0["partIntol_recov"] = []
  defects0["globIntol_recov"] = []
  for idef in defects0["partIntol"]: # Create a duplicated list of intol defect to monitor the recoverability
    defects0["partIntol_recov"].append("%s__recov"%idef)
  for idef in defects0["globIntol"]: # Create a duplicated list of intol defect to monitor the recoverability
    defects0["globIntol_recov"].append("%s__recov"%idef)
  defects0["intol_recov"] = defects0["partIntol_recov"] + defects0["globIntol_recov"]

# If the description is not available, define it with the defect name
  for iDef in defects0["intol"]+defects0["tol"]:
    if iDef not in list(defectVeto["description"].keys()):
      defectVeto["description"][iDef] = iDef

# New dynamic way to define the color for the defect affecting the year / tag
  defectVeto["color"] = {}
  iColor=0
  for iDefectVeto in list(defectVeto["description"].keys()):
     # If more than 12 defects affecting a system, add new color in vector below - https://root.cern.ch/doc/master/pict1_TColor_002.png 
     colorPalette = [kBlue+3,kRed+1,kGreen-2,kOrange,kBlue-4,kMagenta+2,kYellow+1,kSpring-9,kAzure+6,kViolet-4,kPink,kBlue-9,kPink-6]
     if (os.path.exists("YearStats-%s/%s/%s/loss-%s.dat"%(system,year,tag,iDefectVeto))) and iColor<len(colorPalette):
        defectVeto["color"][iDefectVeto] = colorPalette[iColor]
        iColor = iColor + 1
     else:
        defectVeto["color"][iDefectVeto] = kBlack

  if (system == "LAr"):
     defectVeto["color"]["noiseBurst"] = kBlue-2
     defectVeto["color"]["miniNoiseBurst"] = kMagenta+1
     defectVeto["color"]["corruption"] = kAzure+9


# The definition below are there only to avoid crashed, as these summed histogram are not (yet?)  displayed 
  defectVeto["description"]["allIntol"] = "Dummy"
  defectVeto["description"]["allIntol_recov"] = "Dummy"
  defectVeto["color"]["allIntol"] = kBlack
  defectVeto["color"]["allIntol_recov"] = kBlack
  defectVeto["description"]["allVeto"] = "Dummy"
  defectVeto["color"]["allVeto"] = kBlack

  return True
