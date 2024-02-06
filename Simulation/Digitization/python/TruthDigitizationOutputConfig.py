"""Configure Truth output for digitization with ComponentAccumulator style

Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.Enums import ProductionStep
from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg


def TruthDigitizationOutputCfg(flags):
    """Return ComponentAccumulator with Truth output items"""
    prefix = ''
    if flags.Common.ProductionStep == ProductionStep.PileUpPresampling:
        prefix = flags.Overlay.BkgPrefix

    ItemList = [
        f"McEventCollection#{prefix}TruthEvent",
        f"TrackRecordCollection#{prefix}MuonEntryLayer",
    ]
    if not flags.Digitization.PileUp:
        ItemList += [
           f"TrackRecordCollection#{prefix}CaloEntryLayer",
           f"TrackRecordCollection#{prefix}MuonExitLayer",
        ]
        # Extra TrackRecordCollections to copy for cosmics Digitization
        if 'TrackRecordCollection#CosmicPerigee' in flags.Input.TypedCollections:
            ItemList += ["TrackRecordCollection#CosmicPerigee"]
        if 'TrackRecordCollection#CosmicRecord' in flags.Input.TypedCollections:
            ItemList += ["TrackRecordCollection#CosmicRecord"]

    from RunDependentSimComps.PileUpUtils import pileupInputCollections
    puCollections = pileupInputCollections(flags.Digitization.PU.LowPtMinBiasInputCols)

    dropped_jet_vars = ['constituentLinks',
                        'constituentWeights',
                        'ConeExclBHadronsFinal',
                        'ConeExclCHadronsFinal',
                        'ConeExclTausFinal',
                        'GhostPartons',
                        'GhostBHadronsFinal',
                        'GhostCHadronsFinal',
                        'GhostTausFinal']
    jet_var_str = '.-'.join ([''] + dropped_jet_vars)

    if "AntiKt4TruthJets" in puCollections:
        ItemList += [
            f"xAOD::JetContainer#{prefix}InTimeAntiKt4TruthJets",
            f"xAOD::AuxContainerBase!#{prefix}InTimeAntiKt4TruthJetsAux" + jet_var_str,
            f"xAOD::JetContainer#{prefix}OutOfTimeAntiKt4TruthJets",
            f"xAOD::AuxContainerBase!#{prefix}OutOfTimeAntiKt4TruthJetsAux" + jet_var_str,
        ]
    if "AntiKt6TruthJets" in puCollections:
        ItemList += [
            f"xAOD::JetContainer#{prefix}InTimeAntiKt6TruthJets",
            f"xAOD::AuxContainerBase!#{prefix}InTimeAntiKt6TruthJetsAux" + jet_var_str,
            f"xAOD::JetContainer#{prefix}OutOfTimeAntiKt6TruthJets",
            f"xAOD::AuxContainerBase!#{prefix}OutOfTimeAntiKt6TruthJetsAux" + jet_var_str,
        ]
    if "TruthPileupParticles" in puCollections:
        ItemList += [
            f"xAOD::TruthParticleContainer#{prefix}TruthPileupParticles",
            f"xAOD::TruthParticleAuxContainer#{prefix}TruthPileupParticlesAux.",
        ]

    return OutputStreamCfg(flags, "RDO", ItemList)
