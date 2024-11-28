# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool

def getTrigTauPrecisionIDHypoToolMonitoring(flags, name: str, tau_ids: list[str]):
    monTool = GenericMonitoringTool(flags, f'MonTool_{name}')
    monTool.HistPath = f'TrigTauRecMerged_TrigTauPrecisionIDHypo/{name}'

    # Define quantities to be monitored
    monTool.defineHistogram('NInputTaus', path='EXPERT', type='TH1F', title='Input Taus (before selection); N Taus; Entries', xbins=10, xmin=0, xmax=10)

    labels = ['Initial', 'p_{T}', 'NTracks & NIsoTracks', 'ID']
    monTool.defineHistogram('CutCounter', path='EXPERT', type='TH1I', title='Passed Tau cuts; Cut; Entries', xbins=len(labels), xmin=0, xmax=len(labels), xlabels=labels)

    monTool.defineHistogram('PtAccepted', path='EXPERT', type='TH1F', title='Accepted Tau p_{T}; p_{T} [GeV]; Entries', xbins=80, xmin=0, xmax=800)
    monTool.defineHistogram('NTracksAccepted', path='EXPERT', type='TH1F', title='Accepted Tau Tracks; N Tracks; Entries', xbins=10, xmin=0, xmax=10)
    monTool.defineHistogram('NIsoTracksAccepted', path='EXPERT', type='TH1F', title='Accepted Tau Isolation Tracks; N Isolation Tracks; Entries', xbins=10, xmin=0, xmax=10)

    for tau_id in tau_ids:
        if tau_id in ['DeepSet', 'RNNLLP']: xbins, xmax = 40, 1
        else: xbins, xmax = 100, 5

        monTool.defineHistogram(f'{tau_id}_TauJetScoreAccepted_0p', path='EXPERT', type='TH1F', title=f'Accepted 0-prong {tau_id} Tau ID score; Score; Entries', xbins=xbins, xmin=0, xmax=xmax)
        monTool.defineHistogram(f'{tau_id}_TauJetScoreTransAccepted_0p', path='EXPERT', type='TH1F', title=f'Accepted 0-prong {tau_id} Tau ID transformed score; Transformed Signal Score; Entries', xbins=xbins, xmin=0, xmax=xmax)

        monTool.defineHistogram(f'{tau_id}_TauJetScoreAccepted_1p', path='EXPERT', type='TH1F', title=f'Accepted 1-prong {tau_id} Tau ID score; Score; Entries', xbins=xbins, xmin=0, xmax=xmax)
        monTool.defineHistogram(f'{tau_id}_TauJetScoreTransAccepted_1p', path='EXPERT', type='TH1F', title=f'Accepted 1-prong {tau_id} Tau ID transformed score; Transformed Signal Score; Entries', xbins=xbins, xmin=0, xmax=xmax)

        monTool.defineHistogram(f'{tau_id}_TauJetScoreAccepted_mp', path='EXPERT', type='TH1F', title=f'Accepted multi-prong {tau_id} Tau ID score; Score; Entries', xbins=xbins, xmin=0, xmax=xmax)
        monTool.defineHistogram(f'{tau_id}_TauJetScoreTransAccepted_mp', path='EXPERT', type='TH1F', title=f'Accepted multi-prong {tau_id} Tau ID transformed score; Transformed Signal Score; Entries', xbins=xbins, xmin=0, xmax=xmax)

    return monTool


def getTrigTauPrecisionDiKaonHypoToolMonitoring(flags, name: str):
    monTool = GenericMonitoringTool(flags, f'MonTool_{name}')
    monTool.HistPath = 'ComboHypo/' + name.replace('leg001_', '')

    # We will fix the naming convention as soon as the first weekly HLT Reprocessing validation goes through, because it will break the references

    monTool.defineHistogram('NInputTaus', path='EXPERT', type='TH1F', title='Input Taus (before selection); N Taus; Entries', xbins=10, xmin=0, xmax=10)

    monTool.defineHistogram('PtAccepted', path='EXPERT', type='TH1F', title='Accepted Tau p_{T}; p_{T} [GeV]; Entries', xbins=80, xmin=0, xmax=800)
    monTool.defineHistogram('NTracksAccepted', path='EXPERT', type='TH1F', title='Accepted Tau Tracks; N Tracks; Entries', xbins=10, xmin=0, xmax=10)
    monTool.defineHistogram('NIsoTracksAccepted', path='EXPERT', type='TH1F', title='Accepted Tau Isolation Tracks; N Isolation Tracks; Entries', xbins=10, xmin=0, xmax=10)

    monTool.defineHistogram('dRAccepted', path='EXPERT', type='TH1F', title='Accepted Tau Maximum #DeltaR(Tau, Tracks); Maximum #DeltaR(Tau, Tracks); Entries', xbins=40, xmin=0, xmax=0.4)
    monTool.defineHistogram('massTrkSysAccepted', path='EXPERT', type='TH1F', title='Accepted Tau Di-pion system Mass; m_{#pi#pi} [GeV]; Entries', xbins=50, xmin=0, xmax=3)
    monTool.defineHistogram('massTrkSysKaonAccepted', path='EXPERT', type='TH1F', title='Accepted Tau Di-kaon system Mass; m_{KK} [GeV]; Entries', xbins=50, xmin=0, xmax=3)
    monTool.defineHistogram('massTrkSysKaonPiAccepted', path='EXPERT', type='TH1F', title='Accepted Tau Kaon+Pion system Mass; m_{K#pi} [GeV]; Entries', xbins=50, xmin=0, xmax=3)
    monTool.defineHistogram('leadTrkPtAccepted', path='EXPERT', type='TH1F', title='Accepted Tau Leading Track p_{T}; Leading Track p_{T} [GeV]; Entries', xbins=50, xmin=0, xmax=300)
    monTool.defineHistogram('etOverPtLeadTrkAccepted', path='EXPERT', type='TH1F', title='Accepted Tau (E_{T}^{EM} + E_{T}^{Had}) / p_{T}^{lead trk.}; (E_{T}^{EM} + E_{T}^{Had}) / p_{T}^{lead trk.}; Entries', xbins=50, xmin=0, xmax=5)
    monTool.defineHistogram('EMOverTrkSysPAccepted', path='EXPERT', type='TH1F', title='Accepted Tau E_{T}^{EM} over Track system p_{T}; E_{T}^{EM} / p_{T}^{trk sys}; Entries', xbins=50, xmin=0, xmax=5)

    return monTool
