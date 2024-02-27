#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# art-description: art job for all_ttbar_tier0_pu40
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena
# art-input: valid1.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8514_e8528_s4159_s4114_r14799_tid34200060_00
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



useCA_Reco = True
Slices  = ['muon','muon-tnp','electron','electron-tnp','tau','bjet','fsjet']
Events  = 4000
Threads = 8 
Slots   = 8
Release = "current"

ExtraAna = " --doTIDATier0=True "

Input   = 'ttbar'    # defined in TrigValTools/share/TrigValInputs.json  
GridFiles = True

Jobs = [] 

Comp = [ ( "L2muon",         "L2muon",        "data-hists-tier0.root",   " -b HLT/TRIDT/Muon/All     -s '_HLT_IDTrack' '/HLT_IDTrack' -c TIDAhisto-tier0.dat     --ncols 3 -d HLTL2-plots-muon " ),
         ( "L2muonTnP",      "L2muonTnP",     "data-hists-tier0.root",   " -b HLT/TRIDT/Muon/All     -s '_HLT_IDTrack' '/HLT_IDTrack' -c TIDAhisto-tier0-TnP.dat --ncols 3 -d HLTL2-plots-muon-TnP " ),
         ( "L2electron",     "L2electron",    "data-hists-tier0.root",   " -b HLT/TRIDT/Egamma/All   -s '_HLT_IDTrack' '/HLT_IDTrack' -c TIDAhisto-tier0.dat     --ncols 3 -d HLTL2-plots-electron " ),
         ( "L2electronTnP",  "L2electronTnP", "data-hists-tier0.root",   " -b HLT/TRIDT/Egamma/All   -s '_HLT_IDTrack' '/HLT_IDTrack' -c TIDAhisto-tier0-TnP.dat --ncols 3 -d HLTL2-plots-electron-TnP " ),
         ( "L2tau",          "L2tau",         "data-hists-tier0.root",   " -b HLT/TRIDT/Tau/All      -s '_HLT_IDTrack' '/HLT_IDTrack' -c TIDAhisto-tier0.dat     --ncols 3 -d HLTL2-plots-tau " ),
         ( "L2bjet",         "L2bjet",        "data-hists-tier0.root",   " -b HLT/TRIDT/Bjet/All     -s '_HLT_IDTrack' '/HLT_IDTrack' -c TIDAhisto-tier0.dat     --ncols 3 -d HLTL2-plots-bjet " ),   
         ( "FSjetoffline",   "L2fsjet",       "data-hists-tier0.root",   " -b HLT/TRIDT/Bjet/All     -s '_HLT_IDTrack' '/HLT_IDTrack' -c TIDAhisto-tier0.dat     --ncols 3 -d HLTL2-plots-FS " ),
         ( "FSt0vtx",        "L2fst0vtx",     "data-hists-tier0.root",   " -b HLT/TRIDT/Bjet/All     -s '_HLT_IDTrack' '/HLT_IDTrack' -c TIDAhisto-tier0-vtx.dat --ncols 3 -d HLTL2-plots-vtx " ),

         ( "EFmuon",         "EFmuon",        "data-hists-tier0.root",   " -b HLT/TRIDT/Muon/All     -s '_HLT_IDTrack' '/HLT_IDTrack' -c TIDAhisto-tier0.dat     --ncols 3 -d HLTEF-plots-muon " ),
         ( "EFmuonTnP",      "EFmuonTnP",     "data-hists-tier0.root",   " -b HLT/TRIDT/Muon/All     -s '_HLT_IDTrack' '/HLT_IDTrack' -c TIDAhisto-tier0-TnP.dat --ncols 3 -d HLTEF-plots-muon-TnP " ),
         ( "EFelectron",     "EFelectron",    "data-hists-tier0.root",   " -b HLT/TRIDT/Egamma/All   -s '_HLT_IDTrack' '/HLT_IDTrack' -c TIDAhisto-tier0.dat     --ncols 3 -d HLTEF-plots-electron " ),
         ( "EFelectronTnP",  "EFelectronTnP", "data-hists-tier0.root",   " -b HLT/TRIDT/Egamma/All   -s '_HLT_IDTrack' '/HLT_IDTrack' -c TIDAhisto-tier0-TnP.dat --ncols 3 -d HLTEF-plots-electron-TnP " ),
         ( "EFtau",          "EFtau",         "data-hists-tier0.root",   " -b HLT/TRIDT/Tau/All      -s '_HLT_IDTrack' '/HLT_IDTrack' -c TIDAhisto-tier0.dat     --ncols 3 -d HLTEF-plots-tau " ),
         ( "EFbjet",         "EFbjet",        "data-hists-tier0.root",   " -b HLT/TRIDT/Bjet/All     -s '_HLT_IDTrack' '/HLT_IDTrack' -c TIDAhisto-tier0.dat     --ncols 3 -d HLTEF-plots-bjet " ) ]
   
from AthenaCommon.Include import include 
include("TrigInDetValidation/TrigInDetValidation_Base.py")

