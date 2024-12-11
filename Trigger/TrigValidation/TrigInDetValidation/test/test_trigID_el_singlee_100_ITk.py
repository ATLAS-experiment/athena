#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# art-description: art job for el_singlee_100_ITk
# art-type: grid
# art-include: main/Athena
# art-input: mc21_14TeV.900497.PG_single_epm_Pt100_etaFlatnp0_43.recon.RDO.e8481_s4149_r14697
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

Slices  = ['electron']
Events  = 20000 
Threads = 8
Slots   = 8
Input   = 'Single_el_Run4'    # defined in TrigValTools/share/TrigValInputs.json
GridFiles = True
# by default, all MC tests override the global conditions tag and force defaultConditionsTags.RUN3_MC, which is not suitable for Run4
conditionsOverride = 'Run4'

preexec_trig = "flags.Tracking.doTruth=False;flags.Trigger.enableL1CaloPhase1=False;"

Jobs = [ ( "Truth",       " TIDAdata-run4.dat                    -o data-hists.root -p 11" ),
         ( "Offline",     " TIDAdata-run4-offline.dat -r Offline -o data-hists-offline.root" ) ]

Comp = [ ( "L2ele",              "L2electron",      "data-hists.root",         " -c TIDAhisto-panel.dat  -d HLTL2-plots " ),
         ( "L2ele-lowpt",        "L2electronLowpt", "data-hists.root",         " -c TIDAhisto-panel.dat  -d HLTL2-plots-lowpt " ),
         ( "L2eleoffline",       "L2electron",      "data-hists-offline.root", " -c TIDAhisto-panel.dat  -d HLTL2-plots-offline " ),
         ( "L2eleoffline-lowpt", "L2electronLowpt", "data-hists-offline.root", " -c TIDAhisto-panel.dat  -d HLTL2-plots-lowpt-offline " ),
         ( "EFele",              "EFelectron",      "data-hists.root",         " -c TIDAhisto-panel.dat  -d HLTEF-plots " ),
         ( "EFele-lowpt",        "EFelectronLowpt", "data-hists.root",         " -c TIDAhisto-panel.dat  -d HLTEF-plots-lowpt " ),
         ( "EFeleoffline",       "EFelectron",      "data-hists-offline.root", " -c TIDAhisto-panel.dat  -d HLTEF-plots-offline " ),
         ( "EFeleoffline-lowpt", "EFelectronLowpt", "data-hists-offline.root", " -c TIDAhisto-panel.dat  -d HLTEF-plots-lowpt-offline " ) ]

from AthenaCommon.Include import include 
include("TrigInDetValidation/TrigInDetValidation_Base.py")
