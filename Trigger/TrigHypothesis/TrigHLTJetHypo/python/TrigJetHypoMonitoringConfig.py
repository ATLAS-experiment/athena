# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool

def TrigJetHypoToolMonitoring(flags, histPath, histFlags):
    montool = GenericMonitoringTool(flags, "MonTool", HistPath = histPath)
    # Always make these. Timing plots are 100 ms bins (expect everything in 0 bin)
    montool.defineHistogram('Et', title='Jet E_{T};E_{T} (GeV)', xbins=100, xmin=0, xmax=500, path='EXPERT', type='TH1F' )
    montool.defineHistogram('TIME_jetHypo,NJetsIn', title='JetHypo time vs input jets;time (ms) ;N(jets)', xbins=50, xmin=0, xmax=5000, ybins=60, ymin=0, ymax=120, path='EXPERT', type='TH2F' )
    montool.defineHistogram('TIME_jetHypo,NJetsOut', title='JetHypo time vs jets;time (ms) ;N(jets)', xbins=50, xmin=0, xmax=5000, ybins=30, ymin=-0.5, ymax=29.5, path='EXPERT', type='TH2F' )
    montool.defineHistogram('NJetsIn', title='Jet multiplicity input;N(jets)', xbins=30, xmin=-0.5, xmax=29.5, path='EXPERT', type='TH1F' )   
    montool.defineHistogram('NJetsOut', title='Jet multiplicity output;N(jets)', xbins=30, xmin=-0.5, xmax=29.5, path='EXPERT', type='TH1F' )       
    # Conditional histograms: monitor the mass for largeR jets (anything but a4), and etaphi for simple smallR
    if 'a4' not in histFlags:  montool.defineHistogram('Mass', title='Jet mass;m (GeV)', xbins=100, xmin=0, xmax=200, path='EXPERT', type='TH1F' )
    if ('simple' in histFlags) and ('a4' in histFlags) and all("HT" not in flag for flag in histFlags): 
        montool.defineHistogram('Eta,Phi', title='Jet #eta vs #phi;#eta;#phi', xbins=40, xmin=-5, xmax=5, ybins=25, ymin=-3.145, ymax=3.145, path='EXPERT', type='TH2F' )
    # Conditional histograms: hypoScenarios like HTXXXX will match.
    if any("HT" in flag for flag in histFlags) : 
        montool.defineHistogram('HT', title='Event H_{T};H_{T} (GeV)', xbins=100, xmin=0, xmax=3000, path='EXPERT', type='TH1F' )
        montool.defineHistogram('NJets', title='Jet multiplicity;N(jets)', xbins=20, xmin=-0.5, xmax=19.5, path='EXPERT', type='TH1F' )   
    if any("Z" in flag for flag in histFlags) : 
        montool.defineHistogram('dipz_z', title='DIPZ z;z (mm)', xbins=100, xmin=-50, xmax=50, path='EXPERT', type='TH1F' )
        montool.defineHistogram('dipz_negLogSigma2', title='DIPZ negLogSigma2;negLogSigma2', xbins=100, xmin=-20, xmax=5, path='EXPERT', type='TH1F' )
        montool.defineHistogram('NJets', title='Jet multiplicity;N(jets)', xbins=30, xmin=-0.5, xmax=29.5, path='EXPERT', type='TH1F' )   
    if ('a10sd' in histFlags):
        montool.defineHistogram("GN2Xv01_phbb", title='GN2X_phbb; p_{Hbb}', xbins=50, xmin=0, xmax=1, path='EXPERT', type='TH1F')
        montool.defineHistogram("GN2Xv01_phcc", title='GN2X_phcc; p_{Hcc}', xbins=50, xmin=0, xmax=1, path='EXPERT', type='TH1F')
        montool.defineHistogram("GN2Xv01_ptop", title='GN2X_ptop; p_{top}', xbins=50, xmin=0, xmax=1, path='EXPERT', type='TH1F')
        montool.defineHistogram("GN2Xv01_pqcd", title='GN2X_pqcd; p_{QCD}', xbins=50, xmin=0, xmax=1, path='EXPERT', type='TH1F')
        montool.defineHistogram("GN2Xv01_discriminant", title='GN2Xv01 discriminant (fcc=0, ftop=0.25); GN2Xv01 discriminant', xbins=50, xmin=-30, xmax=30, path='EXPERT', type='TH1F')
        montool.defineHistogram("GN2XTrig_phbb", title='GN2XTrig_phbb; p_{Hbb}', xbins=50, xmin=0, xmax=1, path='EXPERT', type='TH1F')
        montool.defineHistogram("GN2XTrig_ptop", title='GN2XTrig_ptop; p_{top}', xbins=50, xmin=0, xmax=1, path='EXPERT', type='TH1F')
        montool.defineHistogram("GN2XTrig_pqcd", title='GN2XTrig_pqcd; p_{QCD}', xbins=50, xmin=0, xmax=1, path='EXPERT', type='TH1F')
        montool.defineHistogram("GN2XTrig_discriminant", title='GN2XTrig discriminant (ftop=0.25); GN2XTrig discriminant', xbins=50, xmin=-30, xmax=30, path='EXPERT', type='TH1F')
    return montool


def TrigJetCRHypoToolMonitoring(histPath):
    montool = GenericMonitoringTool(None,"MonTool", HistPath = histPath)
    default_bin_count = 100
    montool.defineHistogram('jet_pt', title="Jet Transverse Momentum;p_{T} (GeV)", xbins = default_bin_count, xmin=0, xmax=1000, path='EXPERT', type='TH1F')
    montool.defineHistogram('jet_eta', title="Jet #eta;#eta", xbins = default_bin_count, xmin=-5, xmax=5, path='EXPERT', type='TH1F')
    montool.defineHistogram('jet_phi', title="Jet #phi;#phi", xbins = default_bin_count, xmin=-3.5, xmax=3.5, path='EXPERT', type='TH1F')
    montool.defineHistogram('jet_emf', title="Jet emf;emf", xbins = default_bin_count, xmin=0, xmax=1, path='EXPERT', type='TH1F')
    montool.defineHistogram('jet_emf_pl', title="Jet emf pl;emf pl", xbins = default_bin_count, xmin=0, xmax=1, path='EXPERT', type='TH1F')
    montool.defineHistogram('jet_emf,jet_emf_pl', path='EXPERT', type='TH2F', title="Jet emf vs jet emf rpl",
                         xbins = default_bin_count, xmin=-0, xmax=1, ybins = default_bin_count, ymin=0, ymax=1)

    montool.defineHistogram('jet_logR_pl', title="Jet logR plp;logR plp", xbins = default_bin_count, xmin=-2, xmax=2, path='EXPERT', type='TH1F')
    montool.defineHistogram('jet_logR', title="Jet logR;logR", xbins = default_bin_count, xmin=-2, xmax=2, path='EXPERT', type='TH1F')
    montool.defineHistogram('jet_tr_pt', title="Jet track pt;tr_pt",  xbins = default_bin_count, xmin=0, xmax=1000, path='EXPERT', type='TH1F')
    montool.defineHistogram('jet_tr_DR', title="Jet track dr;track dr", xbins = default_bin_count, xmin=0, xmax=1, path='EXPERT', type='TH1F')

    return montool
