#!/usr/bin/env python

# art-description: DirectIOART TFile::Open RucioRedirector
# art-type: grid
# art-include: main/Athena

import ROOT
ROOT.gEnv.SetValue("Davix.Debug", 3.0)

### no redirection option ###
f = ROOT.TFile.Open("https://rucio-lb-prod.cern.ch/redirect/mc15_13TeV/AOD.05536542._000001.pool.root.1")
if f: print("art-result: 0 DirectIOART_TFileOpen_RucioRedirect_noOpts")
else: print("art-result: 1 DirectIOART_TFileOpen_RucioRedirect_noOpts")
del f

### schemes=root ###
f = ROOT.TFile.Open("https://rucio-lb-prod.cern.ch/redirect/mc15_13TeV/AOD.05536542._000001.pool.root.1?schemes=root")
if f: print("art-result: 0 DirectIOART_TFileOpen_RucioRedirect_schemesROOT")
else: print("art-result: 1 DirectIOART_TFileOpen_RucioRedirect_schemesROOT")
del f

### schemes=davs ###
f = ROOT.TFile.Open("https://rucio-lb-prod.cern.ch/redirect/mc15_13TeV/AOD.05536542._000001.pool.root.1?schemes=davs")
if f: print("art-result: 0 DirectIOART_TFileOpen_RucioRedirect_schemesDAVS")
else: print("art-result: 1 DirectIOART_TFileOpen_RucioRedirect_schemesDAVS")
del f

### select=goeip ###
f = ROOT.TFile.Open("https://rucio-lb-prod.cern.ch/redirect/mc15_13TeV/AOD.05536542._000001.pool.root.1?select=geoip")
if f: print("art-result: 0 DirectIOART_TFileOpen_RucioRedirect_selectGEOIP")
else: print("art-result: 1 DirectIOART_TFileOpen_RucioRedirect_selectGEOIP")
del f

### rse=LRZ-LMU_DATADISK ###
f = ROOT.TFile.Open("https://rucio-lb-prod.cern.ch/redirect/mc15_13TeV/AOD.05536542._000001.pool.root.1?rse=LRZ-LMU_DATADISK")
if f: print("art-result: 0 DirectIOART_TFileOpen_RucioRedirect_rseLRZ-LMU_DATADISK")
else: print("art-result: 1 DirectIOART_TFileOpen_RucioRedirect_rseLRZ-LMU_DATADISK")
del f

outstr = """<?xml version="1.0" encoding="UTF-8" standalone="no" ?>
<!-- Edited By POOL -->
<!DOCTYPE POOLFILECATALOG SYSTEM "InMemory">
<POOLFILECATALOG>

</POOLFILECATALOG>"""
with open("PoolFileCatalog.xml", "w") as outfile:
  outfile.write(outstr)
