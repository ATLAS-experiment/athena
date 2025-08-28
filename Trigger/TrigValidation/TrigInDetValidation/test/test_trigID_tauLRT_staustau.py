#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# art-description: art job for tauLRT_staustau
# art-type: grid
# art-include: main/Athena/x86_64-el9-gcc14-opt
# art-include: 24.0/Athena
# art-input: valid1.516640.MGPy8EG_A14NNPDF23LO_StauStauLLP_100_0_1ns.recon.RDO.e8514_e8528_s4369_s4370_r16083_tid42134894_00
# art-input-nfiles: 3
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


Slices  = ['tauLRT']
Events  = 5000
Threads = 8
Slots   = 8
Release = "current"
Input   = 'StauStau'    # defined in TrigValTools/share/TrigValInputs.json

ExtraAna = " --LRT=True --parentpdgid=15 "

# legacy 
# preinclude_file = 'RDOtoRDOTrigger:TrigInDetValidation/TIDAlrt_preinclude.py'

# CA
# ATR-25582 - FSLRT is now excluded from the default dev menu so need to change to the full dev 
# menu rather than the filtered versions
preexec_trig="flags.Trigger.triggerMenuSetup='Dev_pp_run3_v1';"


Jobs = [ ( "Offline",  " TIDAdata-run3-offline-lrt.dat -r Offline+InDetLargeD0TrackParticles -o data-hists-offline-lrt.root", "Test_bin_lrt.dat" ),
         ( "Truth",    " TIDAdata-run3-lrt.dat                    -o data-hists-lrt.root",         "Test_bin_lrt.dat" ) ]

Comp = [ ( "EFtauLRT",       "EFtauLRT",      "data-hists-lrt.root",         " -c TIDAhisto-panel.dat  -d HLTEF-plots " ),
         ( "L2tauLRT",       "L2tauLRT",      "data-hists-lrt.root",         " -c TIDAhisto-panel.dat  -d HLTL2-plots " ),
         ( "EFtauLRTOff",    "EFtauLRT",      "data-hists-offline-lrt.root", " -c TIDAhisto-panel.dat  -d HLTEF-plots-offline " ),
         ( "L2tauLRTOff",    "L2tauLRT",      "data-hists-offline-lrt.root", " -c TIDAhisto-panel.dat  -d HLTL2-plots-offline " ) ]


from AthenaCommon.Include import include
include("TrigInDetValidation/TrigInDetValidation_Base.py")
