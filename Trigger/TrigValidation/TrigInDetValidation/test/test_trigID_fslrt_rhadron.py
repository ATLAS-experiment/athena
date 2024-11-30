#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# art-description: art job for fslrt_rhadron
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena
# art-input: valid1.545836.MGPy8EG_A14NNPDF23LO_TT_RPVdirectBL_2400_tau_3ns.recon.RDO.e8582_e8528_s4418_s4370_r16083_tid42189726_00
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


Slices  = ['FSLRT']
Events  = 8000
Threads = 8
Slots   = 8
Input   = 'RHadron'    # defined in TrigValTools/share/TrigValInputs.json
GridFiles = True
ExtraAna = ' --LRT=True '
Release = "current"

# legacy                                                                                                                                                                                 
# preinclude_file = 'RDOtoRDOTrigger:TrigInDetValidation/TIDAlrt_preinclude.py'

# CA
# ATR-25582 - FSLRT is now excluded from the default dev menu so need to change to the full dev
# menu rather than the filtered versions
preexec_trig="flags.Trigger.triggerMenuSetup='Dev_pp_run3_v1';"


Jobs = [ ( "Truth",  " TIDAdata-run3-fslrt.dat -o data-hists.root ", "Test_bin_lrt.dat" ),
         ( "Offline",    " TIDAdata-run3-offline-fslrt.dat -r Offline+InDetLargeD0TrackParticles -o data-hists-offline.root", "Test_bin_lrt.dat" ) ]

Comp = [ ( "L2FSLRT",  "L2FSLRT",  "data-hists.root",  " -c TIDAhisto-panel.dat -d HLTL2-plots -sx Reference Truth " ),
         ( "EFFSLRT",  "EFFSLRT", "data-hists.root",   " -c TIDAhisto-panel.dat -d HLTEF-plots -sx Reference Truth   " ),
         ( "L2FSLRToffline",   "L2FSLRT","data-hists-offline.root",   " -c TIDAhisto-panel.dat -d HLTL2-plots-offline -sx Reference Offline " ),
         ( "EFFSLRToffline",   "EFFSLRT", "data-hists-offline.root",   " -c TIDAhisto-panel.dat -d HLTEF-plots-offline -sx Reference Offline " )
       ]


from AthenaCommon.Include import include
include("TrigInDetValidation/TrigInDetValidation_Base.py")
