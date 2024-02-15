#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# art-description: art job for ellrt_staustau
# art-type: grid
# art-include: main/Athena
# art-include: 23.0/Athena
# art-input: valid1.516757.MGPy8EG_A14NNPDF23LO_SelSelLLP_100_0_1ns.recon.RDO.e8514_e8528_s4159_s4114_r14844_tid34200279_00
# art-input-nfiles: 4
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
Slices  = ['L2electronLRT']
Events  = 8000 
Threads = 8 
Slots   = 8
Input   = 'SelSel'    # defined in TrigValTools/share/TrigValInputs.json
GridFiles = True
Release = "current"

ExtraAna = ' -c LRT="True" '

# legacy 
# preinclude_file = 'RDOtoRDOTrigger:TrigInDetValidation/TIDAlrt_preinclude.py'

# CA
# ATR-25582 - FSLRT is now excluded from the default dev menu so need to change to the full dev 
# menu rather than the filtered versions
preexec_trig="flags.Trigger.triggerMenuSetup='Dev_pp_run3_v1';"



Jobs = [ ( "Truth",  " TIDAdata-run3-lrt.dat -o data-hists.root -p 11", "Test_bin_lrt.dat" ),
         ( "Offline",    " TIDAdata-run3-offline-lrt.dat -r Offline+InDetLargeD0TrackParticles -o data-hists-offline.root", "Test_bin_lrt.dat" ) ]

Comp = [ ( "L2electronLRT",  "L2electronLRT",  "data-hists.root",  " -c TIDAhisto-panel.dat -d HLTL2-plots -sx Reference Truth " ),
         ( "EFelectronLRT",  "EFelectronLRT", "data-hists.root",   " -c TIDAhisto-panel.dat -d HLTEF-plots -sx Reference Truth " ),
         ( "L2electronLRToffline",   "L2electronLRT","data-hists-offline.root",   " -c TIDAhisto-panel.dat -d HLTL2-plots-offline -sx Reference Offline " ),
         ( "EFelectronLRToffline",   "EFelectronLRT", "data-hists-offline.root",   " -c TIDAhisto-panel.dat -d HLTEF-plots-offline -sx Reference Offline " )
       ]


from AthenaCommon.Include import include 
include("TrigInDetValidation/TrigInDetValidation_Base.py")
