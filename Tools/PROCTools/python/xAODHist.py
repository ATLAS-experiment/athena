#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from __future__ import print_function
import sys
import os
import ROOT
import argparse


def safeRetrieve(evt, typ, key):
    if evt.contains(typ, key):
        return evt.retrieve(typ, key)
    print(f'WARNING: Cannot find object {typ}/{key}')
    return []


def xAODHist(evt, phys=False, analysis=False, histfile=None):
    # Set output HIST filename
    if histfile:
        histfilename = histfile
    else:
        histfilename = "hist.root"

    # Book histograms
    hfile = ROOT.TFile( histfilename, 'RECREATE', 'ROOT file with histograms' )

    if phys:
      hphys_nclus = ROOT.TH1F( 'phys_nclus', 'phys_nclus', 20, 0, 20 )
      hphys_nidtracks = ROOT.TH1F( 'phys_nidtracks', 'phys_nidtracks', 20, 0, 20 )
      hphys_ntautracks = ROOT.TH1F( 'phys_ntautracks', 'phys_ntautracks', 20, 0, 20 )
      hphys_ntaus = ROOT.TH1F( 'phys_ntaus', 'phys_taus', 20, 0, 20 )
      hphys_nmuons = ROOT.TH1F( 'phys_nmuons', 'phys_nmuons', 20, 0, 20 )
      hphys_nelecs = ROOT.TH1F( 'phys_nelecs', 'phys_nelecs', 20, 0, 20 )
      hphys_nphotons = ROOT.TH1F( 'phys_nphotons', 'phys_nphotons', 20, 0, 20 )
      hphys_njets = ROOT.TH1F( 'phys_njets', 'phys_njets', 20, 0, 20 )
      hphys_nfakeelectrons = ROOT.TH1F( 'phys_nfakeelectrons', 'phys_nfakeelectrons', 20, 0, 20 )
      hphys_nfakephotons = ROOT.TH1F( 'phys_nfakephotons', 'phys_nfakephotons', 20, 0, 20 )

    if analysis:
        hntaujet = ROOT.TH1F( 'ana_ntaujet', 'ana_ntaujet', 20, 0, 20 )
        htaujetpt = ROOT.TH1F( 'ana_taujet_pt', 'ana_taujet_pt', 200, 0, 200 )
        htaujeteta = ROOT.TH1F( 'ana_taujet_eta', 'ana_taujet_eta', 100, -5, 5 )
        htaujetphi = ROOT.TH1F( 'ana_taujet_phi', 'ana_taujet_phi', 70, -3.5, 3.5 )

        hnelectron = ROOT.TH1F( 'ana_nelectron', 'ana_nelectron', 20, 0, 20 )
        helectronpt = ROOT.TH1F( 'ana_electron_pt', 'ana_electron_pt', 200, 0, 200 )
        helectroneta = ROOT.TH1F( 'ana_electron_eta', 'ana_electron_eta', 100, -5, 5 )
        helectronphi = ROOT.TH1F( 'ana_electron_phi', 'ana_electron_phi', 70, -3.5, 3.5 )

        hnmuon = ROOT.TH1F( 'ana_nmuon', 'ana_nmuon', 20, 0, 20 )
        hmuonpt = ROOT.TH1F( 'ana_muon_pt', 'ana_muon_pt', 200, 0, 200 )
        hmuoneta = ROOT.TH1F( 'ana_muon_eta', 'ana_muon_eta', 100, -5, 5 )
        hmuonphi = ROOT.TH1F( 'ana_muon_phi', 'ana_muon_phi', 70, -3.5, 3.5 )

        hnphoton = ROOT.TH1F( 'ana_nphoton', 'ana_nphoton', 20, 0, 20 )
        hphotonpt = ROOT.TH1F( 'ana_photon_pt', 'ana_photon_pt', 200, 0, 200 )
        hphotoneta = ROOT.TH1F( 'ana_photon_eta', 'ana_photon_eta', 100, -5, 5 )
        hphotonphi = ROOT.TH1F( 'ana_photon_phi', 'ana_photon_phi', 70, -3.5, 3.5 )

        hnjet = ROOT.TH1F( 'ana_njet', 'ana_njet', 20, 0, 20 )
        hjetpt = ROOT.TH1F( 'ana_jet_pt', 'ana_jet_pt', 200, 0, 200 )
        hjeteta = ROOT.TH1F( 'ana_jet_eta', 'ana_jet_eta', 100, -5, 5 )
        hjetphi = ROOT.TH1F( 'ana_jet_phi', 'ana_jet_phi', 70, -3.5, 3.5 )

    # Loop over events
    for i in range(0, evt.getEntries()):
        evt.getEntry(i)

        if phys:
            clusters = safeRetrieve(
                evt, "xAOD::CaloClusterContainer", "CaloCalTopoClusters")
            nclus = len(clusters)
            hphys_nclus.Fill( nclus )

            idTracks = safeRetrieve(evt,
                                    "xAOD::TrackParticleContainer", "InDetTrackParticles")
            nIdTracks = len(idTracks)
            hphys_nidtracks.Fill( nIdTracks )

            tautracks = safeRetrieve(evt, "xAOD::TauTrackContainer", "TauTracks")
            nTauTracks = len(tautracks)
            hphys_ntautracks.Fill( nTauTracks )

            taus = safeRetrieve(evt, "xAOD::TauJetContainer", "TauJets")
            nTaus = len(taus)
            hphys_ntaus.Fill( nTaus )

            muons = safeRetrieve(evt, "xAOD::MuonContainer", "Muons")
            nMuons = len(muons)
            hphys_nmuons.Fill( nMuons )

            electrons = safeRetrieve(evt, "xAOD::ElectronContainer", "Electrons")
            nElec = len(electrons)
            hphys_nelecs.Fill( nElec )

            photons = safeRetrieve(evt, "xAOD::PhotonContainer", "Photons")
            nPhot = len(photons)
            hphys_nphotons.Fill( nPhot )

            jets = safeRetrieve(evt,"xAOD::JetContainer", "AntiKt4EMPFlowJets")
            nJet = len(jets)
            hphys_njets.Fill( nJet )

            nTrueElectrons = 0
            nTruePhotons = 0
            acc = ROOT.SG.ConstAccessor(
              'ElementLink< xAOD::TruthParticleContainer>')('truthParticleLink')

            if nElec > 0 and acc.isAvailable(electrons.at(0)):
                for i in range(nElec):
                    truthLink = acc(electrons.at(i))
                    if(truthLink.isValid()):
                        pdgId = truthLink.pdgId()
                        if abs(pdgId) == 11:
                            nTrueElectrons += 1

            if nPhot > 0 and acc.isAvailable(photons.at(0)):
                for i in range(nPhot):
                    truthLink = acc(photons.at(i))
                    if(truthLink.isValid()):
                        pdgId = truthLink.pdgId()
                        if (pdgId == 22):
                            nTruePhotons += 1

            nFakeElectrons = nElec - nTrueElectrons
            nFakePhotons = nPhot - nTruePhotons
            hphys_nfakeelectrons.Fill( nFakeElectrons )
            hphys_nfakephotons.Fill( nFakePhotons )

        # PHYSLITE input with Analysis containers
        if analysis:
            ana_taujets = safeRetrieve(evt,"xAOD::TauJetContainer", "AnalysisTauJets")
            ana_ntaujet = len(ana_taujets)
            hntaujet.Fill( ana_ntaujet )
            for j in ana_taujets:
                htaujetpt.Fill( j.pt()/1000. )
                htaujeteta.Fill( j.eta() )
                htaujetphi.Fill( j.phi() )

            ana_muons = safeRetrieve(evt,"xAOD::MuonContainer", "AnalysisMuons")
            ana_nmuon = len(ana_muons)
            hnmuon.Fill( ana_nmuon )
            for j in ana_muons:
                hmuonpt.Fill( j.pt()/1000. )
                hmuoneta.Fill( j.eta() )
                hmuonphi.Fill( j.phi() )

            ana_electrons = safeRetrieve(evt,"xAOD::ElectronContainer", "AnalysisElectrons")
            ana_nelectron = len(ana_electrons)
            hnelectron.Fill( ana_nelectron )
            for j in ana_electrons:
                helectronpt.Fill( j.pt()/1000. )
                helectroneta.Fill( j.eta() )
                helectronphi.Fill( j.phi() )

            ana_photons = safeRetrieve(evt,"xAOD::PhotonContainer", "AnalysisPhotons")
            ana_nPhot = len(ana_photons)
            hnphoton.Fill( ana_nPhot )
            for j in ana_photons:
                hphotonpt.Fill( j.pt()/1000. )
                hphotoneta.Fill( j.eta() )
                hphotonphi.Fill( j.phi() )

            ana_jets = safeRetrieve(evt,"xAOD::JetContainer", "AnalysisJets")
            ana_nJet = len(ana_jets)
            hnjet.Fill( ana_nJet )
            for j in ana_jets:
                hjetpt.Fill( j.pt()/1000. )
                hjeteta.Fill( j.eta() )
                hjetphi.Fill( j.phi() )
    
    # Store HIST in output file in different directories
    if phys:
        hfile.cd()
        hfile.mkdir("PHYS")
        hfile.cd("PHYS")
        hphys_nclus.Write()
        hphys_nidtracks.Write()
        hphys_ntautracks.Write()
        hphys_ntaus.Write()
        hphys_nmuons.Write()
        hphys_nelecs.Write()
        hphys_nphotons.Write()
        hphys_njets.Write()
        hphys_nfakeelectrons.Write()
        hphys_nfakephotons.Write()

    if analysis:
        hfile.cd()
        hfile.mkdir("AnalysisJets")
        hfile.cd("AnalysisJets")
        hnjet.Write()
        hjetpt.Write()
        hjeteta.Write()
        hjetphi.Write()

        hfile.cd("..")
        hfile.mkdir("AnalysisPhotons")
        hfile.cd("AnalysisPhotons")
        hnphoton.Write()
        hphotonpt.Write()
        hphotoneta.Write()
        hphotonphi.Write()

        hfile.cd("..")
        hfile.mkdir("AnalysisTauJets")
        hfile.cd("AnalysisTauJets")
        hntaujet.Write()
        htaujetpt.Write()
        htaujeteta.Write()
        htaujetphi.Write()

        hfile.cd("..")
        hfile.mkdir("AnalysisElectrons")
        hfile.cd("AnalysisElectrons")
        hnelectron.Write()
        helectronpt.Write()
        helectroneta.Write()
        helectronphi.Write()

        hfile.cd("..")
        hfile.mkdir("AnalysisMuons")
        hfile.cd("AnalysisMuons")
        hnmuon.Write()
        hmuonpt.Write()
        hmuoneta.Write()
        hmuonphi.Write()

    hfile.Close()

    return 

def main():
    parser = argparse.ArgumentParser(
        description="Extracts a few basic quantities from the xAOD file and dumps them into a hist ROOT file")
    parser.add_argument("xAODFile", nargs='?', type=str,
                        help="xAOD filename", action="store")
    parser.add_argument("--phys", help="Create histogram file from DAOD_PHYS or AOD variables",
                        action="store_true", default=False)
    parser.add_argument("--analysis", help="Create histogram file from DAOD_PHYSLITE n/pt/eta/phi variables",
                        action="store_true", default=False)
    parser.add_argument("--outputHISTFile", help="histogram output filename",
                        action="store", default=None)
    parser.add_argument("--inputisESD", help="Set if input is ESD",
                        action="store_true", default=False)

    args = parser.parse_args()

    if len(sys.argv) < 2:
        parser.print_help()
        sys.exit(1)

    # Check input file existance
    if not os.access(args.xAODFile, os.R_OK):
        print("ERROR, can't access file {}".format(args.xAODFile))
        sys.exit(1)

    # Create single inputfile
    filelist = args.xAODFile

    # Setup TEvent object and add inputs
    evt = ROOT.POOL.TEvent(
        ROOT.POOL.TEvent.kPOOLAccess if args.inputisESD else ROOT.POOL.TEvent.kClassAccess)
    stat = evt.readFrom(filelist)
    if not stat:
        print("ERROR, failed to open file {} with POOL.TEvent".format(
            args.xAODFile))
        sys.exit(1)
        pass

    xAODHist(evt, args.phys, args.analysis, args.outputHISTFile)

    return 0

if __name__ == "__main__":

    main()
