#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Elliot - This test is a duplicate of test_trigID_tau_ztautau_lowpt_pu46.py which will use CA for the RDOtoRDOTrigger step. Included to ensure that results for non-CA and CA implementations are consistent, intended so that this duplicate will be deleted once all tests are migrated to use CA for the RDOtoRDOTrigger step. Confirmed with ID trigger coordinators.

# art-description: art job for tau_ztautau_lowpt_pu46_CA
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena
# art-input: valid1.601191.PhPy8EG_AZNLO_Ztautau.recon.RDO.e8514_e8528_s4159_s4114_r14799_tid34200039_00
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


Slices  = ['tau']
Events  = 6000
Threads = 8
Slots   = 8
Release = "current"
Input   = 'Ztautau'    # defined in TrigValTools/share/TrigValInputs.json  
GridFiles = True

useCA_Reco = True # Use CA for RDOtoRDOTrigger step

preexec_trig = "from AthenaCommon.SystemOfUnits import GeV;flags.Trigger.InDetTracking.tauIso.pTmin=0.8*GeV;"

Jobs = [ ( "Offline",  " TIDAdata-run3-offline.dat -r Offline -o data-hists-offline.root" ),
         ( "Truth",    " TIDAdata-run3.dat                    -o data-hists.root" ) ]


Comp = [ ( "L2tau",        "L2tau",       "data-hists.root",          " -c TIDAhisto-panel.dat  -d HLTL2-plots " ),
         ( "EFtau",        "EFtau",       "data-hists.root",          " -c TIDAhisto-panel.dat  -d HLTEF-plots " ),
         ( "L2tauOff",     "L2tau",       "data-hists-offline.root",  " -c TIDAhisto-panel.dat  -d HLTL2-plots-offline " ),
         ( "EFtauOff",     "EFtau",       "data-hists-offline.root",  " -c TIDAhisto-panel.dat  -d HLTEF-plots-offline " ),
         ( "EFvtx",        "EFtauvtx",    "data-hists-offline.root",  " -c TIDAhisto-panel-vtx.dat  -d HLTL2-plots-vtx  --ncols 3" ) ]

   
from AthenaCommon.Include import include 
include("TrigInDetValidation/TrigInDetValidation_Base.py")
