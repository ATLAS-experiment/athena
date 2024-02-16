#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# art-description: art job for mu_zmumu_pu40
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena
# art-input: valid1.601190.PhPy8EG_AZNLO_Zmumu.recon.RDO.e8514_e8528_s4159_s4114_r14799_tid34171433_00
# art-input-nfiles: 5
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
Slices  = ['muon','muon-tnp']
Events  = 10000
Threads = 8
Slots   = 8
Input   = 'Zmumu'    # defined in TrigValTools/share/TrigValInputs.json
GridFiles = True

Jobs = [ ( "Truth",       " TIDAdata-run3.dat                    -o data-hists.root -p 13" ),
         ( "Offline",     " TIDAdata-run3-offline.dat -r Offline -o data-hists-offline.root" ) ]

Comp = [ ( "L2muon",              "L2muonTnP",      "data-hists.root",         " -c TIDAhisto-panel-TnP.dat  -d HLTL2-plots " ),
         ( "L2muon-lowpt",        "L2muonLowpt", "data-hists.root",         " -c TIDAhisto-panel.dat  -d HLTL2-plots-lowpt " ),
         ( "L2muonoffline",       "L2muonTnP",      "data-hists-offline.root", " -c TIDAhisto-panel-TnP.dat  -d HLTL2-plots-offline " ),
         ( "L2muonoffline-lowpt", "L2muonLowpt", "data-hists-offline.root", " -c TIDAhisto-panel.dat  -d HLTL2-plots-lowpt-offline " ),
         ( "EFmuon",              "EFmuonTnP",      "data-hists.root",         " -c TIDAhisto-panel-TnP.dat  -d HLTEF-plots " ),
         ( "EFmuon-lowpt",        "EFmuonLowpt", "data-hists.root",         " -c TIDAhisto-panel.dat  -d HLTEF-plots-lowpt " ),
         ( "EFmuonoffline",       "EFmuonTnP",      "data-hists-offline.root", " -c TIDAhisto-panel-TnP.dat  -d HLTEF-plots-offline " ),
         ( "EFmuonoffline-lowpt", "EFmuonLowpt", "data-hists-offline.root", " -c TIDAhisto-panel.dat  -d HLTEF-plots-lowpt-offline " ) ]


from AthenaCommon.Include import include 
include("TrigInDetValidation/TrigInDetValidation_Base.py")


 
