#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# art-description: art job for cosmic_data
# art-type: grid
# art-include: main/Athena/x86_64-el9-gcc14-opt
# art-include: 24.0/Athena
# art-input: group.trig-hlt.data23_cos.00457007.physics_CosmicMuons.merge.RAW
# art-input-nfiles: 4
# art-pathena-flags-remove: "--respectLB"
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


Slices  = ['cosmic']
Events  = 4000
Threads = 8 
Slots   = 8
Release = "current"
preexec_reco = ["from AthenaConfiguration.Enums import BeamType", "flags.Beam.Type=BeamType.Cosmics",
                "flags.Tracking.doTRTStandalone=False",
                "flags.Tracking.doForwardTracks=False",
                "flags.Tracking.doLargeD0=False"]
Input   = 'data_cos'    # defined in TrigValTools/share/TrigValInputs.json
# the art-pathena-flags-remove flag allows multiple LBs to be processed in 1 job (ATR-26472)
GridFiles = True
# needed when processing multiple LBs in trigbs_extractStream.py
MultipleLB = True

Jobs = [ ( "Offline",     " TIDAdata-run3-offline-cosmic.dat      -r Offline -o data-hists-offline.root" ) ]


Comp = [ ( "EFcosmic",       "EFcosmic",      "data-hists-offline.root",   " -c TIDAhisto-panel.dat  -d HLTEF-plots " ) ]
   
from AthenaCommon.Include import include 
include("TrigInDetValidation/TrigInDetValidation_Base_data.py")

