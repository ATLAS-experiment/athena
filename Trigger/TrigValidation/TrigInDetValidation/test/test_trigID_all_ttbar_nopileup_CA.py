#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Elliot - This test is a duplicate of test_trigID_all_ttbar_nopileup.py which will use CA for the RDOtoRDOTrigger step. Included to ensure that results for non-CA and CA implementations are consistent, intended so that this duplicate will be deleted once all tests are migrated to use CA for the RDOtoRDOTrigger step. Confirmed with ID trigger coordinators.

# art-description: art job for all_ttbar_nopileup_CA
# art-type: grid
# art-include: main/Athena
# art-include: 23.0/Athena
# art-input: valid1.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8514_e8528_s4159_s4114_r14838_tid34200062_00
# art-input-nfiles: 2
# art-athena-mt: 8
# art-html: https://idtrigger-val.web.cern.ch/idtrigger-val/TIDAWeb/TIDAart/?jobdir=
# art-output: *.txt
# art-output: *.log
# art-output: log.*
# art-output: *.out
# art-output: *.err
# art-output: *.log.tar.gz
# art-output: *.new
# art-output: *.json
# art-output: d*.root
# art-output: e*.root
# art-output: T*.root
# art-output: *.check*
# art-output: HLT*
# art-output: times*
# art-output: cost-perCall
# art-output: cost-perEvent
# art-output: cost-perCall-chain
# art-output: cost-perEvent-chain
# art-output: *.dat 


Slices  = ['muon','electron','tau','bjet','fsjet']
Events  = 4000
Threads = 8 
Slots   = 8
Release = "current"

useCA_Reco = True # Use CA for RDOtoRDOTrigger step

Input   = 'ttbar_noPU'    # defined in TrigValTools/share/TrigValInputs.json  
GridFiles = True

Jobs = [ ( "Offline",     " TIDAdata-run3-offline.dat      -r Offline -o data-hists-offline.root" ),
         ( "OfflineVtx",  " TIDAdata-run3-offline-vtx.dat  -r Offline -o data-hists-offline-vtx.root" ) ]

Comp = [ ( "L2muon",       "L2muon",      "data-hists-offline.root",      " -c TIDAhisto-panel.dat  -d HLTL2-plots-muon " ),
         ( "L2electron",   "L2electron",  "data-hists-offline.root",      " -c TIDAhisto-panel.dat  -d HLTL2-plots-electron " ),
         ( "L2tau",        "L2tau",       "data-hists-offline.root",      " -c TIDAhisto-panel.dat  -d HLTL2-plots-tau " ),
         ( "L2bjet",       "L2bjet",      "data-hists-offline.root",      " -c TIDAhisto-panel.dat  -d HLTL2-plots-bjet " ),   
         ( "FSjetoffline", "L2fsjet",     "data-hists-offline.root",      " -c TIDAhisto-panel.dat  -d HLTL2-plots-FS " ),
         ( "FSvtx",        "L2fsjetvtx",  "data-hists-offline-vtx.root",  " -c TIDAhisto-panel-vtx.dat  -d HLTL2-plots-vtx     --ncols 3" ),
         ( "FSvtxall",     "L2fsjetvtx",  "data-hists-offline.root",      " -c TIDAhisto-panel-vtx.dat  -d HLTL2-plots-vtxall  --ncols 3" ), 

         ( "EFmuon",       "EFmuon",      "data-hists-offline.root",   " -c TIDAhisto-panel.dat  -d HLTEF-plots-muon " ),
         ( "EFelectron",   "EFelectron",  "data-hists-offline.root",   " -c TIDAhisto-panel.dat  -d HLTEF-plots-electron " ),
         ( "EFtau",        "EFtau",       "data-hists-offline.root",   " -c TIDAhisto-panel.dat  -d HLTEF-plots-tau " ),
         ( "EFbjet",       "EFbjet",      "data-hists-offline.root",   " -c TIDAhisto-panel.dat  -d HLTEF-plots-bjet " ) ]
   
from AthenaCommon.Include import include 
include("TrigInDetValidation/TrigInDetValidation_Base.py")
