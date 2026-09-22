# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool

def tauMonitoringCaloOnlyMVA(flags: AthConfigFlags, name: str = 'CaloMVA', RoI_name: str = 'L1', jet: str = 'lc'):
    monTool = GenericMonitoringTool(flags, 'MonTool')
    monTool.HistPath = f'TrigTauRecMerged_Calo{jet.upper()}MVA'

    monTool.defineHistogram('NTauCandidates', path='EXPERT', type='TH1F', title=name+' Tau candidates; N Tau; Entries', xbins=10, xmin=-1.0, xmax=9)

    # Tau kinematics (from TauJet)
    monTool.defineHistogram('Pt', path='EXPERT', type='TH1F', title=name+' Tau E_{T}; E_{T} [GeV]; Entries', xbins=100, xmin=0, xmax=1000)
    monTool.defineHistogram('Eta', path='EXPERT', type='TH1F', title=name+' Tau #eta; #eta; Entries', xbins=100, xmin=-2.6, xmax=2.6)
    monTool.defineHistogram('Phi', path='EXPERT', type='TH1F', title=name+' Tau #phi; #phi; Entries', xbins=100, xmin=-3.2, xmax=3.2)
    monTool.defineHistogram('Eta, Phi', path='EXPERT', type='TH2F', title=name+' Tau #eta vs #phi; #eta; #phi', xbins=100, xmin=-2.6, xmax=2.6, ybins=100, ymin=-3.2, ymax=3.2)
    monTool.defineHistogram('Pt, Eta', path='EXPERT', type='TH2F', title=name+' Tau E_{T} vs #eta; E_{T} [GeV]; #eta', xbins=100 , xmin=0, xmax=1000, ybins=100, ymin=-2.6, ymax=2.6)
    monTool.defineHistogram('Pt, Phi', path='EXPERT', type='TH2F', title=name+' Tau E_{T} vs #phi; E_{T} [GeV]; #phi', xbins=100 , xmin=0, xmax=1000, ybins=100, ymin=-3.2, ymax=3.2)

    # RoI kinematics (from the step RoI)
    if RoI_name:
        monTool.defineHistogram('EtaRoI', path='EXPERT', type='TH1F', title=RoI_name+' RoI #eta; #eta; Entries', xbins=100, xmin=-2.6, xmax=2.6)
        monTool.defineHistogram('PhiRoI', path='EXPERT', type='TH1F', title=RoI_name+' RoI #phi; #phi; Entries', xbins=100, xmin=-3.2, xmax=3.2)
        monTool.defineHistogram('EtaRoI, PhiRoI', path='EXPERT', type='TH2F', title=RoI_name+' RoI #eta vs #phi; #eta; #phi', xbins=100, xmin=-2.6, xmax=2.6, ybins=100, ymin=-3.2, ymax=3.2)

        monTool.defineHistogram('dEtaTau_RoI', path='EXPERT', type='TH1F', title=name+' #Delta#eta(Tau, '+RoI_name+' RoI); #Delta#eta(Tau, '+RoI_name+' RoI); Entries', xbins=100, xmin=-0.4, xmax=0.4)
        monTool.defineHistogram('dPhiTau_RoI', path='EXPERT', type='TH1F', title=name+' #Delta#phi(Tau, '+RoI_name+' RoI); #Delta#phi(Tau, '+RoI_name+' RoI); Entries', xbins=100, xmin=-0.15, xmax=0.15)
        monTool.defineHistogram('dEtaTau_RoI, dPhiTau_RoI', path='EXPERT', type='TH2F', title=name+' #Delta#eta(Tau, '+RoI_name+' RoI) vs #Delta#phi(Tau, '+RoI_name+' RoI); #Delta#eta(Tau, '+RoI_name+' RoI); #Delta#phi(Tau, '+RoI_name+' RoI)',
                                                                xbins=100 , xmin=-0.4, xmax=0.4,
                                                                ybins=100 , ymin=-0.15, ymax=0.15)

    # TauJet reconstruction variables
    monTool.defineHistogram('mEflowApprox', path='EXPERT', type='TH1F', title=name+' TauJet Log10(max(mEflowApprox, 140)); Log10(max(mEflowApprox, 140)); Entries', xbins=50, xmin=0, xmax=5)
    monTool.defineHistogram('ptRatioEflowApprox', path='EXPERT', type='TH1F', title=name+' TauJet min(ptRatioEflowApprox, 4); min(ptRatioEflowApprox, 4); Entries', xbins=50, xmin=0, xmax=4)
    monTool.defineHistogram('pt_jetseed_log', path='EXPERT', type='TH1F', title=name+' TauJet Log10(ptJetSeed); Log10(ptJetSeed); Entries', xbins=50, xmin=3.5, xmax=7)
    monTool.defineHistogram('etaDetectorAxis', path='EXPERT', type='TH1F', title=name+' TauJet etaDetectorAxis; etaDetectorAxis', xbins=100, xmin=-2.6, xmax=2.6)
    monTool.defineHistogram('ptDetectorAxis', path='EXPERT', type='TH1F', title=name+' TauJet ptDetectorAxis; ptDetectorAxis; Entries', xbins=50, xmin=1, xmax=1000)
    monTool.defineHistogram('ptDetectorAxis_log', path='EXPERT', type='TH1F', title=name+' TauJet Log10(ptDetectorAxis); Log10(ptDetectorAxis); Entries', xbins=50, xmin=0, xmax=5)

    # TauJet low-level Calorimeter variables
    monTool.defineHistogram('NCaloCells', path='EXPERT', type='TH1F', title=name+' TauJet calorimeter cells; N Cells; Entries', xbins=100, xmin=0, xmax=6000)
    monTool.defineHistogram('EtHad, EtEm', path='EXPERT', type='TH2F', title=name+' TauJet E_{T}^{Had} vs E_{T}^{EM}; E_{T}^{Had} (at EM scale) [GeV]; E_{T}^{EM} (at EM scale) [GeV]',
                                                 xbins=30, xmin=0, xmax=150,
                                                 ybins=30, ymin=0, ymax=150)
    monTool.defineHistogram('EMFrac', path='EXPERT', type='TH1F', title=name+' TauJet EM fraction; E_{T}^{EM} / (E_{T}^{EM} + E_{T}^{Had}) (at EM scale); Entries', xbins=70, xmin=-0.1, xmax=1.3)
    monTool.defineHistogram('IsoFrac', path='EXPERT', type='TH1F', title=name+' TauJet Isolation fraction; Isolation fraction; Entries', xbins=80, xmin=-0.4, xmax=1.2)
    monTool.defineHistogram('CentFrac', path='EXPERT', type='TH1F', title=name+' TauJet Central fraction; Central fraction; Entries', xbins=80, xmin=-0.4, xmax=1.2)

    # TauJet clusters mean variables
    monTool.defineHistogram('clustersMeanCenterLambda', path='EXPERT', type='TH1F', title=name+' TauJet clustersMeanCenterLambda; clustersMeanCenterLambda; Entries', xbins=40, xmin=0, xmax=2500)
    monTool.defineHistogram('clustersMeanFirstEngDens', path='EXPERT', type='TH1F', title=name+' TauJet clustersMeanFirstEngDens; clustersMeanFirstEngDens; Entries', xbins=40, xmin=-8.7, xmax=-5.5)
    monTool.defineHistogram('clustersMeanSecondLambda', path='EXPERT', type='TH1F', title=name+' TauJet clustersMeanSecondLambda; clustersMeanSecondLambda; Entries', xbins=20, xmin=0, xmax=6e5)
    monTool.defineHistogram('clustersMeanPresamplerFrac', path='EXPERT', type='TH1F', title=name+' TauJet clustersMeanPresamplerFrac; clustersMeanPresamplerFrac; Entries', xbins=20, xmin=0, xmax=0.2)
    monTool.defineHistogram('clustersMeanEMProbability', path='EXPERT', type='TH1F', title=name+' TauJet clustersMeanEMProbability; clustersMeanEMProbability; Entries', xbins=20, xmin=0, xmax=1)

    # Cluster variables
    monTool.defineHistogram('NClusters', path='EXPERT', type='TH1F', title=name+' TauJet calorimeter clusters; N Clusters; Entries', xbins=15, xmin=0, xmax=15)
    monTool.defineHistogram('cluster_et_log', path='EXPERT', type='TH1F', title=name+' TauJet Log10(Cluster E_{T}); Log10(Cluster E_{T}); Entries', xbins=50, xmin=1, xmax=7) 
    monTool.defineHistogram('cluster_dEta', path='EXPERT', type='TH1F', title=name+' TauJet #Delta#eta(Cluster, Tau); #Delta#eta(Cluster, Tau); Entries', xbins=50, xmin=-0.5, xmax=0.5)
    monTool.defineHistogram('cluster_dPhi', path='EXPERT', type='TH1F', title=name+' TauJet #Delta#phi(Cluster, Tau); #Delta#phi(Cluster, Tau); Entries', xbins=50, xmin=-0.5, xmax=0.5)
    monTool.defineHistogram('cluster_log_SECOND_R', path='EXPERT',type='TH1F', title=name+' TauJet Log10(cluster_SECOND_R); Log10(cluster_SECOND_R); Entries', xbins=50, xmin=-3, xmax=7)
    monTool.defineHistogram('cluster_SECOND_LAMBDA', path='EXPERT',type='TH1F', title=name+' TauJet Log10(cluster_SECOND_LAMBDA); Log10(cluster_SECOND_LAMBDA); Entries', xbins=50, xmin=-3, xmax=7)
    monTool.defineHistogram('cluster_CENTER_LAMBDA', path='EXPERT',type='TH1F', title=name+' TauJet Log10(cluster_CENTER_LAMBDA); Log10(cluster_CENTER_LAMBDA); Entries', xbins=50, xmin=-2, xmax=5)

    labels = ['NoROIDescr', 'NoCellCont', 'EmptyCellCont', 'NoClustCont', 'NoClustKey', 'EmptyClustCont', 'NoJetAttach', 'NoHLTtauAttach', 'NoHLTtauDetAttach', 'NoHLTtauXdetAttach']
    monTool.defineHistogram('calo_errors', path='EXPERT', type='TH1F', title=name+' TauJet Calo Reco Errors; Error; Entries', xbins=10, xmin=-0.5, xmax=9.5, xlabels=labels)

    return monTool


def tauMonitoringCaloHits(flags: AthConfigFlags, name: str, hitz_algs: list[str], tau_ids: list[str], jet: str = 'lc'):
    monTool = tauMonitoringCaloOnlyMVA(flags, name, RoI_name='', jet=jet)
    monTool.HistPath = f'TrigTauRecMerged_{name}'

    # HitZ input variables
    monTool.defineHistogram('pt_jetseed', path='EXPERT', type='TH1F', title=name+' TauJet p_{T}^{jet seed}; p_{T}^{jet seed} [GeV]; Entries', xbins=50, xmin=0, xmax=1000)
    monTool.defineHistogram('eta_jetseed', path='EXPERT', type='TH1F', title=name+' TauJet #eta_{jet seed}; #eta_{jet seed}; Entries', xbins=50, xmin=-2.5, xmax=2.5)

    # TauJet hits variables
    monTool.defineHistogram('NHits', path='EXPERT', type='TH1F', title=name+' TauJet associated hits; N Hits; Entries', xbins=30, xmin=0, xmax=300)
    monTool.defineHistogram('hit_z', path='EXPERT', type='TH1F', title=name+' TauJet associated hits z; Hit z; Entries', xbins=40, xmin=-200, xmax=200)
    monTool.defineHistogram('hit_dPhi', path='EXPERT', type='TH1F', title=name+' TauJet #Delta#phi(Hit, Tau); #Delta#phi(Hit, Tau); Entries', xbins=50, xmin=-0.5, xmax=0.5)
    # We could add more histograms here for the hit properties (e.g. hit layers, hit types, etc...), but given the
    # large number of hits per TauJet (O(100)), this could lead to a non-negligible time overhead.

    for hitz_alg in hitz_algs:
        monTool.defineHistogram(f'{hitz_alg}_z0', path='EXPERT', type='TH1F', title=f'{name} TauJet {hitz_alg} regression z_{{0}}; z_{{0}} [mm]; Entries', xbins=60, xmin=-185, xmax=185)
        monTool.defineHistogram(f'{hitz_alg}_z0_sigma', path='EXPERT', type='TH1F', title=f'{name} TauJet {hitz_alg} regression #sigma_{{z}}; #sigma_{{z}} [mm]; Entries', xbins=60, xmin=0, xmax=40)

    for tau_id in tau_ids:
        if getattr(flags.Trigger.Offline.Tau, tau_id).OutputDiscriminant == 1: xmax = 1
        else: xmax = 5

        monTool.defineHistogram(f'{tau_id}_TauJetScore;{tau_id}_TauJetScore_0p', path='EXPERT', type='TH1F', title=f'{name} TauJet {tau_id} Tau ID score; Score; Entries', xbins=100, xmin=0, xmax=xmax)
        monTool.defineHistogram(f'{tau_id}_TauJetScoreTrans;{tau_id}_TauJetScoreTrans_0p', path='EXPERT', type='TH1F', title=f'{name} TauJet {tau_id} Tau ID transformed score; Transformed Signal Score; Entries', xbins=100, xmin=0, xmax=1)

    return monTool



def tauMonitoringPrecision(flags: AthConfigFlags, name: str = 'Precision', RoI_name: str = 'tauIso', tau_ids: list[str] = [], alg_name: str = ''):
    monTool = tauMonitoringCaloOnlyMVA(flags, name, RoI_name)
    monTool.HistPath = f'TrigTauRecMerged_{name}' + (f'_{alg_name}' if alg_name else '')

    # TauJet track variables
    monTool.defineHistogram('NTracks', path='EXPERT', type='TH1F', title=name+' TauJet core tracks; N Tracks; Entries', xbins=17, xmin=-2.0, xmax=15)
    monTool.defineHistogram('NIsoTracks', path='EXPERT', type='TH1F', title=name+' TauJet wide tracks; N Tracks; Entries', xbins=17, xmin=-2.0, xmax=15)

    monTool.defineHistogram('ipSigLeadTrk', path='EXPERT', type='TH1F', title=name+' TauJet leading track IPsig; Leading Track IPsig; Entries', xbins=100, xmin=-50, xmax=50)
    monTool.defineHistogram('trFlightPathSig', path='EXPERT', type='TH1F', title=name+' TauJet track FlightPath sig.; Track FlightPath sig.; Entries', xbins=100, xmin=-20, xmax=40)
    monTool.defineHistogram('massTrkSys', path='EXPERT', type='TH1F', title=name+' TauJet track system mass; m_{trk} [GeV]; Entries', xbins=100, xmin=0, xmax=50)
    monTool.defineHistogram('dRmax', path='EXPERT', type='TH1F', title=name+' TauJet Max #DeltaR(Tau, Track); Max #DeltaR(Tau, Track); Entries', xbins=50, xmin=0, xmax=0.25)
    monTool.defineHistogram('TrkAvgDist', path='EXPERT', type='TH1F', title=name+' TauJet avg. #DeltaR(Tau, Track) (p_{T} weighted); Avg. #DeltaR(Tau, Track) (p_{T} weighted); Entries', xbins=41, xmin=-0.01, xmax=0.4)
    monTool.defineHistogram('innerTrkAvgDist', path='EXPERT', type='TH1F', title=name+' TauJet avg. #DeltaR(Tau, Core Track) (p_{T} weighted); Avg. #DeltaR(Tau, Core Track) (p_{T} weighted); Entries', xbins=40, xmin=-0.05, xmax=0.5)
    monTool.defineHistogram('EtovPtLead', path='EXPERT', type='TH1F', title=name+' TauJet (E_{T}^{EM} + E_{T}^{Had}) / p_{T}^{lead trk.}; (E_{T}^{EM} + E_{T}^{Had}) / p_{T}^{lead trk.}; Entries', xbins=41, xmin=-0.5, xmax=20.0)
    monTool.defineHistogram('PSSFraction', path='EXPERT', type='TH1F', title=name+' TauJet PreSampler Strip E_{T} / E_{T}; E_{T}^{presampler strip} / E_{T}; Entries', xbins=50, xmin=-0.5, xmax=1)
    monTool.defineHistogram('EMPOverTrkSysP', path='EXPERT', type='TH1F', title=name+' TauJet E_{T}^{EM} over Track system p_{T}; E_{T}^{EM} / p_{T}^{trk sys}; Entries', xbins=41, xmin=-0.5, xmax=20.0)
    monTool.defineHistogram('ChPiEMEOverCaloEME', path='EXPERT', type='TH1F', title=name+' TauJet E_{T} of #pi^{#pm} over E_{T}^{EM}; (p_{T}^{trk sys} - E_{T}^{Had}) / E_{T}^{EM}; Entries', xbins=40, xmin=-20, xmax=20)

    monTool.defineHistogram('vertex_x', path='EXPERT', type='TH1F', title=name+' TauJet Vertex x; Vertex x; Entries', xbins=100, xmin=-1, xmax=1)
    monTool.defineHistogram('vertex_y', path='EXPERT', type='TH1F', title=name+' TauJet Vertex y; Vertex y; Entries', xbins=100, xmin=-2, xmax=0)
    monTool.defineHistogram('vertex_z', path='EXPERT', type='TH1F', title=name+' TauJet Vertex z; Vertex z; Entries', xbins=120, xmin=-120, xmax=120)

    # TauJet Track variables
    monTool.defineHistogram('NAllTracks', path='EXPERT', type='TH1F', title=name+' TauJet total tracks; N Tracks; Entries', xbins=20, xmin=0, xmax=20)
    monTool.defineHistogram('track_pt_log', path='EXPERT', type='TH1F', title=name+' TauJet Log10(Track p_{T}); Log10(Track p_{T}); Entries', xbins=50, xmin=2.7, xmax=7)
    monTool.defineHistogram('track_dEta', path='EXPERT', type='TH1F', title=name+' TauJet #Delta#eta(Track, Tau); #Delta#eta(Track, Tau); Entries', xbins=50, xmin=-0.5, xmax=0.5)
    monTool.defineHistogram('track_dPhi', path='EXPERT', type='TH1F', title=name+' TauJet #Delta#phi(Track, Tau); #Delta#phi(Track, Tau); Entries', xbins=50, xmin=-0.5, xmax=0.5)
    monTool.defineHistogram('track_d0_abs_log', path='EXPERT',type='TH1F', title=name+' TauJet Log10(|Track d_{0}| + 1e-6); Log10(|Track d_{0}| + 1e-6); Entries', xbins=50, xmin=-6.1, xmax=2)
    monTool.defineHistogram('track_z0sinthetaTJVA_abs_log', path='EXPERT', type='TH1F', title=name+' TauJet Track Log10(|z_{0}^{TJVA} sin(#theta)| + 1e-6); Log10(|z_{0}^{TJVA} sin(#theta)| + 1e-6); Entries', xbins=50, xmin=-6.1, xmax=4)
    monTool.defineHistogram('track_nPixelHitsPlusDeadSensors', path='EXPERT', type='TH1F', title=name+' TauJet Track Pixel hits + Pixel dead sensors; N Pixel hits + N Pixel dead sensors; Entries', xbins=11, xmin=0, xmax=11)
    monTool.defineHistogram('track_nSCTHitsPlusDeadSensors', path='EXPERT', type='TH1F', title=name+' TauJet Track SCT hits + SCT dead sensors; N SCT hits + N SCT dead sensors; Entries', xbins=20, xmin=0, xmax=20)

    labels = ['NoTrkCont', 'NoVtxCont']
    monTool.defineHistogram('track_errors', path='EXPERT', type='TH1F', title=name+' TauJet Tracking Errors; Error; Entries', xbins=2, xmin=-0.5, xmax=1.5, xlabels=labels)

    # TauJet hits variables
    monTool.defineHistogram('NHits', path='EXPERT', type='TH1F', title=name+' TauJet associated hits; N Hits; Entries', xbins=30, xmin=0, xmax=300)
    monTool.defineHistogram('hit_z', path='EXPERT', type='TH1F', title=name+' TauJet associated hits z; Hit z; Entries', xbins=40, xmin=-200, xmax=200)
    monTool.defineHistogram('hit_dPhi', path='EXPERT', type='TH1F', title=name+' TauJet #Delta#phi(Hit, Tau); #Delta#phi(Hit, Tau); Entries', xbins=50, xmin=-0.5, xmax=0.5)

    for tau_id in tau_ids:
        tau_id_flags = getattr(flags.Trigger.Offline.Tau, tau_id)
        if hasattr(tau_id_flags, 'NetworkConfig') or tau_id_flags.OutputDiscriminant == 1: xbins, xmax = 40, 1
        else: xbins, xmax = 100, 5

        monTool.defineHistogram(f'{tau_id}_TauJetScore_0p', path='EXPERT', type='TH1F', title=f'{name} 0-prong TauJet {tau_id} Tau ID score; Score; Entries', xbins=xbins, xmin=0, xmax=xmax)
        monTool.defineHistogram(f'{tau_id}_TauJetScoreTrans_0p', path='EXPERT', type='TH1F', title=f'{name} 0-prong TauJet {tau_id} Tau ID transformed score; Transformed Signal Score; Entries', xbins=xbins, xmin=0, xmax=xmax)

        monTool.defineHistogram(f'{tau_id}_TauJetScore_1p', path='EXPERT', type='TH1F', title=f'{name} 1-prong TauJet {tau_id} Tau ID score; Score; Entries', xbins=xbins, xmin=0, xmax=xmax)
        monTool.defineHistogram(f'{tau_id}_TauJetScoreTrans_1p', path='EXPERT', type='TH1F', title=f'{name} 1-prong TauJet {tau_id} Tau ID transformed score; Transformed Signal Score; Entries', xbins=xbins, xmin=0, xmax=xmax)

        monTool.defineHistogram(f'{tau_id}_TauJetScore_mp', path='EXPERT', type='TH1F', title=f'{name} multi-prong TauJet {tau_id} Tau ID score; Score; Entries', xbins=xbins, xmin=0, xmax=xmax)
        monTool.defineHistogram(f'{tau_id}_TauJetScoreTrans_mp', path='EXPERT', type='TH1F', title=f'{name} multi-prong TauJet {tau_id} Tau ID transformed score; Transformed Signal Score; Entries', xbins=xbins, xmin=0, xmax=xmax)

    return monTool
