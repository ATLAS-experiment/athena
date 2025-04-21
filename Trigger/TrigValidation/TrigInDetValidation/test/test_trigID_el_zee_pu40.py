#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# art-description: art job for el_zee_pu40
# art-type: grid
# art-include: main/Athena/x86_64-el9-gcc14-opt
# art-include: 24.0/Athena
# art-input: valid1.601189.PhPy8EG_AZNLO_Zee.recon.RDO.e8514_e8528_s4369_s4370_r16083_tid42133937_00
# art-input-nfiles: 8
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


Slices  = ['electron','electronTnP']
Events  = 16000
Threads = 8
Slots   = 8
Input   = 'Zee'    # defined in TrigValTools/share/TrigValInputs.json
GridFiles = True
Release = "current"

Jobs = [ ( "Truth",       " TIDAdata-run3.dat                    -o data-hists.root -p 11" ),
         ( "Offline",     " TIDAdata-run3-offline.dat -r Offline -o data-hists-offline.root" ) ]

Comp = [ ( "L2ele",              "L2electronTnP",      "data-hists.root",         " -c TIDAhisto-panel-TnP.dat  -d HLTL2-plots " ),
         ( "L2ele-lowpt",        "L2electronLowpt", "data-hists.root",         " -c TIDAhisto-panel.dat  -d HLTL2-plots-lowpt " ),
         ( "L2eleoffline",       "L2electronTnP",      "data-hists-offline.root", " -c TIDAhisto-panel-TnP.dat  -d HLTL2-plots-offline " ),
         ( "L2eleoffline-lowpt", "L2electronLowpt", "data-hists-offline.root", " -c TIDAhisto-panel.dat  -d HLTL2-plots-lowpt-offline " ),
         ( "EFele",              "EFelectronTnP",      "data-hists.root",         " -c TIDAhisto-panel-TnP.dat  -d HLTEF-plots " ),
         ( "EFele-lowpt",        "EFelectronLowpt", "data-hists.root",         " -c TIDAhisto-panel.dat  -d HLTEF-plots-lowpt " ),
         ( "EFeleoffline",       "EFelectronTnP",      "data-hists-offline.root", " -c TIDAhisto-panel-TnP.dat  -d HLTEF-plots-offline " ),
         ( "EFeleoffline-lowpt", "EFelectronLowpt", "data-hists-offline.root", " -c TIDAhisto-panel.dat  -d HLTEF-plots-lowpt-offline " )]


from AthenaCommon.Include import include 
include("TrigInDetValidation/TrigInDetValidation_Base.py")
