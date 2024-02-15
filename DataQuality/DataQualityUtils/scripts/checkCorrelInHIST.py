#!/usr/bin env python
# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
# Script to browse the unmerged HIST files and correlate the number of entries in a region defined by (x;y,delta) arguments
# Uses the pathExtract library to extract the EOS path
# See the twiki: https://twiki.cern.ch/twiki/bin/viewauth/Atlas/UsingDQInframerge
# Author : Benjamin Trocme (LPSC Grenoble) / 2017

import os, sys
import argparse
import xmlrpc.client
from DataQualityUtils import pathExtract         
import ctypes
import sqlite3


import ROOT as R

R.gStyle.SetPalette(1)
R.gStyle.SetOptStat("em")

nLB = 8000

dqmpassfile="/afs/cern.ch/user/l/larmon/public/atlasdqmpass.txt"

conn = None
cursor = None

# Some definitions for wildcards which can be used for input plot lists
wildcards = {}
wildcardplots = ["CellOccupancyVsEtaPhi", "fractionOverQthVsEtaPhi","DatabaseNoiseVsEtaPhi","CellAvgEnergyVsEtaPhi"]

for plot in wildcardplots:
  wildcards[plot] = {}
  wildcards[plot]["EMB"] = { "P":"Presampler","1":"Sampling1","2":"Sampling2","3":"Sampling3"}
  wildcards[plot]["EMEC"] = { "P":"Presampler","1":"Sampling1","2":"Sampling2","3":"Sampling3"}
  wildcards[plot]["HEC"] = { "0":"Sampling0","1":"Sampling1","2":"Sampling2","3":"Sampling3"}
  wildcards[plot]["FCAL"] = { "1":"Sampling1","2":"Sampling2","3":"Sampling3"}

# e.g. Tile/Cell/AnyPhysTrig/TileCellEneEtaPhi_SampB_AnyPhysTrig
# e.g. Tile/Cell/AnyPhysTrig/TileCellEtaPhiOvThr_SampB_AnyPhysTrig
wildcards["TileCellEneEtaPhi"] = [ "A", "B", "D", "E" ]
wildcards["TileCellEtaPhiOvThr"] = [ "A", "B", "D", "E" ]

def expandWildCard(histlist):
  newhistlist = []
  grouped = {}  # document the grouped plots, so we can show in one canvas
  for hist in histlist:
    if "*" in hist:      
      foundwc = False
      for wc in wildcards.keys():        
        if wc in hist:
          foundwc = True
          newpaths = []
          if "Tile" in wc:
            for samp in wildcards[wc]:
              tmp_path = hist
              new_path = tmp_path.replace("*",samp)
              newpaths.append(new_path)
          else:
            for part in wildcards[wc].keys():
              tmp_path = hist
              if part+"*" in tmp_path:
                for samp in wildcards[wc][part].keys():

                  new_path = tmp_path.replace(part+"*", part+samp)

                  if "*" in new_path:
                    new_path = new_path.replace("*", wildcards[wc][part][samp])
                  newpaths.append(new_path)

          if len(newpaths) == 0: 
            print("Failed to get the full paths from the wildcard...")
            sys.exit()

          print("Expanded",wc,"wildcard to give",len(newpaths),"histograms")
          newhistlist.extend(newpaths)      
      if foundwc is False:
        print("A wildcard has been used, but the requested histogram is not yet defined in this script. See the wildcards dictionary:",wildcards.keys())
        sys.exit()
      grouped[hist] = newpaths
    else:
      newhistlist.append(hist)
  return newhistlist, grouped


def convHname(hname):
  DET = { "EMB":"0", "EMEC":"1","HEC":"2","FCAL":"3" }
  AC = { "A":"1", "C":"-1" }
  SAM = { "P":"0" }
  for samp in range(0,4): SAM[str(samp)] = str(samp)
  for det in DET.keys():
    for sam in SAM.keys():
      for ac in AC.keys():  
        if det+sam+ac in hname:
          return DET[det], AC[ac], SAM[sam]
  return None, None, None

def LIDProposal(hname, eta, phi, delta=0.15):
  """ Print a proposed LArID translator (https://atlas-larmon.cern.ch/LArIdtranslator/) SQL query """
  det, ac, sam = convHname(hname)
  if det is None: return
  if int(det) == 0 and abs(eta) > 1.4 and int(sam)>1:
    sam = str(int(sam)-1)
  proposal = "DET="+det+" and AC="+ac+" and SAM="+sam+" and ETA between "+str(eta-delta)+" and "+str(eta+delta)+" and PHI between "+str(phi-delta)+" and "+str(phi+delta)
  print("*"*30)
  print("Proposal query for LArID translator for plot",hname,"is as follows:")
  print(proposal)


def setupDqmAPI():
  """ Connect to the atlasDQM web API service: https://twiki.cern.ch/twiki/bin/viewauth/Atlas/DQWebServiceAPIs """
  if (not os.path.isfile(dqmpassfile)):
    print("To connect to the DQ web service APIs, you need to generate an atlasdqm key and store it in the specified location ("+dqmpassfile+"). The contents should be yourname:key")
    print("To generate a key, go here : https://atlasdqm.cern.ch/dqauth/")
    sys.exit()
  passfile = open(dqmpassfile)
  passwd = passfile.read().strip(); passfile.close()
  passurl = 'https://%s@atlasdqm.cern.ch'%passwd
  s = xmlrpc.client.ServerProxy(passurl)
  return s

def lbStr(lb):
  """ Return the lb number in string format, e.g. _lb0001 """
  return "_lb"+str(lb).zfill(4)

def hType(hist, verbose=False):
  """ Return type of the provided hist as a string, or None if it's not a hist """
  if isinstance(hist,R.TH2):
    return "2D"
  elif isinstance(hist,R.TH1):
    return "1D"
  else:
    if verbose:
      print("The input hist is not TH1/TH2... it is",type(hist),"- returning None")
    return None


def checkCorrel(histos, listLB, checkList):
  # Dump the correlations in histograms to be displayed
  cRatio = {}
  paveCorrel = {}

  correls = {}
  
  fractionNonZero = 0
  for iPath in histos.keys():
    for iPath2 in histos.keys():
      corr = "%s_%s"%(iPath,iPath2)
      corr2 = "%s_%s"%(iPath2,iPath)
      if not any([ cl in iPath for cl in checkList]) and not any([cl in iPath2 for cl in checkList]):
        print("Skipping comparison of",iPath,"vs",iPath2)
        if corr in correls.keys(): del correls[corr]
        if corr2 in correls.keys(): del correls[corr2]
        continue
      print("Checking correl for",iPath,"vs",iPath2)
      if corr not in correls.keys(): correls[corr] = {}
      if corr2 not in correls.keys(): correls[corr2] = {}
      if (iPath != iPath2 and "hCorrel" not in correls[corr2].keys()): # Correlation plots
        print("====== I am checking correlation between %s and %s"%(iPath.split("/")[-1],iPath2.split("/")[-1]))
        correls[corr]["hCorrel"] = R.TH2D( "Correlation_%s"%corr,"Correlation_%s"%corr,
                                50, min(histos[iPath]["nbHitInHot"])-1, max(histos[iPath]["nbHitInHot"])+1,
                                50, min(histos[iPath2]["nbHitInHot"])-1, max(histos[iPath2]["nbHitInHot"])+1 )
        correls[corr]["hCorrel"].SetXTitle(iPath.split("/")[-1])
        correls[corr]["hCorrel"].SetYTitle(iPath2.split("/")[-1])      

        correls[corr]["nbHitRatio"] = [-999.]*nLB
        correls[corr2]["nbHitRatio"] = [-999.]*nLB
        for iLB in listLB:
          if (histos[iPath2]["nbHitInHot"][iLB] !=0):
            correls[corr]["nbHitRatio"][iLB] = histos[iPath]["nbHitInHot"][iLB]/histos[iPath2]["nbHitInHot"][iLB]
          if (histos[iPath]["nbHitInHot"][iLB] !=0):
            correls[corr2]["nbHitRatio"][iLB] = histos[iPath2]["nbHitInHot"][iLB]/histos[iPath]["nbHitInHot"][iLB]

        correls[corr]["hRatio"] = R.TH1D("Ratio_%s"%corr,"Ratio_%s"%corr,100,0.,max(correls[corr]["nbHitRatio"])+1)
        correls[corr]["hRatio"].SetXTitle("%s/%s"%(iPath.split("/")[-1],iPath2.split("/")[-1]))
        correls[corr]["hRatio"].SetMarkerColor(R.kBlue+2)
        correls[corr]["hRatio"].SetMarkerStyle(20)
        correls[corr2]["hRatio"] = R.TH1D("Ratio_%s"%corr2,"Ratio_%s"%corr2,100,0.,max(correls[corr2]["nbHitRatio"])+1)
        correls[corr2]["hRatio"].SetXTitle("%s/%s"%(iPath2.split("/")[-1],iPath.split("/")[-1]))
        correls[corr2]["hRatio"].SetMarkerColor(R.kBlue+2)
        correls[corr2]["hRatio"].SetMarkerStyle(20)

        for iLB in listLB:
          if (histos[iPath]["nbHitInHot"][iLB] !=0 or histos[iPath2]["nbHitInHot"][iLB] != 0.):
            correls[corr]["hCorrel"].Fill(histos[iPath]["nbHitInHot"][iLB],histos[iPath2]["nbHitInHot"][iLB])
            print("LB: %d -> %.2f / %.2f"%(iLB,histos[iPath]["nbHitInHot"][iLB],histos[iPath2]["nbHitInHot"][iLB]))
          if correls[corr]["nbHitRatio"][iLB]!= -999:
            correls[corr]["hRatio"].Fill(correls[corr]["nbHitRatio"][iLB])
          if correls[corr2]["nbHitRatio"][iLB]!= -999:
            correls[corr2]["hRatio"].Fill(correls[corr2]["nbHitRatio"][iLB])

        correls[corr]["cCorrel"] = R.TCanvas("Correl-%s"%corr,"Correl-%s"%corr)
        correls[corr]["hCorrel"].Draw("COLZ")     
        paveCorrel[corr] = R.TPaveText(.1,.72,.9,.9,"NDC")
        paveCorrel[corr].SetFillColor(R.kBlue-10)
        paveCorrel[corr].AddText("Run %d / %d LBs in total - %d LBs with >=1 entry in either plot"%(args.runNumber,len(listLB),correls[corr]["hCorrel"].GetEntries()))
        paveCorrel[corr].AddText("Correlation factor:%.3f"%(correls[corr]["hCorrel"].GetCorrelationFactor()))

        try:
          fractionNonZero = correls[corr]["hRatio"].Integral(2,100)/correls[corr]["hRatio"].Integral(1,100)
        except ZeroDivisionError: 
          fractionNonZero = 0
        if fractionNonZero != 0.:
          meanNonZero = correls[corr]["hRatio"].GetMean()/fractionNonZero
        else:
          meanNonZero = 0.
        paveCorrel[corr].AddText("When >=1 entry in X plot(%d LBs), %.0f %% events with >=1 entry in Y plot(<ratio>=%.2f)"%(correls[corr]["hRatio"].Integral(1,100),fractionNonZero*100.,meanNonZero))      
        try:
          fractionNonZero = correls[corr2]["hRatio"].Integral(2,100)/correls[corr2]["hRatio"].Integral(1,100)
        except ZeroDivisionError:
          fractionNonZero = 0
        if fractionNonZero != 0.:
          meanNonZero = correls[corr2]["hRatio"].GetMean()/fractionNonZero
        else:
          meanNonZero = 0.
        paveCorrel[corr].AddText("When >=1 entry in Y plot(%d LBs), %.0f %% events with >=1 entry in X plot(<ratio>=%.2f)"%(correls[corr2]["hRatio"].Integral(1,100),fractionNonZero*100.,meanNonZero))
        paveCorrel[corr].Draw()

        if args.draw1D:
          correls[corr]["cRatio"] = R.TCanvas("Ratio-%s"%corr,"Ratio-%s"%corr)
          correls[corr]["hRatio"].Draw("P HIST")     
          correls[corr2]["cRatio"] = R.TCanvas("Ratio-%s"%corr2,"Ratio-%s"%corr2)
          correls[corr2]["hRatio"].Draw("P HIST")     

        correls[corr]["cCorrel"].Update() # make sure all of the text is there

      elif ("hEvol" not in histos[iPath].keys()): # Evolution of nb of hit per LB
        histos[iPath]["hEvol"] = R.TH1D("Evolution_%s"%iPath,"%s"%(iPath.split("/")[-1]),max(listLB)-min(listLB),min(listLB),max(listLB))
        histos[iPath]["hEvol"].SetXTitle("Luminosity block")
        histos[iPath]["hEvol"].SetYTitle("Nb of hits")
        histos[iPath]["hEvol"].SetMarkerColor(R.kGreen+2)
        histos[iPath]["hEvol"].SetMarkerStyle(20)

        for iLB in listLB:
          histos[iPath]["hEvol"].Fill(iLB,histos[iPath]["nbHitInHot"][iLB])

        if args.draw1D:
          histos[iPath]["cEvol"] = R.TCanvas("LB evol - %s"%iPath)
          histos[iPath]["hEvol"].Draw("P HIST")
      
  return correls, fractionNonZero

def getSummary(histos, correls, fractionNonZero):
  print("====== Summary data")
  already = []
  for iPath in histos.keys():
    for iPath2 in histos.keys():
      corr = "%s_%s"%(iPath,iPath2)
      corr2 = "%s_%s"%(iPath2,iPath)
      if corr not in correls.keys() and corr2 not in correls.keys():
        continue
      if (iPath != iPath2 and corr2 not in already): # Correlation plots
        print("====== %s vs %s"%(iPath.split("/")[-1],iPath2.split("/")[-1]))
        print("Correlation factor: %.3f"%(correls[corr]["hCorrel"].GetCorrelationFactor()))
        try:
          fractionNonZero = correls[corr]["hRatio"].Integral(2,100)/correls[corr]["hRatio"].Integral(1,100)
        except ZeroDivisionError:
          fractionNonZero = 0
        if fractionNonZero != 0.:
          meanNonZero = correls[corr]["hRatio"].GetMean()/fractionNonZero
        else:
          meanNonZero = 0.
        print("When there is at least one entry in %s (%d LBs), there are %.1f %% of events with an entry in %s - Mean ratio: %.2f"%(iPath2.split("/")[-1],correls[corr]["hRatio"].Integral(1,100),fractionNonZero*100.,iPath.split("/")[-1],meanNonZero))

        try:
          fractionNonZero = correls[corr2]["hRatio"].Integral(2,100)/correls[corr2]["hRatio"].Integral(1,100)
        except ZeroDivisionError:
          fractionNonZero = 0
        if fractionNonZero != 0.:
          meanNonZero = correls[corr2]["hRatio"].GetMean()/fractionNonZero
        else:
          meanNonZero = 0.
        print("When there is at least one entry in %s (%d LBs), there are %.1f %% of events with an entry in %s - Mean ratio: %.2f"%(iPath.split("/")[-1],correls[corr2]["hRatio"].Integral(1,100),fractionNonZero*100.,iPath2.split("/")[-1],meanNonZero))

        already.append(corr)


def topNBins(histname,h,topn,bins,h_fracQth=None):
  content = []
  binx = []
  biny = []
  fracQthContent = []
  for nb in bins:
    x, y, z = ctypes.c_int(1), ctypes.c_int(1), ctypes.c_int(1)
    h.GetBinXYZ(nb, x, y, z)
    c = h.GetBinContent(nb)
    xcent = h.GetXaxis().GetBinCenter(x.value)
    xlow = h.GetXaxis().GetBinLowEdge(x.value)
    xhi = h.GetXaxis().GetBinUpEdge(x.value)
    ycent = h.GetYaxis().GetBinCenter(y.value)
    ylow = h.GetYaxis().GetBinLowEdge(y.value)
    yhi = h.GetYaxis().GetBinUpEdge(y.value)
    content.append(c)
    if h_fracQth is not None and isinstance(h_fracQth, R.TH1):
      fracQthContent.append(h_fracQth.GetBinContent(nb))
    else:
      fracQthContent.append(0)
    binx.append( [xlow,xcent,xhi] )
    biny.append( [ylow,ycent,yhi] )
  # hottest bins
  top = sorted(range(len(content)), key=lambda i: content[i], reverse=True)[:topn]
  top = [ t for t in top if content[t] != 0 ]
  if len(top) == 0: return
  print("*"*30)
  print("**",len(top),"hottest bins in",h.GetName(),"**")
  det, ac, sam = convHname(h.GetName())

  if det is not None:
    proposal = "DET="+det+" and AC="+ac+" and SAM="+sam+" and" 
  else:
    proposal = ""
  for ind in top:
    # Special treatment for barrel
    thisprop = proposal
    if det is not None and int(det) == 0:
      if (abs(binx[ind][0]) > 1.4 or abs(binx[ind][2]) > 1.4) and int(sam)>1:
        thissam = str(int(sam)-1)
        thisprop = thisprop.replace("SAM="+sam, "SAM="+thissam)
    printstr = thisprop+" ETA between "+str(format(binx[ind][0],".4f"))+" and "+str(format(binx[ind][2],".4f"))+" and PHI between "+str(format(biny[ind][0],".4f"))+" and "+str(format(biny[ind][2],".4f"))

    ONL_ID = []
    if "LArCellMon" in histname:
      conn = sqlite3.connect('/afs/cern.ch/user/l/larmon/public/prod/LArIdtranslator/LArId.db')
      conn.row_factory  = sqlite3.Row
      cursor = conn.cursor()
      cmd = 'select distinct LARID.ONL_ID from LARID where '
      cmd += printstr.replace("DET","LARID.DET").replace("SAM","LARID.SAM").replace("ETA","LARID.ETA").replace("PHI","LARID.PHI")
      cursor.execute(cmd)
      all_row = cursor.fetchall()
      ONL_ID=[hex(all_row[i]['ONL_ID']) for i in range(len(all_row))]
      
    

    printstr += " (content = "+str(content[ind])
    if h_fracQth is not None:
      printstr += " & /Qth = "+str(format(fracQthContent[ind],".3f"))
    printstr += ")"
    if len(ONL_ID) != 0:
      printstr += " ONL_ID = "+(", ".join(ONL_ID))
    print(printstr)


if __name__ == "__main__":
  parser = argparse.ArgumentParser(description='Process some integers.')
  parser.add_argument('-r','--run',type=int,dest='runNumber',default='267599',help="Run number",action='store')
  parser.add_argument('-ll','--lowerlb',type=int,dest='lowerlb',default='0',help="Lower lb",action='store')
  parser.add_argument('-ul','--upperlb',type=int,dest='upperlb',default='999999',help="Upper lb",action='store')
  parser.add_argument('-s','--stream',dest='stream',default='Main',help="Stream with or without prefix: express/CosmicCalo/Main/ZeroBias/MinBias",action='store')
  parser.add_argument('-t','--tag',dest='tag',default='',help="DAQ tag: data16_13TeV, data16_cos...By default retrieve it via atlasdqm",action='store')
  parser.add_argument('-a','--amiTag',dest='amiTag',default='f',help="First letter of AMI tag: x->express / f->bulk",action='store')
  parser.add_argument('-x','--globalX',type=float,dest='globalX',default='-999.',help='X region common to all histos',action='store')
  parser.add_argument('-y','--globalY',type=float,dest='globalY',default='-999.',help='Y region common to all histos - Only for 2d',action='store')
  parser.add_argument('-ia','--integralAbove',type=float,dest='integralAbove',default='-999.',help='Lower bound of integral - Not used so far',action='store')
  parser.add_argument('-d','--globalDelta',type=float,dest='globalDelta',default='0.15',help='Distance to look around x/(x;y) for 1d/2d plot',action='store')
  parser.add_argument('--histo',dest='histo',default='',help='ROOT-file path of histograms - As many as you want with : [type("1d" or "2d")] [root path] [x] [y if 2d] [delta] (if not provided use global)',action='store',nargs="*")
  parser.add_argument('--histoWD',dest='histoWD',default='',help='Webdisplay path of histograms - As many as you want with : [type("1d" or "2d")] [root path] [x] [y if 2d] [delta] (if not provided use global)',action='store',nargs="*")
  parser.add_argument('--onlyBoxes', dest='onlyBoxes', default=False, action='store_true', help="Only show the histograms with the box around the target area - don't do the correlation analysis")
  parser.add_argument('--draw1D', dest='draw1D', default=False, action='store_true', help="Also draw the 1D correlation plots")
  parser.add_argument('-n','--topN', dest='topN', type=int, default=4, help="Report the N bins with the highest content")
  parser.add_argument('--checkCorr', dest='checkCorr', nargs='*', default = [ 'CaloTopoClusters' ], help="Only check the full per-LB correlation for combinations of plots where one of the plots contains this substring. As many as you want")
  args = parser.parse_args()
  
  # Info for the DQM APIs... if we are using them
  run_spec = {'stream': 'physics_CosmicCalo', 'proc_ver': 1,'source': 'tier0', 'low_run': args.runNumber, 'high_run':args.runNumber}

  dqmAPI = None

  grouped = {}

  if args.tag == "": # Try to retrieve the data project tag via atlasdqm
    dqmAPI = setupDqmAPI()
    run_info= dqmAPI.get_run_information(run_spec)
    if '%d'%args.runNumber not in list(run_info.keys()) or len(run_info['%d'%args.runNumber])<2:
      print("Unable to retrieve the data project tag via atlasdqm... Please double check your atlasdqmpass.txt or define it by hand with -t option")
      sys.exit()
    args.tag = run_info['%d'%args.runNumber][1]
    

  if len(args.histo) > 0: # The histograms ROOT file paths are directly provided 
    hArgs = args.histo
    for h in hArgs:
      if "*" in h:
        print("A wildcard was passed for histogram name input - this is not yet supported. Perhaps you meant to use the histoWD option?")
        sys.exit()
  elif len(args.histoWD) > 0: # The histograms paths are provided as webdisplay paths
    print("Web display paths provided: I will have to retrieve the ROOT file path of histograms")    
    args.histoWD, grouped = expandWildCard(args.histoWD)

    if dqmAPI is None:
      dqmAPI = setupDqmAPI()
      prefix = {'express':'express_','Egamma':'physics_','CosmicCalo':'physics_','JetTauEtmiss':'physics_','Main':'physics_','ZeroBias':'physics_','MinBias':'physics_'}
      run_spec['stream'] = "%s%s"%(prefix[args.stream],args.stream)
    hArgs = []
    for hist in args.histoWD:
      dqmf_config = dqmAPI.get_dqmf_configs(run_spec, hist)
      if len(dqmf_config.keys())== 0:
        print("Problem getting hist path from the provided WD string... is there a typo? You submitted",hist)
        print("Note - if you see two strings here perhaps you had a misplaced quote in your input arguments")
        sys.exit()
      histpath = dqmf_config['%d'%args.runNumber]['annotations']['inputname']
      hArgs.append(histpath)
      if hist in [ val for k,v in grouped.items() for val in v ]:
        gk = [ k for k,b in grouped.items() if hist in grouped[k] ][0]
        gi = grouped[gk].index(hist)
        grouped[gk][gi] = histpath
  else:
    print("You need to define at least 1 histogram...")
    sys.exit()
  

  print("Requested histograms are",hArgs)
  
  histos = {}
  canvs = {}
  for h in hArgs:
    # Print a proposed LArID translator SQL query
    # LIDProposal(h, args.globalX, args.globalY, args.globalDelta)
    histos[h] = {}
    canvs[h] = {}

  for hist in histos.keys():
    histos[hist]["nbHitInHot"] = [0.] * nLB
    histos[hist]["regionBins"] = []

  print("Finding the path to the merged hist file")
  mergedFilePath = pathExtract.returnEosHistPath( args.runNumber,
                                                  args.stream, args.amiTag, 
                                                  args.tag )
  runFilePath = "root://eosatlas.cern.ch/%s"%(mergedFilePath).rstrip()
  if ("FILE NOT FOUND" in runFilePath):
    print("No merged file found for this run")
    print("pathExtract.returnEosHistPath( "+str(args.runNumber)+", "+str( args.stream)+", "+str(args.amiTag)+", "+str(args.tag)+" )")
    print("HINT: check if there is a folder like","/eos/atlas/atlastier0/rucio/"+args.tag+"/physics_"+args.stream+"/00"+str(args.runNumber)+"/"+args.tag+".00"+str(args.runNumber)+".physics_"+args.stream+".*."+args.amiTag)
  else:
      print("I have found the merged HIST file %s"%(runFilePath))
      drawngroup = {}
      print("Opening the merged hist file and getting some information")
      f = R.TFile.Open(runFilePath)
      print("File is",runFilePath)
      for hist in histos.keys():
        hpath = "run_%d/%s"%(args.runNumber,hist)
        print("Reading histogram",hpath)
        histos[hist]["merged"] = f.Get(hpath)
        if not isinstance(histos[hist]["merged"], R.TH1) and not isinstance(histos[hist]["merged"], R.TH2): 
          print("WARNING: path",hpath,"was not found in merged file")
          continue
        histos[hist]["type"] = hType(histos[hist]["merged"])
        histos[hist]["min"] = histos[hist]["merged"].GetMinimum()*0.8
        histos[hist]["max"] = histos[hist]["merged"].GetMaximum()*1.2    
        histos[hist]["merged"].SetMinimum(histos[hist]["min"])
        histos[hist]["merged"].SetMaximum(histos[hist]["max"])


        # here, find another way to define x y delta?
        tmp_x = args.globalX
        tmp_delta = args.globalDelta
        # steps for iterating over bins in the scan
        nSteps = 1000
        subStep = 2*tmp_delta/nSteps

        groupname = None
        if hist in [ val for k,v in grouped.items() for val in v ]:
          groupname = [ k for k,b in grouped.items() if hist in grouped[k] ][0]
          if groupname not in canvs.keys():
            canvs[groupname] = R.TCanvas(groupname.replace("*","x"), groupname.replace("*","x"), 400*len(grouped[groupname]), 400)
            drawngroup[groupname] = 1
            if len(grouped[groupname]) <6:
              canvs[groupname].Divide(len(grouped[groupname]),1)
              print("dividing canvas", len(grouped[groupname]))
            else:
              print("Too many plots in the wildcard", groupname, len(grouped[groupname]))
              groupname = None

        if groupname is None:
          canvs[hist]["canv"] = R.TCanvas(hist, hist)
          thiscanv = canvs[hist]["canv"]
        else:
          thiscanv = canvs[groupname]        
          thiscanv.cd(drawngroup[groupname])

        if histos[hist]["type"] == "1d":
          histos[hist]["merged"].Draw()
          histos[hist]["box"] = R.TBox( tmp_x-tmp_delta,
                                     histos[hist]["min"],
                                     tmp_x+tmp_delta,
                                     histos[hist]["max"] )
          # Extract the list of bins where to count.
          # Scans the window to find all bins that fall in the window
          # The regionBins is defined for each histogram allowing different binning
          for ix in range(nSteps):
            iX = tmp_x - tmp_delta + ix * subStep 
            tmp_bin = histos[hist]["merged"].FindBin(iX)
            if (tmp_bin not in histos[hist]["regionBins"]):
              histos[hist]["regionBins"].append(tmp_bin)
        else: # 2D hist
          tmp_y = args.globalY
          R.gPad.SetLogz(1)
          R.gStyle.SetPalette(1)
          R.gStyle.SetOptStat("")
          print("Draw",hist)
          histos[hist]["merged"].Draw("COLZ")
          histos[hist]["box"] = R.TBox( tmp_x-tmp_delta,
                                     tmp_y-tmp_delta,
                                     tmp_x+tmp_delta,
                                     tmp_y+tmp_delta )
          # find the >Qth plot equivalent if this is the occupancy vs eta phi plot
          QthHist = None
          if "CellOccupancyVsEtaPhi" in hist:
            QthHistPath= hist.replace("2d_Occupancy/CellOccupancyVsEtaPhi", "2d_PoorQualityFraction/fractionOverQthVsEtaPhi").replace("_5Sigma_CSCveto", "_hiEth_noVeto").replace("_hiEth_CSCveto", "_hiEth_noVeto")
            QthHist = f.Get("run_%d/%s"%(args.runNumber,QthHistPath))

          # Extract the list of bins where to count.
          # Scans the window to find all bins that fall in the window
          # The regionBins is defined for each histogram allowing different binning
          for ix in range(nSteps):
            iX = tmp_x - tmp_delta + ix * subStep 
            for iy in range (nSteps):
              iY = tmp_y - tmp_delta + iy * subStep
              tmp_bin = histos[hist]["merged"].FindBin(iX,iY)
              if (tmp_bin not in histos[hist]["regionBins"]):
                histos[hist]["regionBins"].append(tmp_bin)
          topNBins(hist,histos[hist]["merged"],args.topN,histos[hist]["regionBins"], QthHist)
        # Draw a box on each of the plots, highlighting the region that we will compare
        histos[hist]["box"].SetLineColor(R.kRed+1)
        histos[hist]["box"].SetLineWidth(3)
        histos[hist]["box"].SetFillStyle(0)    
        histos[hist]["box"].Draw()

        thiscanv.objs = []
        thiscanv.objs.append(histos[hist]["box"])
        thiscanv.Update()
        if groupname is not None:
          drawngroup[groupname] += 1
          thiscanv.cd()

  if args.onlyBoxes is True:
    input("I am done...")
    sys.exit()
  

  print("Finding the paths to the per-LB hist files")
  # Extract all the unmerged files available with the LB range
  lbFilePathList = pathExtract.returnEosHistPathLB( args.runNumber,
                                                    args.lowerlb, args.upperlb,
                                                    args.stream, args.amiTag, args.tag )

  if isinstance(lbFilePathList,str) and "NOT FOUND" in lbFilePathList:
    print("Could not find per-LB files for this run")
    print("HINT: check if there is a folder like","/eos/atlas/atlastier0/tzero/prod/"+args.tag+"/physics_"+args.stream+"/00"+str(args.runNumber)+"/"+args.tag+".00"+str(args.runNumber)+".physics_"+args.stream+".*."+args.amiTag)
    sys.exit()

  print("I have found %d unmerged HIST files"%(len(lbFilePathList)))
  print("The first one is root://eosatlas.cern.ch/%s"%(lbFilePathList[0]))
  print("The last one is root://eosatlas.cern.ch/%s"%(lbFilePathList[-1]))

  print("Now iterating over per-LB files and getting bin contents")
  # Loop on all unmerged files
  # and store number of hits per histogram
  listLB = []
  for count,lbFile in enumerate(lbFilePathList):
    lbFilePath = "root://eosatlas.cern.ch/%s"%(lbFile).rstrip()
    # Extract lb from the filename and display it
    ilb = int((lbFile.split("_lb")[1]).split("._")[0])
    if ilb not in listLB:
      listLB.append(ilb)
    if (count%100 == 0):
      sys.stdout.write("\n I processed %d/%d files \n LBs:"%(count,len(lbFilePathList)))
    sys.stdout.write("%d "%(ilb))
    sys.stdout.flush()
    fLB = R.TFile.Open(lbFilePath)
    for iPath in histos.keys():
      histos[iPath][ilb] = fLB.Get("run_%d/%s"%(args.runNumber,iPath))
      if not isinstance(histos[iPath][ilb], R.TH1) and not isinstance(histos[iPath][ilb], R.TH2):
        continue

      for iBin in histos[iPath]['regionBins']:
        histos[iPath]["nbHitInHot"][ilb] = histos[iPath]["nbHitInHot"][ilb] + histos[iPath][ilb].GetBinContent(iBin)

    fLB.Close()

  print("Time to check for correlations")
  correls, fractionNonZero = checkCorrel(histos, listLB, args.checkCorr)
  
  getSummary(histos, correls, fractionNonZero)

  input("I am done...")
