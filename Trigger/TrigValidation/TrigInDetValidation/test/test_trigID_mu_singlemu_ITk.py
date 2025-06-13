#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# art-description: art job for mu_singlemu_ITk
# art-type: grid
# art-include: main/Athena/x86_64-el9-gcc14-opt
# art-input: mc21_14TeV.900498.PG_single_muonpm_Pt100_etaFlatnp0_43.recon.RDO.e8481_s4149_r14697
# art-input-nfiles: 20
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

Slices  = ['muon']
Events  = 20000 
Threads = 8
Slots   = 8
Input   = 'Single_mu_Run4'    # defined in TrigValTools/share/TrigValInputs.json
# by default, all MC tests override the global conditions tag and force defaultConditionsTags.RUN3_MC, which is not suitable for Run4
conditionsOverride = 'Run4'

preexec_trig = "flags.Tracking.doTruth=False;flags.Trigger.enableL1CaloPhase1=False;"

Jobs = [ ( "Truth",       " TIDAdata-run4.dat                    -o data-hists.root -p 13",   "Test_bin.dat" ),
         ( "Offline",     " TIDAdata-run4-offline.dat -r Offline -o data-hists-offline.root", "Test_bin.dat" ) ]

Comp = [ ( "L2muon",              "L2muon",      "data-hists.root",         " -c TIDAhisto-panel.dat  -d HLTL2-plots " ),
         ( "L2muon-lowpt",        "L2muonLowpt", "data-hists.root",         " -c TIDAhisto-panel.dat  -d HLTL2-plots-lowpt " ),
         ( "L2muonoffline",       "L2muon",      "data-hists-offline.root", " -c TIDAhisto-panel.dat  -d HLTL2-plots-offline " ),
         ( "L2muonoffline-lowpt", "L2muonLowpt", "data-hists-offline.root", " -c TIDAhisto-panel.dat  -d HLTL2-plots-lowpt-offline " ),
         ( "EFmuon",              "EFmuon",      "data-hists.root",         " -c TIDAhisto-panel.dat  -d HLTEF-plots " ),
         ( "EFmuon-lowpt",        "EFmuonLowpt", "data-hists.root",         " -c TIDAhisto-panel.dat  -d HLTEF-plots-lowpt " ),
         ( "EFmuonoffline",       "EFmuon",      "data-hists-offline.root", " -c TIDAhisto-panel.dat  -d HLTEF-plots-offline " ),
         ( "EFmuonoffline-lowpt", "EFmuonLowpt", "data-hists-offline.root", " -c TIDAhisto-panel.dat  -d HLTEF-plots-lowpt-offline " ) ]

from AthenaCommon.Include import include 
include("TrigInDetValidation/TrigInDetValidation_Base.py")
