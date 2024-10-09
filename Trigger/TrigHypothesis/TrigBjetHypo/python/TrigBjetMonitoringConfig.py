# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool

def TrigBjetBtagHypoToolMonitoring(flags, histPath):
    montool = GenericMonitoringTool(flags, "MonTool", HistPath = histPath)
    montool.defineHistogram('btag_pb', title=': Probability jets are B-jets', xbins=100, xmin=0, xmax=1, path='EXPERT', type='TH1F' )
    montool.defineHistogram('btag_pc', title=': Probability jets are Charm-jets', xbins=100, xmin=0, xmax=1, path='EXPERT', type='TH1F' )
    montool.defineHistogram('btag_pu', title=': Probability jets are Light-jets', xbins=100, xmin=0, xmax=1, path='EXPERT', type='TH1F' )
    montool.defineHistogram('btag_llr', title=': Log(P_{b}/P_{light}), Likelihood Ratio between the B-jet and Light-flavour Jet Hypotheses', xbins=100, xmin=-10, xmax=50, path='EXPERT', type='TH1F' )
    # hypo stage histogram
    montool.defineHistogram(
        'stage',
        title="Bjet hypothesis result",
        xbins=4,
        xlabels=['no beamspot', 'no primary vertex', 'fail', 'pass'],
        path='EXPERT',
    )

    return montool


def TrigBjetOnlineMonitoring(flags, name="TrigBjetOnlineMonitoring"):

    def make_flavor_hists(montool, tagger):
        montool.defineHistogram('btag_'+tagger+'_pb', title=tagger+': Probability jets are B-jets', type='TH1F', path='EXPERT', xbins=100, xmin=0, xmax=1)
        montool.defineHistogram('btag_'+tagger+'_pc', title=tagger+': Probability jets are Charm-jets', type='TH1F', path='EXPERT', xbins=100, xmin=0, xmax=1)
        montool.defineHistogram('btag_'+tagger+'_pu', title=tagger+': Probability jets are Light-jets', type='TH1F', path='EXPERT', xbins=100, xmin=0, xmax=1)
        montool.defineHistogram('btag_'+tagger+'_llr', title=tagger+': Log(P_{b}/P_{light}), Likelihood Ratio between the B-jet and Light-flavour Jet Hypotheses', type='TH1F', path='EXPERT', xbins=100, xmin=-10, xmax=50)

    def make_b_flavor_hists(montool, tagger):
        montool.defineHistogram('bbtag_'+tagger+'_pb', title=tagger+': Probability jets are B-jets', type='TH1F', path='EXPERT', xbins=100, xmin=0, xmax=1)
        montool.defineHistogram('bbtag_'+tagger+'_pbb', title=tagger+': Probability jets are BB-jets', type='TH1F', path='EXPERT', xbins=100, xmin=0, xmax=1)


    montool = GenericMonitoringTool(flags, name, HistPath = name)
    default_bin_count = 100

        # Event Histograms
    montool.defineHistogram('track_count', title="Number of Tracks per Trigger Decision", xbins = 100, xmin=0, xmax=100, path='EXPERT', type='TH1I')
    montool.defineHistogram('jet_count', title="Number of Jets Considered for B-Tagging", xbins = 20, xmin=0, xmax=20, path='EXPERT', type='TH1I')
    montool.defineHistogram('vertex_count', title="Number of Primary Vertex Candidates per Event", xbins = 200, xmin=0, xmax=200, path='EXPERT', type='TH1I')

    # Primary Vertex Histogram
    montool.defineHistogram('primVtx_x', title="Primary Vertex X", xbins = default_bin_count, xmin=-2, xmax=2, path='EXPERT', type='TH1F')
    montool.defineHistogram('primVtx_y', title="Primary Vertex Y", xbins = default_bin_count, xmin=-2, xmax=2, path='EXPERT', type='TH1F')
    montool.defineHistogram('primVtx_z', title="Primary Vertex Z", xbins = default_bin_count, xmin=-300, xmax=300, path='EXPERT', type='TH1F')

    ## Track Histograms
    montool.defineHistogram('track_Et', title="Track Transverse Energy;E_{T} (GeV)", xbins = default_bin_count, xmin=0, xmax=200, path='EXPERT', type='TH1F')
    montool.defineHistogram('track_eta', title="Track #eta;#eta", xbins = default_bin_count, xmin=-5, xmax=5, path='EXPERT', type='TH1F')
    montool.defineHistogram('track_phi', title="Track #phi;#phi", xbins = default_bin_count, xmin=-3.5, xmax=3.5, path='EXPERT', type='TH1F')
    montool.defineHistogram('track_eta,track_phi', path='EXPERT', type='TH2F', title="Track #eta vs #phi;#eta;#phi",
                         xbins = default_bin_count, xmin=-5, xmax=5, ybins = default_bin_count, ymin=-4, ymax=4)
    montool.defineHistogram('track_d0', title="Track d_{0};d_{0} (mm)", xbins = default_bin_count, xmin=-5, xmax=5, path='EXPERT', type='TH1F')
    montool.defineHistogram('track_d0err', title="Track d_{0} Error;d_{0} Error (mm)", xbins = default_bin_count, xmin=0, xmax=10, path='EXPERT', type='TH1F')
    montool.defineHistogram('track_d0sig', title="Track d_{0} Significance;d_{0} #sigma", xbins = default_bin_count, xmin=-100, xmax=100, path='EXPERT', type='TH1F')
    montool.defineHistogram('track_z0', title="Track z_{0};z_{0} (mm)", xbins = default_bin_count, xmin=-200, xmax=200, path='EXPERT', type='TH1F')
    montool.defineHistogram('track_z0err', title="Track z_{0} Error;z_{0} Error (mm)", xbins = default_bin_count, xmin=0, xmax=10, path='EXPERT', type='TH1F')
    montool.defineHistogram('track_z0sig', title="Track z_{0} Significance;z_{0} #sigma", xbins = default_bin_count, xmin=-1000, xmax=1000, path='EXPERT', type='TH1F')

    # Jet Histograms
    montool.defineHistogram('jet_pt', title="Jet Transverse Momentum;p_{T} (GeV)", xbins = default_bin_count, xmin=0, xmax=1000, path='EXPERT', type='TH1F')
    montool.defineHistogram('jet_eta', title="Jet #eta;#eta", xbins = default_bin_count, xmin=-5, xmax=5, path='EXPERT', type='TH1F')
    montool.defineHistogram('jet_phi', title="Jet #phi;#phi", xbins = default_bin_count, xmin=-3.5, xmax=3.5, path='EXPERT', type='TH1F')
    montool.defineHistogram('jet_eta,jet_phi', path='EXPERT', type='TH2F', title="Jet #eta vs #phi;#eta;#phi",
                         xbins = default_bin_count, xmin=-5, xmax=5, ybins = default_bin_count, ymin=-4, ymax=4)

    montool.defineHistogram('Bjet_pt', title="B-Tagged Jet Transverse Momentum;p_{T} (GeV)", xbins = default_bin_count, xmin=0, xmax=1000, path='EXPERT', type='TH1F')
    montool.defineHistogram('Bjet_eta', title="B-Tagged Jet #eta;#eta", xbins = default_bin_count, xmin=-5, xmax=5, path='EXPERT', type='TH1F')
    montool.defineHistogram('Bjet_phi', title="B-Tagged Jet #phi;#phi", xbins = default_bin_count, xmin=-3.5, xmax=3.5, path='EXPERT', type='TH1F')
    montool.defineHistogram('Bjet_eta,Bjet_phi', path='EXPERT', type='TH2F', title="B-Tagged Jet #eta vs #phi;#eta;#phi",
                         xbins = default_bin_count, xmin=-5, xmax=5, ybins = default_bin_count, ymin=-4, ymax=4)

    montool.defineHistogram('jet_bjet_delta_pt', title="Transverse Momentum Difference Between Jet and B-Tagged Jet;#Delta p_{T} (GeV)", xbins = default_bin_count, xmin=0, xmax=1000, path='EXPERT', type='TH1F')
    montool.defineHistogram('jet_bjet_delta_eta', title="#eta Difference Between Jet and B-Tagged Jet;#Delta #eta", xbins = default_bin_count, xmin=-5, xmax=5, path='EXPERT', type='TH1F')
    montool.defineHistogram('jet_bjet_delta_phi', title="#phi Difference Between Jet and B-Tagged Jet;#phi", xbins = default_bin_count, xmin=-3.5, xmax=3.5, path='EXPERT', type='TH1F')
    montool.defineHistogram('jet_bjet_delta_eta,jet_phi', path='EXPERT', type='TH2F', title="#Delta #eta vs #Delta #phi for Jet and B-Tagged Jet;#Delta #eta;#Delta #phi",
                         xbins = default_bin_count, xmin=-5, xmax=5, ybins = default_bin_count, ymin=-4, ymax=4)


    # B-Tagging Histograms
    for tagger in ['DL1d20211216', 'dips20211116', 'GN120220813','GN220240122']: make_flavor_hists(montool, tagger)
    for tagger in []: make_b_flavor_hists(montool, tagger)


    return montool
