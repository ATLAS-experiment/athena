#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# art-description: art job for cosmic
# art-type: grid
# art-include: main/Athena/x86_64-el9-gcc14-opt
# art-include: 24.0/Athena
# art-input: group.trig-hlt.mc23_13p6TeV.310772.CosmicRays_CollisionSetup.recon.RDO.s4261_s4260_r15236_tid36836491_00
# art-input-nfiles: 1
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
# currently only 1k events available, bump up to 4k once staging from tape to disk is complete
Events  = 4000
Threads = 8 
Slots   = 8
Release = "current"
preexec_reco = ["from AthenaConfiguration.Enums import BeamType", "flags.Beam.Type=BeamType.Cosmics",
                "flags.Tracking.doTRTStandalone=False",
                "flags.Tracking.doForwardTracks=False",
                "flags.Tracking.doLargeD0=False"]
Input   = 'mc_cosmics'    # defined in TrigValTools/share/TrigValInputs.json  

Jobs = [ ( "Offline",     " TIDAdata-run3-offline-cosmic.dat      -r Offline -o data-hists-offline.root" ) ]


Comp = [  ("EFcosmic",       "EFcosmic",      "data-hists-offline.root",   " -c TIDAhisto-panel.dat  -d HLTEF-plots " ) ]
   
from AthenaCommon.Include import include 
include("TrigInDetValidation/TrigInDetValidation_Base.py")

